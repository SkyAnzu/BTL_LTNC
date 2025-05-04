#ifndef _PLAYER__H
#define _PLAYER__H

#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <string>

#include "Enemies.h"
class Player {
public:
    //định nghĩa các trạng thái bằng hằng số static const int trong class
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
    void update(int mouseX, int mouseY);
    void render();

    int getCurrentState() const { return currentState; }
    void setState(int newState);

    SDL_Rect getPositionRect() const {
        return destRect;
    }
    SDL_Point getWeaponPivotScreenPosition() const;

    double getWeaponAngle() const;

    void checkCollisionWithEnemies(const Enemy* enemies, int enemyCount);

private:
    SDL_Renderer* renderer;
    SDL_Texture* idleTexture;
    SDL_Texture* moveTexture;
    SDL_Texture* deathTexture;
    SDL_Rect sourceRect;
    SDL_Rect destRect;
    int speed;

    // Thông tin Animation
    int currentState;
    int frameWidth;
    int frameHeight;
    int currentFrame;
    int totalFrames;
    Uint32 animationSpeed;
    Uint32 lastFrameTime;

    int idleFrameCount;
    int moveFrameCount;
    int deathFrameCount;

    SDL_Texture* weaponTextureRight;
    SDL_Texture* weaponTextureLeft;
    SDL_Rect weaponSourceRect;
    SDL_Rect weaponDestRect;
    int weaponOffsetX;
    int weaponOffsetY;
    int weaponWidth;
    int weaponHeight;

    double weaponAngle;       // goc xoay
    SDL_Point weaponPivot;    // diem xoay
    SDL_RendererFlip flipState;

    SDL_Texture* loadTexture(const std::string& path);
    void changeAnimation(int newState);


    SDL_Point currentWeaponPivotScreen;
    SDL_Point currentPivotToUse;
    double    currentAngleToUse;
    double    currentWeaponAngle;

};

#endif
