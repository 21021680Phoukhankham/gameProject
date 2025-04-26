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

enum Direction {
    RIGHT,
    LEFT
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
    Direction mDirection; // Biến hướng
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
    void forceSetState(PlayerState state); // Thêm phương thức mới để bỏ qua kiểm tra khi hồi sinh
    PlayerState getState();
    
    void setDirection(Direction dir);
    Direction getDirection();
    
    int getPosX();
    int getPosY();
    void setPosition(int x, int y);
    
    // Phương thức để truy xuất và thiết lập vận tốc
    int getVelocityX() { return mVelX; }
    int getVelocityY() { return mVelY; }
    void setVelocityX(int velX) { mVelX = velX; }
    void setVelocityY(int velY) { mVelY = velY; }
    
    // Thêm phương thức kiểm tra hoàn thành animation chết
    bool getDeathAnimationFinished() { return (mCurrentState == DEAD && mCurrentFrame == 2); }
};