#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <string>
#include "Header/Player.h"
#include "Header/Bullets.h"


int main(int argc, char* argv[])
{
    // --- Khởi tạo SDL và SDL_image ---
    if (SDL_Init(SDL_INIT_VIDEO) != 0) { SDL_Log("SDL Init Error: %s", SDL_GetError()); return 1; }
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) { SDL_Log("IMG_Init Error: %s", IMG_GetError()); SDL_Quit(); return 1; }
    SDL_Log("SDL and SDL_image initialized successfully.");

    // --- Tạo cửa sổ và renderer ---
    SDL_Window* window = SDL_CreateWindow("20 Minutes Till Dawn", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    if (!window) { /*...*/ IMG_Quit(); SDL_Quit(); return 1; }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) { /*...*/ SDL_DestroyWindow(window); IMG_Quit(); SDL_Quit(); return 1; }
    SDL_Log("Window and Renderer created successfully.");

    // --- Load background ---
    SDL_Texture* background = IMG_LoadTexture(renderer, "assets/images/bg.jpg");
    if (!background) { SDL_Log("Không thể load background 'assets/images/bg.jpg': %s", IMG_GetError()); /*...*/ return 1; }
    SDL_Log("Background loaded successfully.");

    // --- Tạo Player với các animation ---
    std::string idleSpritePath = "assets/images/idle_sprite_sheet.png";
    int idleFrames = 6;
    std::string moveSpritePath = "assets/images/walk_sprite_sheet.png";
    int moveFrames = 8;
    std::string deathSpritePath = "assets/images/death_sprite_sheet.png";
    int deathFrames = 10;
    std::string weaponPathRight = "assets/images/weaponR1.png";
    std::string weaponPathLeft = "assets/images/weaponR2.png";
    Player player(renderer, idleSpritePath, idleFrames, moveSpritePath, moveFrames, deathSpritePath, deathFrames, weaponPathRight, weaponPathLeft);
    SDL_Log("Player object created.");

    // --- Quản lý đạn ---
    std::vector<Bullet> bullets; // Vector chứa các đối tượng đạn đang hoạt động
    std::string bulletTexturePath = "assets/images/bullet.png"; // Đảm bảo có ảnh này
    float bulletSpeed = 12.0f; // Tốc độ của đạn (pixel/frame, cần điều chỉnh theo FPS)
    Uint32 lastShotTime = 0;    // Thời điểm bắn viên đạn cuối cùng
    Uint32 fireDelay = 120;     // Thời gian chờ giữa 2 lần bắn (milliseconds)

    // --- Game Loop ---
    bool quit = false;
    SDL_Event e;
    Uint64 frameStart;
    Uint32 frameTime;
    const int FPS = 60;
    const int frameDelay = 1000 / FPS;
    int mouseX, mouseY;

    SDL_Log("Entering game loop...");
    while (!quit) {
        frameStart = SDL_GetTicks64();
        Uint32 currentTime = SDL_GetTicks();

        // 1. Events
        while (SDL_PollEvent(&e)) {
           if (e.type == SDL_QUIT) { quit = true; }
           if (e.type == SDL_MOUSEBUTTONDOWN) {
               if (e.button.button == SDL_BUTTON_LEFT && player.getCurrentState() != Player::STATE_DYING) {
                   // Kiểm tra thời gian chờ bắn
                   if (currentTime >= lastShotTime + fireDelay) {
                       // Lấy vị trí và góc từ Player
                       SDL_Point spawnPos = player.getWeaponPivotScreenPosition(); // Lấy tọa độ pivot trên màn hình
                       double spawnAngle = player.getWeaponAngle();             // Lấy góc cuối cùng của vũ khí

                       // Tạo viên đạn mới và thêm vào vector
                       bullets.emplace_back(renderer,
                           bulletTexturePath,
                           static_cast<float>(spawnPos.x), // Chuyển sang float
                           static_cast<float>(spawnPos.y), // Chuyển sang float
                           static_cast<float>(spawnAngle), // Chuyển sang float
                           bulletSpeed);

                       lastShotTime = currentTime; // Cập nhật thời điểm bắn cuối
                       // SDL_Log("Fired bullet from (%.1f, %.1f) at angle %.1f", (float)spawnPos.x, (float)spawnPos.y, (float)spawnAngle); // Log debug
                   }
               }
           }
        }

        SDL_GetMouseState(&mouseX, &mouseY); //lay toa do chuot

        // 2. Input
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        player.handleInput(keystate);

        // 3. Update
        player.update(mouseX, mouseY);

        for (auto& bullet : bullets) {
            bullet.update();
        }
        bullets.erase(
            std::remove_if(bullets.begin(), bullets.end(),
                [](const Bullet& b) { return !b.isActive(); }), // Điều kiện xóa: đạn không active
            bullets.end()
        );

        // 4. Render
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, background, NULL, NULL);
        player.render();
        for (auto& bullet : bullets) {
            bullet.render();
        }
        SDL_RenderPresent(renderer);

        // 5. Delay
        frameTime = SDL_GetTicks64() - frameStart;
        if (frameDelay > frameTime) { SDL_Delay(frameDelay - frameTime); }
    }
    SDL_Log("Exiting game loop.");

    // --- Giải phóng ---
    SDL_Log("Cleaning up resources...");
    bullets.clear();
    SDL_DestroyTexture(background); background = nullptr;
    SDL_DestroyRenderer(renderer); renderer = nullptr;
    SDL_DestroyWindow(window); window = nullptr;
    IMG_Quit();
    SDL_Quit();
    SDL_Log("Cleanup complete. Exiting program.");

    return 0;
}