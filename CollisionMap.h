#pragma once
#include <SDL.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "Texture.h"

class CollisionMap {
private:
    std::vector<std::vector<int>> mCollisionData;
    int mMapWidth;
    int mMapHeight;
    int mTileWidth;
    int mTileHeight;

public:
    CollisionMap(int tileWidth = 16, int tileHeight = 16);
    ~CollisionMap();
    
    bool loadCollisionMap(std::string path);
    bool checkCollision(int x, int y);
    void render(SDL_Renderer* renderer, SDL_Rect* camera, Texture* tileSheet);
    
    int getMapWidth() const { return mMapWidth; }
    int getMapHeight() const { return mMapHeight; }
    int getTileWidth() const { return mTileWidth; }
    int getTileHeight() const { return mTileHeight; }
};