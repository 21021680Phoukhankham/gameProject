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
    
    // Thêm các biến điều chỉnh hitbox
    int mHitboxOffsetX;   // Độ lệch X của hitbox so với vị trí nhân vật
    int mHitboxOffsetY;   // Độ lệch Y của hitbox so với vị trí nhân vật
    int mHitboxWidth;     // Chiều rộng của hitbox
    int mHitboxHeight;    // Chiều cao của hitbox
    
    // Thêm các biến cho máu
    int mMaxHealth;        // Máu tối đa
    int mCurrentHealth;    // Máu hiện tại
    bool mIsInvincible;    // Trạng thái bất tử tạm thời sau khi bị tấn công
    int mInvincibleTimer;  // Đếm thời gian bất tử
    
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
    
    // Thêm phương thức getter cho kích thước
    int getWidth() const { return mWidth * 2; } // Nhân 2 vì khi render đã phóng to kích thước lên 2 lần
    int getHeight() const { return mHeight * 2; }
    
    // Thêm getter và setter cho các biến hitbox
    int getHitboxOffsetX() const { return mHitboxOffsetX; }
    int getHitboxOffsetY() const { return mHitboxOffsetY; }
    int getHitboxWidth() const { return mHitboxWidth; }
    int getHitboxHeight() const { return mHitboxHeight; }
    
    void setHitboxOffset(int x, int y) { mHitboxOffsetX = x; mHitboxOffsetY = y; }
    void setHitboxSize(int w, int h) { mHitboxWidth = w; mHitboxHeight = h; }
    
    // Thêm các phương thức cho hệ thống máu
    int getMaxHealth() const { return mMaxHealth; }
    int getCurrentHealth() const { return mCurrentHealth; }
    
    void setMaxHealth(int health) { mMaxHealth = health; }
    void setCurrentHealth(int health) { mCurrentHealth = health; }
    
    // Thêm phương thức nhận sát thương
    bool takeDamage(int damage);
    
    // Phục hồi máu
    void heal(int amount);
    
    // Hiển thị thanh máu
    void renderHealthBar(int camX, int camY);
    
    // Kiểm tra xem người chơi có đang bất tử không
    bool isInvincible() const { return mIsInvincible; }
};