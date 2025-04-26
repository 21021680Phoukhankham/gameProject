#pragma once
#include <SDL.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include "Texture.h"

class OverlayMap {
private:
    std::vector<std::vector<int>> mMap;
    std::vector<SDL_Rect> mTileClips;
    Texture* mTileSheet;
    SDL_Renderer* mRenderer;
    
    int mTileWidth;
    int mTileHeight;
    int mMapWidth;
    int mMapHeight;

public:
    OverlayMap(SDL_Renderer* renderer);
    ~OverlayMap();
    
    bool loadTileSheet(std::string path);
    bool loadMap(std::string path);
    void render(SDL_Rect* camera = nullptr);
    
    int getTileWidth();
    int getTileHeight();
    int getMapWidth();
    int getMapHeight();
    
    Texture* getTileSheet() { return mTileSheet; }
};