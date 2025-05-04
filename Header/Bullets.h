#ifndef BULLET_H
#define BULLET_H

#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <cmath>
#include <vector>

class Bullet {
public:
    Bullet();
    Bullet(SDL_Renderer* renderer, const std::string& texturePath, float startX, float startY, float angleDeg, float speed);

    ~Bullet();
    void update();
    void render();
    bool isActive() const;
    SDL_Rect getRect() const;
    void spawn(SDL_Renderer* renderer, const std::string& texturePath, float startX, float startY, float angleDeg, float speed);
    void deactivate();

private:
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    float x, y;
    float dx, dy;
    int width, height;
    bool active;
    float angle;

    SDL_Texture* loadTexture(const std::string& path);
};

#endif