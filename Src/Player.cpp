#include "Player.h"
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
            SDL_Log("Error: Idle frame count is zero!");
            frameWidth = 0;
        }
        SDL_Log("Frame dimensions determined: %d x %d", frameWidth, frameHeight);
    }
    else {
        SDL_Log("Error: Failed to load idle texture '%s', cannot determine frame size.", idlePath.c_str());
        frameWidth = 114; frameHeight = 194;
        SDL_Log("Using fallback frame size: 32x32");
    }
    // kich thuoc vu khi
    if (weaponTextureRight) {
        SDL_QueryTexture(weaponTextureRight, NULL, NULL, &weaponWidth, &weaponHeight);
        SDL_Log("Weapon dimensions determined: %d x %d", weaponWidth, weaponHeight);
        weaponSourceRect = { 0, 0, weaponWidth, weaponHeight }; // Giả sử vũ khí là 1 ảnh tĩnh
        // --- QUAN TRỌNG: Đặt điểm xoay (Pivot) cho vũ khí ---
        // Đây là tọa độ X, Y *bên trong* ảnh vũ khí (từ góc trên trái)
        // mà bạn muốn súng xoay quanh (ví dụ: phần tay cầm)
        // Cần TINH CHỈNH giá trị này!
        weaponPivot.x = weaponWidth * 0.47;
        weaponPivot.y = weaponHeight * 0.78;
        SDL_Log("Weapon Pivot set to: X=%d, Y=%d", weaponPivot.x, weaponPivot.y);
    }
    else {
        SDL_Log("Error: Failed to load weapon texture '%s'", weaponRightPath.c_str());
        weaponWidth = 0; weaponHeight = 0; // Hoặc đặt kích thước mặc định nhỏ
    }

    destRect.w = frameWidth;
    destRect.h = frameHeight;
    destRect.x = (800 - frameWidth) / 2;
    destRect.y = (600 - frameHeight) / 2;

    sourceRect.x = 0;
    sourceRect.y = 0;
    sourceRect.w = frameWidth;
    sourceRect.h = frameHeight;

    // --- QUAN TRỌNG: Thiết lập Offset cho vũ khí ---
    // Đây là phần cần bạn TINH CHỈNH bằng cách thử nghiệm giá trị
    // Ví dụ: Đặt súng lệch về bên phải và hơi thấp xuống so với góc trên trái player
    weaponOffsetX = frameWidth * 0.01; 
    weaponOffsetY = frameHeight * 0.63;
    SDL_Log("Initial Weapon Offset: X=%d, Y=%d", weaponOffsetX, weaponOffsetY);

    // Tính vị trí vẽ vũ khí ban đầu (sẽ cập nhật trong update)
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
    // Sử dụng hằng số int để kiểm tra state
    if (currentState == Player::STATE_DYING) {
        return;
    }

    bool isMoving = false;
    int dx = 0;
    int dy = 0;

    if (keystate[SDL_SCANCODE_W]) { dy -= speed; isMoving = true; }
    if (keystate[SDL_SCANCODE_S]) { dy += speed; isMoving = true; }
    if (keystate[SDL_SCANCODE_A]) { dx -= speed; isMoving = true; }
    if (keystate[SDL_SCANCODE_D]) { dx += speed; isMoving = true; }

    destRect.x += dx;
    destRect.y += dy;

    // Giới hạn màn hình
    if (destRect.x < 0) destRect.x = 0;
    if (destRect.y < 0) destRect.y = 0;
    if (destRect.x + frameWidth > 800) destRect.x = 800 - frameWidth;
    if (destRect.y + frameHeight > 600) destRect.y = 600 - frameHeight;

    // Thay đổi trạng thái dựa trên input (sử dụng hằng số int)
    if (isMoving && currentState != Player::STATE_MOVING) {
        setState(Player::STATE_MOVING);
    }
    else if (!isMoving && currentState == Player::STATE_MOVING) {
        setState(Player::STATE_IDLE);
    }

    // Test trạng thái DYING (nhấn K)
    if (keystate[SDL_SCANCODE_K] && currentState != Player::STATE_DYING) {
        setState(Player::STATE_DYING);
    }
}

