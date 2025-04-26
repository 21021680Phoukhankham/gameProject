#pragma once
#include <SDL.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include "Texture.h"

class TileMap {
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
    TileMap(SDL_Renderer* renderer);
    ~TileMap();
    
    bool loadTileSheet(std::string path);
    bool loadMap(std::string path);
    void render(SDL_Rect* camera = nullptr);
    
    int getTileWidth();
    int getTileHeight();
    int getMapWidth();
    int getMapHeight();
};