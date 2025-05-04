#include "Player.h"
#include "Enemies.h"
#include <iostream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SDL_Texture* Player::loadTexture(const std::string& path) {
    SDL_Texture* newTexture = IMG_LoadTexture(renderer, path.c_str());
    if (!newTexture) {
        SDL_Log("Unable to load texture %s! SDL_image Error: %s", path.c_str(), IMG_GetError());
    }
    else {
        SDL_Log("Texture loaded successfully: %s", path.c_str());
    }
    return newTexture;
}

Player::Player(SDL_Renderer* renderer,
    const std::string& idlePath, int idleFrames,
    const std::string& movePath, int moveFrames,
    const std::string& deathPath, int deathFrames,
    const std::string& weaponRightPath,
    const std::string& weaponLeftPath)
    : renderer(renderer),
    idleTexture(nullptr), moveTexture(nullptr), deathTexture(nullptr),
    weaponTextureRight(nullptr), weaponTextureLeft(nullptr),
    speed(5),
    currentState(Player::STATE_IDLE),
    currentFrame(0),
    totalFrames(idleFrames),
    animationSpeed(100),
    lastFrameTime(0),
    idleFrameCount(idleFrames),
    moveFrameCount(moveFrames),
    deathFrameCount(deathFrames),
    frameWidth(0), frameHeight(0),
    weaponWidth(0), weaponHeight(0),
    weaponOffsetX(0), weaponOffsetY(0),
    weaponAngle(0.0), weaponPivot({0, 0}),
    flipState(SDL_FLIP_NONE),
    currentWeaponAngle(0.0),
    currentPivotToUse({ 0,0 }),
    currentAngleToUse(0.0),
    currentWeaponPivotScreen({ 0,0 })
{
    idleTexture = loadTexture(idlePath);
    moveTexture = loadTexture(movePath);
    deathTexture = loadTexture(deathPath);

    weaponTextureRight = loadTexture(weaponRightPath);
    weaponTextureLeft = loadTexture(weaponLeftPath);

    if (idleTexture) {
        int totalWidth;
        SDL_QueryTexture(idleTexture, NULL, NULL, &totalWidth, &frameHeight);
        if (idleFrameCount > 0) {
            frameWidth = totalWidth / idleFrameCount;
        }
        else {
            SDL_Log("Error: Idle frame count = 0");
            frameWidth = 0;
        }
    }
    else {
        SDL_Log("Error: Failed to load idle texture '%s'", idlePath.c_str());
        frameWidth = 114; frameHeight = 194;
    }
    // kich thuoc vu khi
    if (weaponTextureRight) {
        SDL_QueryTexture(weaponTextureRight, NULL, NULL, &weaponWidth, &weaponHeight);
        weaponSourceRect = { 0, 0, weaponWidth, weaponHeight };
        //tim diem pivot so voi frame weapon
        weaponPivot.x = weaponWidth * 0.47;
        weaponPivot.y = weaponHeight * 0.78;
    }
    else {
        SDL_Log("Error: Failed to load weapon texture '%s'", weaponRightPath.c_str());
        weaponWidth = 0; weaponHeight = 0;
    }

    destRect.w = frameWidth;
    destRect.h = frameHeight;
    destRect.x = (800 - frameWidth) / 2;
    destRect.y = (600 - frameHeight) / 2;

    sourceRect.x = 0;
    sourceRect.y = 0;
    sourceRect.w = frameWidth;
    sourceRect.h = frameHeight;

    //dat vu khi len player
    weaponOffsetX = frameWidth * 0.01; 
    weaponOffsetY = frameHeight * 0.63;

    //tính vị trí vẽ vũ khí ban đầu
    weaponDestRect.w = weaponWidth;
    weaponDestRect.h = weaponHeight;
    weaponDestRect.x = destRect.x + weaponOffsetX;
    weaponDestRect.y = destRect.y + weaponOffsetY;

    lastFrameTime = SDL_GetTicks();
}

Player::~Player() {
    SDL_Log("Destroying player textures...");
    if (idleTexture) SDL_DestroyTexture(idleTexture);
    if (moveTexture) SDL_DestroyTexture(moveTexture);
    if (deathTexture) SDL_DestroyTexture(deathTexture);
    if (weaponTextureRight) SDL_DestroyTexture(weaponTextureRight);
    if (weaponTextureLeft) SDL_DestroyTexture(weaponTextureLeft);
    idleTexture = moveTexture = deathTexture = weaponTextureRight = weaponTextureLeft = nullptr;
    SDL_Log("Player textures destroyed.");
}

void Player::handleInput(const Uint8* keystate) {
    if (currentState == Player::STATE_DYING) {
        return;
    }

    bool isMoving = false;
    int dx = 0;
    int dy = 0;

    if (keystate[SDL_SCANCODE_W]) {
        dy -= speed; isMoving = true;
    }
    if (keystate[SDL_SCANCODE_S]) {
        dy += speed; isMoving = true;
    }
    if (keystate[SDL_SCANCODE_A]) {
        dx -= speed; isMoving = true;
    }
    if (keystate[SDL_SCANCODE_D]) {
        dx += speed; isMoving = true;
    }

    destRect.x += dx;
    destRect.y += dy;

    //giới hạn màn hình
    if (destRect.x < 0) destRect.x = 0;
    if (destRect.y < 0) destRect.y = 0;
    if (destRect.x + frameWidth > 800) destRect.x = 800 - frameWidth;
    if (destRect.y + frameHeight > 600) destRect.y = 600 - frameHeight;

    if (isMoving && currentState != Player::STATE_MOVING) {
        setState(Player::STATE_MOVING);
    }
    else if (!isMoving && currentState == Player::STATE_MOVING) {
        setState(Player::STATE_IDLE);
    }
}