void Player::update(int mouseX, int mouseY) {
    // 1. Cập nhật animation Player (như cũ)
    if (totalFrames > 1) { // Chỉ chạy animation nếu có nhiều hơn 1 frame
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
    else { // Nếu chỉ có 1 frame thì đặt về 0
        sourceRect.x = 0;
        currentFrame = 0;
    }
    // 2. Xác định trạng thái Flip cho Player
    int playerCenterX = destRect.x + frameWidth / 2;
    flipState = (mouseX < playerCenterX) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    // 3. Cập nhật vị trí Vũ khí DỰA TRÊN FLIP STATE CỦA PLAYER
    currentPivotToUse = weaponPivot;      // Bắt đầu với pivot gốc
    currentAngleToUse = currentWeaponAngle; // Bắt đầu với góc gốc
    if (flipState == SDL_FLIP_HORIZONTAL) { // Player lật trái
        weaponDestRect.x = destRect.x + frameWidth - weaponOffsetX - weaponWidth;
        //currentPivotToUse.x = weaponWidth - weaponPivot.x;
        //currentAngleToUse = fmod(180.0 + currentWeaponAngle, 360.0);
    }
    else { // Player không lật (nhìn phải)
        weaponDestRect.x = destRect.x + weaponOffsetX;

    }
    weaponDestRect.y = destRect.y + weaponOffsetY; // Y không đổi

    // 4. Tính toán góc xoay Vũ khí TỪ TÂM PLAYER (luôn tính góc tuyệt đối)
    // Chỉ tính nếu có texture vũ khí (ít nhất là cái bên phải làm chuẩn)
    if (weaponTextureRight) {
        double playerCenterX_ForAngle = static_cast<double>(destRect.x) + frameWidth / 2.0;
        double playerCenterY_ForAngle = static_cast<double>(destRect.y) + frameHeight / 2.0;
        double deltaX = static_cast<double>(mouseX) - playerCenterX_ForAngle;
        double deltaY = static_cast<double>(mouseY) - playerCenterY_ForAngle;
        double angleRadians = atan2(deltaY, deltaX);
        weaponAngle = angleRadians * 180.0 / M_PI;
    }
    else {
        weaponAngle = 0.0; // Không xoay nếu không có vũ khí
    }
    currentWeaponPivotScreen.x = weaponDestRect.x + currentPivotToUse.x;
    currentWeaponPivotScreen.y = weaponDestRect.y + currentPivotToUse.y;

}

void Player::render() {
    SDL_Texture* currentTexture = nullptr;

    // Sử dụng hằng số int trong switch
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
    default: // Fallback an toàn
        currentTexture = idleTexture;
        break;
    }

    if (currentTexture && frameWidth > 0 && frameHeight > 0) {
        int texW, texH;
        SDL_QueryTexture(currentTexture, NULL, NULL, &texW, &texH);
        if (sourceRect.x + sourceRect.w > texW) {
            SDL_Log("Warning: sourceRect.x (%d + %d) exceeds texture width (%d) for state %d. Clamping.",
                sourceRect.x, sourceRect.w, texW, currentState); // Log currentState trực tiếp
            sourceRect.x = texW - sourceRect.w;
            if (sourceRect.x < 0) sourceRect.x = 0;
        }
        //SDL_RenderCopy(renderer, currentTexture, &sourceRect, &destRect);
        SDL_RenderCopyEx(renderer,
            currentTexture,
            &sourceRect,        // Frame nguồn
            &destRect,          // Vị trí đích
            0.0,                // Góc xoay Player (0 nếu không xoay)
            NULL,               // Điểm xoay Player (NULL là tâm)
            flipState);         // <-- Áp dụng trạng thái lật
    }
    else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &destRect);
        if (frameWidth <= 0 || frameHeight <= 0) {
            SDL_Log("Error rendering Player: Invalid frame dimensions (%d x %d)", frameWidth, frameHeight);
        }
        else if (!currentTexture) {
            SDL_Log("Error rendering Player: Current texture is null for state %d", currentState); // Log currentState trực tiếp
        }
    }

    // 3. Chọn Texture Vũ khí phù hợp
    SDL_Texture* currentWeaponTexture = nullptr;
    SDL_Point pivotToUse = weaponPivot;
    double angleToUse = weaponAngle;

    if (flipState == SDL_FLIP_HORIZONTAL) { // Player lật trái
        currentWeaponTexture = weaponTextureLeft;  // Dùng ảnh vũ khí trái
        pivotToUse.x = weaponWidth - weaponPivot.x;
        angleToUse = - (180.0 - weaponAngle);
    }
    else { // Player nhìn phải
        currentWeaponTexture = weaponTextureRight; // Dùng ảnh vũ khí phải
    }

    // 4. Vẽ Vũ khí đã chọn (KHÔNG FLIP, dùng PIVOT GỐC)
    if (currentWeaponTexture && weaponWidth > 0 && weaponHeight > 0) {
        SDL_RenderCopyEx(renderer,
            currentWeaponTexture, // <-- Texture đã đúng hướng
            &weaponSourceRect,    // Source rect
            &weaponDestRect,      // Vị trí đích đã tính
            angleToUse,          // Góc tuyệt đối
            &pivotToUse,         // <-- Pivot gốc (không cần điều chỉnh)
            SDL_FLIP_NONE);       // <-- Luôn là NONE cho vũ khí
    }
}



// Hàm setState nhận int
void Player::setState(int newState) {
    // Kiểm tra xem state có hợp lệ không (tùy chọn nhưng nên làm)
    if (newState < Player::STATE_IDLE || newState > Player::STATE_DYING) { // Giả sử DYING là state lớn nhất
        SDL_Log("Warning: Attempted to set invalid player state: %d", newState);
        return;
    }

    if (currentState != newState /*&& currentState != Player::STATE_DYING*/) {
        changeAnimation(newState);
    }
}


// Hàm changeAnimation nhận int
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

    // Cập nhật lại kích thước hiển thị theo frame mới
    destRect.w = frameWidth;
    destRect.h = frameHeight;

    if (totalFrames <= 0) {
        SDL_Log("Warning: Total frames for state %d is %d. Forcing to 1.", currentState, totalFrames);
        totalFrames = 1;
        currentFrame = 0;
        sourceRect.x = 0;
    }
}

// Lấy vị trí điểm xoay (pivot) của vũ khí trên màn hình
SDL_Point Player::getWeaponPivotScreenPosition() const {
    // Trả về giá trị đã được tính toán và lưu trữ trong hàm update()
    return currentWeaponPivotScreen;
}

// Lấy góc cuối cùng của vũ khí (đã điều chỉnh theo flip)
double Player::getWeaponAngle() const {
    // Trả về góc đã được tính toán và lưu trữ trong hàm update()
    // Góc này phù hợp để tạo đạn theo hướng nhìn của vũ khí
    return currentAngleToUse;
}