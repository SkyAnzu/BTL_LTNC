#include <SDL_ttf.h>
#include <SDL_image.h>

#include "endMenu.h"

EndMenu::EndMenu(SDL_Renderer* renderer, TTF_Font* font)
    : renderer(renderer), font(font) {
    restartButton = { 300, 300, 200, 50 };
    exitButton = { 300, 370, 200, 50 };
}

EndMenu::~EndMenu() {

}

int EndMenu::show(bool won) {
    bool quit = false;
    SDL_Event e;
    SDL_Color white = { 255, 255, 255 };

    SDL_Rect titleRect;
    SDL_Texture* titleText = createTextTexture(won ? "YOU WIN" : "YOU LOSE", white, titleRect);
    titleRect.x = (800 - titleRect.w) / 2;
    titleRect.y = 150;

    SDL_Rect restartTextRect, exitTextRect;
    SDL_Texture* restartText = createTextTexture("Restart", white, restartTextRect);
    SDL_Texture* exitText = createTextTexture("Exit", white, exitTextRect);

    restartTextRect.x = restartButton.x + (restartButton.w - restartTextRect.w) / 2;
    restartTextRect.y = restartButton.y + (restartButton.h - restartTextRect.h) / 2;
    exitTextRect.x = exitButton.x + (exitButton.w - exitTextRect.w) / 2;
    exitTextRect.y = exitButton.y + (exitButton.h - exitTextRect.h) / 2;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return RESULT_EXIT;
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x = e.button.x;
                int y = e.button.y;
                if (isInside(x, y, restartButton)) return RESULT_RESTART;
                if (isInside(x, y, exitButton)) return RESULT_EXIT;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, titleText, NULL, &titleRect);

        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderFillRect(renderer, &restartButton);
        SDL_RenderFillRect(renderer, &exitButton);

        SDL_RenderCopy(renderer, restartText, NULL, &restartTextRect);
        SDL_RenderCopy(renderer, exitText, NULL, &exitTextRect);

        SDL_RenderPresent(renderer);
    }

    return RESULT_NONE;
}

SDL_Texture* EndMenu::createTextTexture(const std::string& text, SDL_Color color, SDL_Rect& outRect) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    outRect.w = surface->w;
    outRect.h = surface->h;
    SDL_FreeSurface(surface);
    return texture;
}

bool EndMenu::isInside(int x, int y, const SDL_Rect& rect) {
    return (x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h);
}