void Player::update(int mouseX, int mouseY) {
    if (totalFrames > 1) {
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime > lastFrameTime + animationSpeed) {
            currentFrame++;
            if (currentFrame >= totalFrames) {
                if (currentState == Player::STATE_DYING) {
                    currentFrame = totalFrames - 1;
                }
                else {
                    currentFrame = 0;
                }
            }
            lastFrameTime = currentTime;
            sourceRect.x = currentFrame * frameWidth;
        }
    }
    else {
        sourceRect.x = 0;
        currentFrame = 0;
    }
    int playerCenterX = destRect.x + frameWidth / 2;
    flipState = (mouseX < playerCenterX) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    currentPivotToUse = weaponPivot;
    currentAngleToUse = currentWeaponAngle;
    if (flipState == SDL_FLIP_HORIZONTAL) {
        weaponDestRect.x = destRect.x + frameWidth - weaponOffsetX - weaponWidth;
    }
    else {
        weaponDestRect.x = destRect.x + weaponOffsetX;
    }
    weaponDestRect.y = destRect.y + weaponOffsetY;

    //tính góc xoay
    if (weaponTextureRight) {
        double gocPlayerCenterX = static_cast<double>(destRect.x) + frameWidth / 2.0;
        double gocPlayerCenterY = static_cast<double>(destRect.y) + frameHeight / 2.0;
        double deltaX = static_cast<double>(mouseX) - gocPlayerCenterX;
        double deltaY = static_cast<double>(mouseY) - gocPlayerCenterY;
        double angleRadians = atan2(deltaY, deltaX);
        weaponAngle = angleRadians * 180.0 / M_PI;
    }
    else {
        weaponAngle = 0.0;
    }
    currentWeaponPivotScreen.x = weaponDestRect.x + currentPivotToUse.x;
    currentWeaponPivotScreen.y = weaponDestRect.y + currentPivotToUse.y;

}

void Player::render() {
    SDL_Texture* currentTexture = nullptr;
    switch (currentState) {
    case Player::STATE_IDLE:
        currentTexture = idleTexture;
        break;
    case Player::STATE_MOVING:
        currentTexture = moveTexture;
        break;
    case Player::STATE_DYING:
        currentTexture = deathTexture;
        break;
    default:
        currentTexture = idleTexture;
        break;
    }

    if (currentTexture && frameWidth > 0 && frameHeight > 0) {
        SDL_RenderCopyEx(renderer, currentTexture, &sourceRect, &destRect, 0.0, NULL, flipState);
    }

    SDL_Texture* currentWeaponTexture = nullptr;
    SDL_Point pivotToUse = weaponPivot;
    double angleToUse = weaponAngle;

    if (flipState == SDL_FLIP_HORIZONTAL) {
        currentWeaponTexture = weaponTextureLeft;
        pivotToUse.x = weaponWidth - weaponPivot.x;
        angleToUse = - (180.0 - weaponAngle);
    }
    else {
        currentWeaponTexture = weaponTextureRight;
    }

    if (currentWeaponTexture && weaponWidth > 0 && weaponHeight > 0) {
        SDL_RenderCopyEx(renderer, currentWeaponTexture, &weaponSourceRect, &weaponDestRect, angleToUse, &pivotToUse, SDL_FLIP_NONE);
    }
}


void Player::setState(int newState) {
    if (currentState != newState) {
        changeAnimation(newState);
    }
}

void Player::changeAnimation(int newState) {
    SDL_Log("Changing player state from %d to %d", currentState, newState);
    currentState = newState;
    currentFrame = 0;
    lastFrameTime = SDL_GetTicks();

    switch (currentState) {
    case Player::STATE_IDLE:
        totalFrames = idleFrameCount;
        break;
    case Player::STATE_MOVING:
        totalFrames = moveFrameCount;
        break;
    case Player::STATE_DYING:
        totalFrames = deathFrameCount;
        // kích thước spritesheet khác
        if (deathTexture) {
            int totalWidth;
            SDL_QueryTexture(deathTexture, NULL, NULL, &totalWidth, &frameHeight);
            frameWidth = totalWidth / deathFrameCount;
        }
        break;
    default:
        totalFrames = 1;
        break;
    }

    sourceRect.x = 0;
    sourceRect.w = frameWidth;
    sourceRect.h = frameHeight;

    destRect.w = frameWidth;
    destRect.h = frameHeight;
}

SDL_Point Player::getWeaponPivotScreenPosition() const {
    return currentWeaponPivotScreen;
}

double Player::getWeaponAngle() const {
    return currentAngleToUse;
}

void Player::checkCollisionWithEnemies(const Enemy* enemies, int enemyCount) {
    if (currentState == STATE_DYING) return;

    SDL_Rect playerRect = getPositionRect();

    for (int i = 0; i < enemyCount; ++i) {
        if (!enemies[i].isActive()) continue;

        SDL_Rect enemyRect = enemies[i].getRect();

        if (SDL_HasIntersection(&playerRect, &enemyRect)) {
            setState(STATE_DYING);
            break;
        }
    }
}