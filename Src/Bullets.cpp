#include "Bullets.h"
#include "Player.h"
#include <SDL.h>
#include <SDL_image.h>
#include <iostream> // For SDL_Log / std::cout if needed
#include <cmath>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SDL_Texture* Bullet::loadTexture(const std::string& path) {
    SDL_Texture* newTexture = IMG_LoadTexture(renderer, path.c_str());
    if (!newTexture) {
        SDL_Log("Unable to load texture %s! SDL_image Error: %s", path.c_str(), IMG_GetError());
    }
    else {
        // SDL_Log("Bullet texture loaded successfully: %s", path.c_str()); // Optional log
    }
    return newTexture;
}

Bullet::Bullet(SDL_Renderer* ren, const std::string& texturePath, float startX, float startY, float angleDeg, float spd)
    : renderer(ren), texture(nullptr), x(startX), y(startY), width(0), height(0), active(true), angle(angleDeg)
{
    texture = loadTexture(texturePath);
    if (texture) {
        SDL_QueryTexture(texture, NULL, NULL, &width, &height);
        // Center the bullet visually on the start point if you prefer the *center* of the bullet to be at startX, startY
        // x -= width / 2.0f;
        // y -= height / 2.0f;
        // SDL_Log("Bullet dimensions: %d x %d", width, height); // Optional log
    }
    else {
        // Fallback size if texture fails to load
        width = 5; // Example fallback size
        height = 5;
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Bullet texture '%s' failed to load. Using fallback size %dx%d.", texturePath.c_str(), width, height);
    }

    // Convert angle from degrees to radians for trigonometric functions
    float angleRad = angleDeg * M_PI / 180.0f;

    // Calculate velocity components based on angle and speed
    dx = cos(angleRad) * spd;
    dy = sin(angleRad) * spd;

    // SDL_Log("Bullet created at (%.2f, %.2f) with angle %.2f deg, speed %.2f -> (dx=%.2f, dy=%.2f)", startX, startY, angleDeg, spd, dx, dy); // Optional log
}

Bullet::~Bullet() {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
}

void Bullet::update() {
    if (!active) {
        return;
    }

    // Move the bullet based on its velocity
    x += dx;
    y += dy;

    // Deactivate bullet if it goes off-screen
    // Adjust these boundaries if your screen size is different or you want bullets to persist longer off-screen
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;
    // Add a small buffer so bullets disappear slightly off-screen
    const int BUFFER = std::max(width, height); // Use largest dimension as buffer

    if (x + width < -BUFFER || x > SCREEN_WIDTH + BUFFER || y + height < -BUFFER || y > SCREEN_HEIGHT + BUFFER) {
        active = false;
    }
}

void Bullet::render() {
    if (!active) { // Don't render inactive bullets
        return;
    }

    if (!texture) { // Draw a fallback if texture is missing
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Bright Yellow fallback
        SDL_Rect fallbackRect = { static_cast<int>(x), static_cast<int>(y), width, height };
        SDL_RenderFillRect(renderer, &fallbackRect);
        return; // Don't try to render the null texture
    }

    SDL_Rect destRect = { static_cast<int>(x), static_cast<int>(y), width, height };
    // Render with rotation.
    // The bullet texture itself should face right (0 degrees).
    // Use the center of the bullet as the pivot point for rotation.
    SDL_Point center = { width / 2, height / 2 };
    SDL_RenderCopyEx(renderer, texture, NULL, &destRect, angle, &center, SDL_FLIP_NONE);
}

bool Bullet::isActive() const {
    return active;
}

// Returns the integer bounding box for potential collision detection
SDL_Rect Bullet::getRect() const {
    return { static_cast<int>(x), static_cast<int>(y), width, height };
}

void Bullet::ShootToward(SDL_Renderer* renderer,
    std::vector<Bullet>& bullets,
    const std::string& texturePath,
    SDL_Point origin,
    int mouseX, int mouseY,
    float speed)
{
    double dx = mouseX - origin.x;
    double dy = mouseY - origin.y;
    double angleRad = atan2(dy, dx);
    double angleDeg = angleRad * 180.0 / M_PI;

    float offset = 20.0f;
    float spawnX = origin.x + cos(angleRad) * offset;
    float spawnY = origin.y + sin(angleRad) * offset - 15.5;

    bullets.emplace_back(renderer, texturePath, spawnX, spawnY, angleDeg, speed);

}
