#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include "Texture.h"
#include "TileMap.h"
#include "Player.h"
#include "CollisionMap.h"

class Game {
private:
    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
    
    bool mIsRunning;
    int mScreenWidth;
    int mScreenHeight;
    float mScale;
    
    // Biến cho camera
    int mCameraX;
    int mCameraY;
    
    TileMap* mTileMap;
    CollisionMap* mCollisionMap;
    Player* mPlayer;

public:
    Game();
    ~Game();
    
    bool init();
    bool loadMedia();
    void handleEvents();
    void update();
    void render();
    void clean();
    
    bool isRunning();
    
private:
    void adjustCamera();
    bool handlePlayerCollision(int nextX, int nextY);
};