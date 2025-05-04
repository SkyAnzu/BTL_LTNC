#include "Enemies.h"
#include "Bullets.h"    
#include <SDL_image.h>
#include <cstdlib>
#include <ctime>
#include <cmath>

Enemy::Enemy()
    : renderer(nullptr),
    moveTexture(nullptr),
    x(0), y(0), dx(0), dy(0),
    width(0), height(0),
    frameWidth(0), frameHeight(0),
    active(false),
    currentFrame(0), totalFrames(1),
    moveFrameCount(0),
    animationSpeed(100), lastFrameTime(0),
    flipState(SDL_FLIP_NONE),
    sourceRect{0, 0, 0, 0}
{
    srand(static_cast<unsigned>(time(nullptr)));
}

Enemy::~Enemy() {
    if (moveTexture) SDL_DestroyTexture(moveTexture);
}

SDL_Texture* Enemy::loadTexture(const std::string& path) {
    SDL_Texture* tex = IMG_LoadTexture(renderer, path.c_str());
    if (!tex) {
        SDL_Log("Enemy Texture Load Error: %s", SDL_GetError());
    }
    return tex;
}

bool checkOverlap(const SDL_Rect& a, const SDL_Rect& b) {
    return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
}

void Enemy::spawn(SDL_Renderer* ren,
    const std::string& movePath, int moveFrames,
    int screenWidth, int screenHeight,
    const SDL_Rect& playerRect, const Enemy* others, int enemyCount)
{
    renderer = ren;
    moveTexture = loadTexture(movePath);

    moveFrameCount = moveFrames;
    totalFrames = moveFrameCount;
    animationSpeed = 100;
    currentFrame = 0;
    lastFrameTime = SDL_GetTicks();

    if (moveTexture) {
        int totalWidth;
        SDL_QueryTexture(moveTexture, NULL, NULL, &totalWidth, &frameHeight);
        frameWidth = totalWidth / moveFrameCount;
    }
    else {
        frameWidth = frameHeight = 32;
    }

    frameWidth = 98;
    frameHeight = 82;

    width = frameWidth;
    height = frameHeight;

    active = true;

    int side = rand() % 4;
    int attempts = 0;
    do {
        if (side == 0) { x = rand() % screenWidth; y = 0; }
        else if (side == 1) { x = rand() % screenWidth; y = screenHeight - height; }
        else if (side == 2) { x = 0; y = rand() % screenHeight; }
        else { x = screenWidth - width; y = rand() % screenHeight; }

        SDL_Rect newRect = { (int)x, (int)y, width, height };
        bool overlapping = false;
        for (int i = 0; i < enemyCount; ++i) {
            if (others[i].isActive() && checkOverlap(newRect, others[i].getRect())) {
                overlapping = true;
                break;
            }
        }
        if (!checkOverlap(newRect, playerRect) && !overlapping) {
            break;
        }
        attempts++;
    } while (attempts < 20);

    sourceRect = { 0, 0, frameWidth, frameHeight };
}

void Enemy::update(const SDL_Rect& playerRect) {
    if (!active) return;

        //flip theo vị trí player
        int enemyCenterX = (int)x + width / 2;
        int playerCenterX = playerRect.x + playerRect.w / 2;
        flipState = (enemyCenterX < playerCenterX) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

        //di chuyển về phía player
        float centerPlayerX = playerRect.x + playerRect.w / 2;
        float centerPlayerY = playerRect.y + playerRect.h / 2;
        float diffX = centerPlayerX - (x + width / 2);
        float diffY = centerPlayerY - (y + height / 2);
        float distance = sqrt(diffX * diffX + diffY * diffY);

        if (distance != 0) {
            dx = diffX / distance;
            dy = diffY / distance;
        }
        else {
            dx = dy = 0;
        }

        float speed = 2.0f;
        x += dx * speed;
        y += dy * speed;

    Uint32 currentTime = SDL_GetTicks();
    if (currentTime > lastFrameTime + animationSpeed) {
        currentFrame = (currentFrame + 1) % totalFrames;
        sourceRect.x = currentFrame * frameWidth;
        lastFrameTime = currentTime;
    }
}


void Enemy::render() {
    if (!active) return;

    SDL_Rect srcRect;
    srcRect.x = currentFrame * frameWidth;
    srcRect.y = 0;
    srcRect.w = frameWidth;
    srcRect.h = frameHeight;

    SDL_Rect destRect;
    destRect.x = static_cast<int>(x);
    destRect.y = static_cast<int>(y);
    destRect.w = frameWidth;
    destRect.h = frameHeight;

    SDL_RenderCopyEx(renderer, moveTexture, &srcRect, &destRect, 0, NULL, flipState);
}

bool Enemy::isActive() const {
    return active;
}

SDL_Rect Enemy::getRect() const {
    return { (int)x, (int)y, width, height };
}

void Enemy::checkBulletCollision(Bullet* bullets, int bulletCount) {
    if (!active) return;

    SDL_Rect enemyRect = getRect();

    for (int i = 0; i < bulletCount; ++i) {
        if (!bullets[i].isActive()) continue;

        SDL_Rect bulletRect = bullets[i].getRect();
        if (SDL_HasIntersection(&enemyRect, &bulletRect)) {
            active = false;
            bullets[i].deactivate();
            break;
        }
    }
}
