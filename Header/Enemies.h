#ifndef ENEMY_H
#define ENEMY_H

#include <SDL.h>
#include <string>

class Bullet;

class Enemy {
public:


    Enemy();
    ~Enemy();

    void spawn(SDL_Renderer* renderer,
        const std::string& movePath, int moveFrames,
        int screenWidth, int screenHeight,
        const SDL_Rect& playerRect, const Enemy* others, int enemyCount);

    void update(const SDL_Rect& playerRect);
    void render();
    bool isActive() const;
    SDL_Rect getRect() const;

    void checkBulletCollision(Bullet* bullets, int bulletCount);

private:
    SDL_Renderer* renderer;
    SDL_Texture* moveTexture;


    float x, y;
    float dx, dy;
    int width, height;
    int frameWidth, frameHeight;
    bool active;

    int currentFrame;
    int totalFrames;
    int moveFrameCount;

    SDL_Rect sourceRect;
    Uint32 animationSpeed;
    Uint32 lastFrameTime;
    SDL_RendererFlip flipState;

    SDL_Texture* loadTexture(const std::string& path);

};

#endif
