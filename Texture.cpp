#include "Texture.h"
#include <iostream>

Texture::Texture(SDL_Renderer* renderer) {
    mTexture = nullptr;
    mRenderer = renderer;
    mWidth = 0;
    mHeight = 0;
}

Texture::~Texture() {
    free();
}

void Texture::free() {
    if (mTexture != nullptr) {
        SDL_DestroyTexture(mTexture);
        mTexture = nullptr;
        mWidth = 0;
        mHeight = 0;
    }
}

bool Texture::loadFromFile(std::string path) {
    free();

    SDL_Texture* newTexture = nullptr;
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    
    if (loadedSurface == nullptr) {
        std::cout << "Không thể tải hình ảnh " << path.c_str() << "! SDL_image Error: " << IMG_GetError() << std::endl;
    } else {
        newTexture = SDL_CreateTextureFromSurface(mRenderer, loadedSurface);
        if (newTexture == nullptr) {
            std::cout << "Không thể tạo texture từ " << path.c_str() << "! SDL Error: " << SDL_GetError() << std::endl;
        } else {
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;
        }

        SDL_FreeSurface(loadedSurface);
    }

    mTexture = newTexture;
    return mTexture != nullptr;
}

void Texture::render(int x, int y) {
    SDL_Rect renderQuad = { x, y, mWidth, mHeight };
    SDL_RenderCopy(mRenderer, mTexture, nullptr, &renderQuad);
}

void Texture::render(int x, int y, SDL_Rect* clip) {
    SDL_Rect renderQuad = { x, y, clip ? clip->w : mWidth, clip ? clip->h : mHeight };
    SDL_RenderCopy(mRenderer, mTexture, clip, &renderQuad);
}

int Texture::getWidth() {
    return mWidth;
}

int Texture::getHeight() {
    return mHeight;
}