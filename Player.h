#pragma once
#include <SDL.h>
#include "Texture.h"
#include <vector>

enum PlayerState {
    IDLE,      // Đứng yên - hàng 1
    MOVING,    // Di chuyển - hàng 2
    ATTACKING, // Chém - hàng 3
    DEAD       // Chết - hàng 5
};

class Player {
private:
    Texture* mSpriteSheet;
    SDL_Renderer* mRenderer;
    std::vector<SDL_Rect> mClips;
    
    int mPosX;
    int mPosY;
    int mVelX;
    int mVelY;
    
    PlayerState mCurrentState;
    int mCurrentFrame;
    int mFrameCount;
    int mFrameDelay;
    int mFrameTimer;
    
    int mWidth;
    int mHeight;
    
public:
    Player(SDL_Renderer* renderer);
    ~Player();
    
    bool loadMedia(std::string path);
    void handleEvent(SDL_Event& e);
    void update();
    void render(int camX, int camY);
    
    void setState(PlayerState state);
    PlayerState getState();
    
    int getPosX();
    int getPosY();
    void setPosition(int x, int y);
};