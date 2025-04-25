#ifndef _PLAYER__H
#define _PLAYER__H

#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <string>

class Player {
public:
    // Định nghĩa các trạng thái bằng hằng số static const int trong class
    static const int STATE_IDLE = 0;
    static const int STATE_MOVING = 1;
    static const int STATE_DYING = 2;

    Player(SDL_Renderer* renderer,
        const std::string& idlePath, int idleFrames,
        const std::string& movePath, int moveFrames,
        const std::string& deathPath, int deathFrames,
        const std::string& weaponRightPath,
        const std::string& weaponLeftPath);

    ~Player();

    void handleInput(const Uint8* keystate);
    void update(int mouseX, int mouseY); // xử lý animation
    void render();

    int getCurrentState() const { return currentState; }
    void setState(int newState);

    SDL_Rect getPositionRect() const {
        return destRect;
    }
    SDL_Point getWeaponPivotScreenPosition() const;
    double getWeaponAngle() const;
private:
    SDL_Renderer* renderer;

    // Textures cho các trạng thái
    SDL_Texture* idleTexture;
    SDL_Texture* moveTexture;
    SDL_Texture* deathTexture;

    SDL_Rect sourceRect; // Phần của sprite sheet sẽ vẽ (frame hiện tại)
    SDL_Rect destRect;   // Vị trí và kích thước vẽ lên màn hình
    int speed;

    // Thông tin Animation
    int currentState;     // Sử dụng int thay vì PlayerState
    int frameWidth;       // Chiều rộng 1 frame
    int frameHeight;      // Chiều cao 1 frame
    int currentFrame;     // Frame hiện tại đang hiển thị (index)
    int totalFrames;      // Tổng số frame của animation hiện tại
    Uint32 animationSpeed; // Thời gian (ms) giữa các frame
    Uint32 lastFrameTime;  // Thời điểm cập nhật frame cuối cùng

    // Số frame cho mỗi animation
    int idleFrameCount;
    int moveFrameCount;
    int deathFrameCount;

    SDL_Texture* weaponTextureRight;
    SDL_Texture* weaponTextureLeft;
    SDL_Rect weaponSourceRect; // Chỉ cần nếu spritesheet vũ khí có nhiều frame/loại
    SDL_Rect weaponDestRect;   // Vị trí vẽ vũ khí lên màn hình
    int weaponOffsetX;        // Độ lệch X của vũ khí so với vị trí Player
    int weaponOffsetY;        // Độ lệch Y của vũ khí so với vị trí Player
    int weaponWidth;          // Chiều rộng của sprite vũ khí
    int weaponHeight;         // Chiều cao của sprite vũ khí

    double weaponAngle;       // gocs xoay
    SDL_Point weaponPivot;    // diem xoay
    
    SDL_RendererFlip flipState;

    // Hàm nội bộ để load texture
    SDL_Texture* loadTexture(const std::string& path);
    // Hàm nội bộ để chuyển state và reset animation (nhận int)
    void changeAnimation(int newState);




    SDL_Point currentWeaponPivotScreen; // Tọa độ pivot trên màn hình (quan trọng nhất)
    SDL_Point currentPivotToUse;       // Pivot thực tế dùng để vẽ (đã điều chỉnh flip)
    double    currentAngleToUse;       // Góc thực tế dùng để vẽ (đã điều chỉnh flip)
    double    currentWeaponAngle;      // Góc gốc tính toán tới chuột

};

#endif
