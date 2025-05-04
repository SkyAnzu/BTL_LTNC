#ifndef ENDMENU_H
#define ENDMENU_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

class EndMenu {
public:
    static const int RESULT_NONE = 0;
    static const int RESULT_RESTART = 1;
    static const int RESULT_EXIT = 2;

    EndMenu(SDL_Renderer* renderer, TTF_Font* font);
    ~EndMenu();
    int show(bool won);

private:
    SDL_Renderer* renderer;
    TTF_Font* font;

    SDL_Rect restartButton;
    SDL_Rect exitButton;

    SDL_Texture* createTextTexture(const std::string& text, SDL_Color color, SDL_Rect& outRect);
    bool isInside(int x, int y, const SDL_Rect& rect);
};

#endif

