#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream> 
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

#include "Header/Player.h"
#include "Header/Bullets.h"
#include "Header/Enemies.h"
#include "Header/endMenu.h" 

using namespace std;


int main(int argc, char* argv[])
{
    bool restartGame = true;
    TTF_Font* font = nullptr;

    //gameLoop
    while (restartGame) {
        restartGame = false;
        //khởi tạo
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            SDL_Log("SDL_Init Error: %s", SDL_GetError());
            return 1;
        }

        if (TTF_Init() != 0) {
            SDL_Log("TTF_Init Error: %s", TTF_GetError());
            return 1;
        }

        int imgFlags = IMG_INIT_PNG;
        if (!(IMG_Init(imgFlags) & imgFlags)) {
            SDL_Log("IMG_Init Error: %s", IMG_GetError());
            SDL_Quit();
            return 1;
        }
        SDL_Log("SDL and SDL_image initialized successfully.");

        // tạo window, renderer
        SDL_Window* window = SDL_CreateWindow("20 Minutes Till Dawn", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
        if (!window) {
            SDL_Log("Failed to create window: %s", SDL_GetError());
            IMG_Quit();
            SDL_Quit();
            return 1;
        
        }
        SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer) {
            SDL_Log("Failed to create rendere: %s", SDL_GetError());
            SDL_DestroyWindow(window);
            IMG_Quit();
            SDL_Quit();
            return 1;
        }
        SDL_Log("Window and Renderer created successfully.");

        // tạo background
        SDL_Texture* background = IMG_LoadTexture(renderer, "assets/images/bg.jpg");
        if (!background) {
            SDL_Log("Can't load background 'assets/images/bg.jpg': %s", IMG_GetError());
            if (renderer) SDL_DestroyRenderer(renderer);
            if (window) SDL_DestroyWindow(window);
            IMG_Quit();
            TTF_Quit();
            SDL_Quit();
            return 1;
        }
        SDL_Log("Background loaded successfully.");

        font = TTF_OpenFont("assets/font/arial.ttf", 32);
        if (!font) {
            SDL_Log("Font load error: %s", TTF_GetError());
            return 1;
        }

        EndMenu endMenu(renderer, font);

        //khai báo player, weapon
        string idleSpritePath = "assets/images/idle_sprite_sheet.png";
        int idleFrames = 6;
        string moveSpritePath = "assets/images/walk_sprite_sheet.png";
        int moveFrames = 8;
        string deathSpritePath = "assets/images/death_sprite_sheet.png";
        int deathFrames = 10;
        string weaponPathRight = "assets/images/weaponR1.png";
        string weaponPathLeft = "assets/images/weaponR2.png";
        Player player(renderer, idleSpritePath, idleFrames, moveSpritePath, moveFrames, deathSpritePath, deathFrames, weaponPathRight, weaponPathLeft);
        SDL_Log("Player object created.");

        const int MAX_BULLETS = 100;
        Bullet bullets[MAX_BULLETS];
        string bulletTexturePath = "assets/images/bullet.png";
        float bulletSpeed = 12.0f;
        Uint32 lastShotTime = 0;
        Uint32 fireDelay = 120;

        //tạo enemies 
        const int MAX_ENEMIES = 10;
        Enemy enemies[MAX_ENEMIES];
        string enemyMoveTexturePath = "assets/images/enemy_move.png";
        int enemyMoveFrames = 6;
        Uint32 lastSpawnTime = 0;
        Uint32 spawnDelay = 2000; // spawn 1 enemy mỗi 2s


        Uint32 gameStartTime = SDL_GetTicks();
        bool quit = false;
        SDL_Event e;
        Uint64 frameStart;
        Uint32 frameTime;
        const int FPS = 60;
        const int frameDelay = 1000 / FPS;
        int mouseX, mouseY;

        while (!quit) {
            frameStart = SDL_GetTicks64();
            Uint32 currentTime = SDL_GetTicks();

            //events
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
                if (e.type == SDL_MOUSEBUTTONDOWN) {
                    if (e.button.button == SDL_BUTTON_LEFT && player.getCurrentState() != Player::STATE_DYING) {
                        if (currentTime >= lastShotTime + fireDelay) {
                            SDL_Point spawnPos = player.getWeaponPivotScreenPosition();
                            SDL_GetMouseState(&mouseX, &mouseY);    

                            double dx = static_cast<double>(mouseX) - spawnPos.x;
                            double dy = static_cast<double>(mouseY) - spawnPos.y;
                            double angleRad = atan2(dy, dx);
                            double angleDeg = angleRad * 180.0 / M_PI;

                            float offset = 20.0f;
                            float spawnX = spawnPos.x + cos(angleRad) * offset;
                            float spawnY = spawnPos.y + sin(angleRad) * offset - 10.0f; // đẩy đạn lên cao hơn

                            for (int i = 0; i < MAX_BULLETS; ++i) {
                                if (!bullets[i].isActive()) {
                                    bullets[i].spawn(renderer, bulletTexturePath, spawnX, spawnY, static_cast<float>(angleDeg), bulletSpeed);
                                    break;
                                }
                            }

                            lastShotTime = currentTime;
                        }
                    }
                }
            }

            //input
            SDL_GetMouseState(&mouseX, &mouseY);
            const Uint8* keystate = SDL_GetKeyboardState(NULL);
            player.handleInput(keystate);

            // update
            player.update(mouseX, mouseY);

            for (int i = 0; i < MAX_BULLETS; ++i) {
                if (bullets[i].isActive()) {
                    bullets[i].update();
                }
            }

            //spawn enemies nếu đủ thời gian
            if (currentTime >= lastSpawnTime + spawnDelay) {
                for (int i = 0; i < MAX_ENEMIES; ++i) {
                    if (!enemies[i].isActive()) {
                        enemies[i].spawn(renderer,
                            enemyMoveTexturePath, enemyMoveFrames,
                            800, 600,
                            player.getPositionRect(),
                            enemies, MAX_ENEMIES);
                        lastSpawnTime = currentTime;
                        break;
                    }
                }
            }


            for (int i = 0; i < MAX_ENEMIES; ++i) {
                if (enemies[i].isActive()) {
                    enemies[i].update(player.getPositionRect());
                    enemies[i].checkBulletCollision(bullets, MAX_BULLETS);
                }
            }

            player.checkCollisionWithEnemies(enemies, MAX_ENEMIES);

            //render
            SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, background, NULL, NULL);
            player.render();
            for (int i = 0; i < MAX_BULLETS; ++i) {
                if (bullets[i].isActive()) {
                    bullets[i].render();
                }
            }
            for (int i = 0; i < MAX_ENEMIES; ++i) {
                if (enemies[i].isActive()) {
                    enemies[i].render();
                }
            }
            //tính thời gian ảo
            Uint32 now = SDL_GetTicks();
            float realTime = (now - gameStartTime) / 1000.0f;
            float fakeTime = realTime * 10.0f;
            int remainingFakeTime = (int)(1200 - fakeTime);
            if (remainingFakeTime < 0) {
                remainingFakeTime = 0;
            }

            //check endMenu
            bool end = false;
            bool won = false;
            if (!end && (remainingFakeTime <= 0 || player.getCurrentState() == Player::STATE_DYING)) {
                end = true;
                won = (remainingFakeTime <= 0);
                int result = endMenu.show(won);
                if (result == EndMenu::RESULT_RESTART) {
                    if (font) {
                        TTF_CloseFont(font);
                        font = nullptr;
                    }
                    SDL_DestroyTexture(background);
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                    TTF_Quit();
                    IMG_Quit();
                    SDL_Quit();
                    restartGame = true;
                    break;
                }
                else {
                    quit = true;
                    continue;
                }
            }


            //chuyển thành mm:ss
            int min = remainingFakeTime / 60;
            int sec = remainingFakeTime % 60;

            ostringstream clock;
            clock << setw(2) << setfill('0') << min << ":"
                << setw(2) << setfill('0') << sec;
            string timerText = clock.str();

            if (font) {
                SDL_Color textColor = { 255, 255, 255 };
                SDL_Surface* textSurface = TTF_RenderText_Solid(font, timerText.c_str(), textColor);
                if (textSurface) {
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                    SDL_Rect textRect = { 650, 20, textSurface->w, textSurface->h };
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                    SDL_DestroyTexture(textTexture);
                    SDL_FreeSurface(textSurface);
                }
            }

            SDL_RenderPresent(renderer);

            //delay
            frameTime = SDL_GetTicks64() - frameStart;
            if (frameDelay > frameTime) {
                SDL_Delay(frameDelay - frameTime);
            }
        }
        SDL_Log("Exiting game loop.");
        SDL_Log("Cleaning up resources...");
        SDL_DestroyTexture(background); background = nullptr;
        SDL_DestroyRenderer(renderer); renderer = nullptr;
        SDL_DestroyWindow(window); window = nullptr;
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        SDL_Log("Cleanup complete. Exiting program.");
    }
    return 0;
}