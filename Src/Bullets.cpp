#include "Bullets.h"
#include "Player.h"
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <cmath>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SDL_Texture* Bullet::loadTexture(const std::string& path) {
    SDL_Texture* newTexture = IMG_LoadTexture(renderer, path.c_str());
    if (!newTexture) {
        SDL_Log("Can't load texture %s! SDL_image Error: %s", path.c_str(), IMG_GetError());
    }
    return newTexture;
}

Bullet::Bullet()
    : renderer(nullptr), texture(nullptr), x(0), y(0), dx(0), dy(0), width(0), height(0), active(false), angle(0) {
}

Bullet::Bullet(SDL_Renderer* ren, const std::string& texturePath, float startX, float startY, float angleDeg, float spd)
    : renderer(ren), texture(nullptr), x(startX), y(startY), width(0), height(0), active(true), angle(angleDeg)
{
    texture = loadTexture(texturePath);
    if (texture) {
        SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    }
    float angleRad = angleDeg * M_PI / 180.0f;
    dx = cos(angleRad) * spd;
    dy = sin(angleRad) * spd;
}

Bullet::~Bullet() {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
}

void Bullet::update() {
    if (!active) return;

    x += dx;
    y += dy;

    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;

    if (x + width < 0 || x > SCREEN_WIDTH || y + height < 0 || y > SCREEN_HEIGHT) {
        active = false;
    }
}

void Bullet::render() {
    if (!active) {
        return;
    }

    SDL_Rect destRect = { static_cast<int>(x), static_cast<int>(y), width, height };
    SDL_Point center = { width / 2, height / 2 };
    SDL_RenderCopyEx(renderer, texture, NULL, &destRect, angle, &center, SDL_FLIP_NONE);
}

bool Bullet::isActive() const {
    return active;
}

SDL_Rect Bullet::getRect() const {
    return { static_cast<int>(x), static_cast<int>(y), width, height };
}

void Bullet::spawn(SDL_Renderer* ren, const std::string& texturePath, float startX, float startY, float angleDeg, float spd) {
    renderer = ren;
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    texture = loadTexture(texturePath);
    x = startX;
    y = startY;
    active = true;
    angle = angleDeg;

    if (texture) {
        SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    }

    float angleRad = angleDeg * M_PI / 180.0f;
    dx = cos(angleRad) * spd;
    dy = sin(angleRad) * spd;
}

void Bullet::deactivate() {
    active = false;
}

