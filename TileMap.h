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
    std::vector<Texture*> mTileTextures;
    SDL_Renderer* mRenderer;
    
    int mTileWidth;
    int mTileHeight;
    int mMapWidth;
    int mMapHeight;

public:
    TileMap(SDL_Renderer* renderer);
    ~TileMap();
    
    bool loadTiles(std::string basePath);
    bool loadMap(std::string path);
    void render(SDL_Rect* camera = nullptr);
    
    int getTileWidth();
    int getTileHeight();
    int getMapWidth();
    int getMapHeight();
};