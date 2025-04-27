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

// Thêm enum MonsterDirection để xác định hướng quái vật
enum MonsterDirection {
    MONSTER_RIGHT,
    MONSTER_LEFT
};

// Thêm enum cho các giai đoạn tấn công
enum MonsterAttackPhase {
    ATTACK_CHARGE,    // Giai đoạn chuẩn bị tấn công
    ATTACK_LUNGE,     // Giai đoạn lao vào người chơi
    ATTACK_RETREAT,   // Giai đoạn lùi lại
    ATTACK_COOLDOWN   // Giai đoạn nghỉ trước khi tấn công tiếp
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
    
    // Thêm biến hướng
    MonsterDirection mDirection;
    
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
    
    // Biến cho quá trình tấn công
    MonsterAttackPhase mAttackPhase;
    int mAttackTimer;         // Đếm thời gian trong mỗi giai đoạn tấn công
    int mAttackDuration;      // Thời gian tối đa của mỗi giai đoạn
    int mAttackCooldown;      // Thời gian nghỉ giữa các đợt tấn công
    
    // Lưu vị trí trước khi tấn công và vị trí mục tiêu
    int mStartPosX;
    int mStartPosY;
    int mTargetPosX;
    int mTargetPosY;
    int mOriginalPosX;        // Vị trí ban đầu trước khi bắt đầu tấn công
    int mOriginalPosY;        // Vị trí ban đầu trước khi bắt đầu tấn công
    bool mHasDealtDamage;     // Đã gây sát thương trong lượt tấn công này chưa
    
public:
    Monster(SDL_Renderer* renderer, MonsterType type);
    ~Monster();
    
    bool loadMedia(std::string path);
    void update();
    void render(int camX, int camY);
    
    void setState(MonsterState state);
    MonsterState getState();
    
    // Thêm getter và setter cho hướng
    void setDirection(MonsterDirection dir);
    MonsterDirection getDirection();
    
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
    
    // Các phương thức mới cho hệ thống tấn công
    void startAttack(int targetX, int targetY);
    void updateAttack();
    MonsterAttackPhase getAttackPhase() const { return mAttackPhase; }
    
    // Các phương thức phụ trợ mới cho hệ thống tấn công
    int getAttackTimer() const { return mAttackTimer; }
    int getAttackDuration() const { return mAttackDuration; }
    int getAttackCooldown() const { return mAttackCooldown; }
    void resetAttackTimer() { mAttackTimer = 0; }
    void updateAttackTimer() { mAttackTimer++; }
    void setAttackPhase(MonsterAttackPhase phase) { mAttackPhase = phase; }
    
    int getStartPosX() const { return mStartPosX; }
    int getStartPosY() const { return mStartPosY; }
    int getTargetPosX() const { return mTargetPosX; }
    int getTargetPosY() const { return mTargetPosY; }
    int getOriginalPosX() const { return mOriginalPosX; }
    int getOriginalPosY() const { return mOriginalPosY; }
    
    void setStartAndTargetPos(int startX, int startY, int targetX, int targetY) {
        mStartPosX = startX;
        mStartPosY = startY;
        mTargetPosX = targetX;
        mTargetPosY = targetY;
    }
    
    bool hasDealtDamage() const { return mHasDealtDamage; }
    void setHasDealtDamage(bool value) { mHasDealtDamage = value; }
    
    // Phương thức cập nhật animation tấn công
    void updateAttackAnimation();
};