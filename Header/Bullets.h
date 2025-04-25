#ifndef BULLET_H
#define BULLET_H

#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <cmath> // For sin, cos
#include <vector>

class Bullet {
public:
    // Constructor: Takes renderer, texture path, starting position (float), angle (degrees), and speed
    Bullet(SDL_Renderer* renderer, const std::string& texturePath, float startX, float startY, float angleDeg, float speed);

    // Destructor: Cleans up the texture
    ~Bullet();

    // update: Moves the bullet based on its velocity and checks boundaries
    void update();

    // render: Draws the bullet on the screen with rotation
    void render();

    // isActive: Checks if the bullet is still active (e.g., on-screen)
    bool isActive() const;

    // getRect: Returns the bullet's bounding box (useful for collisions later)
    SDL_Rect getRect() const;

    static void ShootToward(SDL_Renderer* renderer, std::vector<Bullet>& bullets, const std::string& texturePath,
        SDL_Point origin, int mouseX, int mouseY, float speed);

private:
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    float x, y; // Precise position (float for smooth movement)
    float dx, dy; // Velocity components
    int width, height; // Dimensions of the bullet texture
    bool active; // Is the bullet currently active?
    float angle; // Store angle for rendering rotation

    // Helper to load texture
    SDL_Texture* loadTexture(const std::string& path);
};

#endif // BULLET_H