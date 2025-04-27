#pragma once
#include <SDL.h>
#include "Texture.h"
#include <vector>
#include <string>

enum MonsterState {
    MONSTER_IDLE,       // Đứng yên - hàng 1
    MONSTER_MOVING,     // Di chuyển - hàng 2
    MONSTER_ATTACKING,  // Tấn công - hàng 3
    MONSTER_HURT,       // Bị tấn công - hàng 4
    MONSTER_DEAD        // Chết - hàng 5
};

enum MonsterType {
    SLIME,
    // Thêm các loại quái vật khác ở đây sau này
};

class Monster {
private:
    Texture* mSpriteSheet;
    SDL_Renderer* mRenderer;
    std::vector<SDL_Rect> mClips;
    
    int mPosX;
    int mPosY;
    int mVelX;
    int mVelY;
    
    MonsterState mCurrentState;
    int mCurrentFrame;
    int mFrameDelay;
    int mFrameTimer;
    
    int mWidth;
    int mHeight;
    
    // Thông tin về hitbox
    int mHitboxOffsetX;
    int mHitboxOffsetY;
    int mHitboxWidth;
    int mHitboxHeight;
    
    // Thông tin về máu
    int mMaxHealth;
    int mCurrentHealth;
    bool mHasBeenHit;   // Kiểm tra xem quái vật đã bị đánh gần đây chưa
    int mHitCooldown;   // Đếm thời gian giữa các lần bị đánh
    
    // Loại quái vật
    MonsterType mType;
    
    // Thông tin về số lượng frame cho mỗi trạng thái
    int mIdleFrames;
    int mMovingFrames;
    int mAttackingFrames;
    int mHurtFrames;
    int mDeadFrames;
    
public:
    Monster(SDL_Renderer* renderer, MonsterType type);
    ~Monster();
    
    bool loadMedia(std::string path);
    void update();
    void render(int camX, int camY);
    
    void setState(MonsterState state);
    MonsterState getState();
    
    int getPosX();
    int getPosY();
    void setPosition(int x, int y);
    
    int getVelocityX() { return mVelX; }
    int getVelocityY() { return mVelY; }
    void setVelocityX(int velX) { mVelX = velX; }
    void setVelocityY(int velY) { mVelY = velY; }
    
    bool getDeathAnimationFinished();
    
    int getWidth() const { return mWidth * 2; }
    int getHeight() const { return mHeight * 2; }
    
    // Getter và setter cho hitbox
    int getHitboxOffsetX() const { return mHitboxOffsetX; }
    int getHitboxOffsetY() const { return mHitboxOffsetY; }
    int getHitboxWidth() const { return mHitboxWidth; }
    int getHitboxHeight() const { return mHitboxHeight; }
    
    void setHitboxOffset(int x, int y) { mHitboxOffsetX = x; mHitboxOffsetY = y; }
    void setHitboxSize(int w, int h) { mHitboxWidth = w; mHitboxHeight = h; }
    
    // Phương thức kiểm tra va chạm với player
    bool checkCollisionWithPlayer(SDL_Rect playerHitbox);
    
    // Phương thức xử lý khi bị tấn công
    void takeDamage(int damage);
    
    // Getter cho máu
    int getCurrentHealth() const { return mCurrentHealth; }
    int getMaxHealth() const { return mMaxHealth; }
    
    // Phương thức vẽ thanh máu
    void renderHealthBar(int camX, int camY);
};