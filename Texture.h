#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <string>

class Texture {
private:
    SDL_Texture* mTexture;
    SDL_Renderer* mRenderer;
    int mWidth;
    int mHeight;

public:
    Texture(SDL_Renderer* renderer);
    ~Texture();
    
    void free();
    bool loadFromFile(std::string path);
    void render(int x, int y);
    void render(int x, int y, SDL_Rect* clip);
    void render(int x, int y, SDL_Rect* clip, int width, int height);
    
    SDL_Texture* getTexture() { return mTexture; }
    
    int getWidth();
    int getHeight();
};