#include "Monster.h"
#include <iostream>

Monster::Monster(SDL_Renderer* renderer, MonsterType type) {
    mSpriteSheet = new Texture(renderer);
    mRenderer = renderer;
    
    mPosX = 0;
    mPosY = 0;
    mVelX = 0;
    mVelY = 0;
    
    mCurrentState = MONSTER_IDLE;
    mCurrentFrame = 0;
    mFrameDelay = 8;
    mFrameTimer = 0;
    
    mWidth = 0;
    mHeight = 0;
    
    // Khởi tạo giá trị mặc định cho hitbox
    mHitboxOffsetX = 0;
    mHitboxOffsetY = 0;
    mHitboxWidth = 0;
    mHitboxHeight = 0;
    
    // Khởi tạo máu
    mMaxHealth = 100;
    mCurrentHealth = mMaxHealth;
    mHasBeenHit = false;
    mHitCooldown = 0;
    
    // Lưu loại quái vật
    mType = type;
    
    // Khởi tạo số frame cho mỗi trạng thái
    switch (mType) {
        case SLIME:
            mIdleFrames = 4;
            mMovingFrames = 6;
            mAttackingFrames = 7;
            mHurtFrames = 3;
            mDeadFrames = 5;
            mMaxHealth = 100; // Máu của slime
            break;
        default:
            mIdleFrames = 4;
            mMovingFrames = 6;
            mAttackingFrames = 7;
            mHurtFrames = 3;
            mDeadFrames = 5;
            break;
    }
    
    mCurrentHealth = mMaxHealth;
}

Monster::~Monster() {
    if (mSpriteSheet != nullptr) {
        delete mSpriteSheet;
        mSpriteSheet = nullptr;
    }
    mClips.clear();
}

bool Monster::loadMedia(std::string path) {
    if (!mSpriteSheet->loadFromFile(path)) {
        std::cout << "Không thể tải sprite sheet quái vật: " << path << std::endl;
        return false;
    }
    
    mClips.clear();
    
    // Lấy kích thước thực tế của sprite sheet
    int sheetWidth = mSpriteSheet->getWidth();
    int sheetHeight = mSpriteSheet->getHeight();
    
    // Số lượng frame theo chiều ngang, lấy max
    int maxFrames = 7; // Giả sử trạng thái tấn công có nhiều frame nhất (7)
    
    // Tính kích thước của mỗi frame
    mWidth = sheetWidth / maxFrames;
    mHeight = sheetHeight / 5; // 5 hàng trạng thái
    
    // Thiết lập kích thước hitbox mặc định
    mHitboxWidth = mWidth;
    mHitboxHeight = mHeight;
    
    std::cout << "Kích thước sprite sheet quái vật: " << sheetWidth << "x" << sheetHeight << std::endl;
    std::cout << "Kích thước mỗi frame: " << mWidth << "x" << mHeight << std::endl;
    
    // Tạo các clip cho từng frame trong sprite sheet
    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < maxFrames; c++) {
            SDL_Rect clip = {
                c * mWidth,
                r * mHeight,
                mWidth,
                mHeight
            };
            mClips.push_back(clip);
        }
    }
    
    return true;
}

void Monster::update() {
    // Nếu đã chết không cập nhật vị trí
    if (mCurrentState == MONSTER_DEAD) {
        // Chỉ cập nhật animation nếu chưa hoàn thành animation chết
        if (!getDeathAnimationFinished()) {
            mFrameTimer++;
            if (mFrameTimer >= mFrameDelay) {
                mFrameTimer = 0;
                if (mCurrentFrame < mDeadFrames - 1) {
                    mCurrentFrame++;
                }
            }
        }
        return;
    }
    
    // Cập nhật vị trí
    mPosX += mVelX;
    mPosY += mVelY;
    
    // Nếu đang trong trạng thái bị thương, giảm thời gian cooldown
    if (mHasBeenHit) {
        mHitCooldown--;
        if (mHitCooldown <= 0) {
            mHasBeenHit = false;
        }
    }
    
    // Cập nhật frame animation
    mFrameTimer++;
    if (mFrameTimer >= mFrameDelay) {
        mFrameTimer = 0;
        
        // Xử lý số frame khác nhau cho mỗi trạng thái
        int maxFrames = 0;
        
        switch (mCurrentState) {
            case MONSTER_IDLE:
                maxFrames = mIdleFrames;
                break;
            case MONSTER_MOVING:
                maxFrames = mMovingFrames;
                break;
            case MONSTER_ATTACKING:
                maxFrames = mAttackingFrames;
                // Nếu hoàn thành animation tấn công, chuyển về trạng thái idle
                if (mCurrentFrame >= maxFrames - 1) {
                    setState(MONSTER_IDLE);
                    return;
                }
                break;
            case MONSTER_HURT:
                maxFrames = mHurtFrames;
                // Nếu hoàn thành animation bị tấn công, chuyển về trạng thái idle
                if (mCurrentFrame >= maxFrames - 1) {
                    setState(MONSTER_IDLE);
                    return;
                }
                break;
            case MONSTER_DEAD:
                maxFrames = mDeadFrames;
                // Nếu đang ở trạng thái chết, dừng ở frame cuối
                if (mCurrentFrame >= maxFrames - 1) {
                    return;
                }
                break;
        }
        
        // Cập nhật frame
        if (maxFrames > 0) {
            mCurrentFrame = (mCurrentFrame + 1) % maxFrames;
        }
    }
}

void Monster::render(int camX, int camY) {
    int row = 0;
    
    switch (mCurrentState) {
        case MONSTER_IDLE:
            row = 0; // Hàng 1
            break;
        case MONSTER_MOVING:
            row = 1; // Hàng 2
            break;
        case MONSTER_ATTACKING:
            row = 2; // Hàng 3
            break;
        case MONSTER_HURT:
            row = 3; // Hàng 4
            break;
        case MONSTER_DEAD:
            row = 4; // Hàng 5
            break;
    }
    
    int maxFramesPerRow = 7; // Số frame tối đa trên mỗi hàng
    int index = row * maxFramesPerRow + mCurrentFrame;
    
    if (index >= 0 && index < (int)mClips.size()) {
        SDL_Rect* currentClip = &mClips[index];
        
        // Vị trí render trên màn hình
        int renderX = mPosX - camX;
        int renderY = mPosY - camY;
        
        // Phóng to kích thước của quái vật khi hiển thị (x2)
        int renderWidth = mWidth * 2;
        int renderHeight = mHeight * 2;
        
        // Tạo vùng hiển thị
        SDL_Rect renderQuad = {renderX, renderY, renderWidth, renderHeight};
        
        // Render texture
        SDL_RenderCopy(mRenderer, mSpriteSheet->getTexture(), currentClip, &renderQuad);
        
        // Render thanh máu (chỉ khi quái vật còn sống)
        if (mCurrentState != MONSTER_DEAD) {
            renderHealthBar(camX, camY);
        }
    }
}

void Monster::renderHealthBar(int camX, int camY) {
    // Vị trí thanh máu (ngay trên đầu quái vật)
    int barX = mPosX - camX;
    int barY = mPosY - camY - 10; // Dịch lên trên 10px
    
    // Kích thước thanh máu
    int barWidth = mWidth * 2; // Rộng bằng quái vật
    int barHeight = 5; // Cao 5px
    
    // Vẽ viền thanh máu
    SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255); // Màu đen
    SDL_Rect outlineRect = {barX, barY, barWidth, barHeight};
    SDL_RenderDrawRect(mRenderer, &outlineRect);
    
    // Vẽ phần nền thanh máu
    SDL_SetRenderDrawColor(mRenderer, 255, 0, 0, 255); // Màu đỏ
    SDL_Rect backgroundRect = {barX + 1, barY + 1, barWidth - 2, barHeight - 2};
    SDL_RenderFillRect(mRenderer, &backgroundRect);
    
    // Tính toán phần máu hiện tại
    int healthWidth = (int)((float)(barWidth - 2) * ((float)mCurrentHealth / (float)mMaxHealth));
    
    // Vẽ phần máu hiện tại
    SDL_SetRenderDrawColor(mRenderer, 0, 255, 0, 255); // Màu xanh lá
    SDL_Rect healthRect = {barX + 1, barY + 1, healthWidth, barHeight - 2};
    SDL_RenderFillRect(mRenderer, &healthRect);
}

void Monster::setState(MonsterState state) {
    // Nếu đang ở trạng thái chết, không chuyển đổi trạng thái
    if (mCurrentState == MONSTER_DEAD) {
        return;
    }
    
    if (mCurrentState != state) {
        mCurrentState = state;
        mCurrentFrame = 0;
        mFrameTimer = 0;
    }
}

MonsterState Monster::getState() {
    return mCurrentState;
}

int Monster::getPosX() {
    return mPosX;
}

int Monster::getPosY() {
    return mPosY;
}

void Monster::setPosition(int x, int y) {
    mPosX = x;
    mPosY = y;
}

bool Monster::getDeathAnimationFinished() {
    return (mCurrentState == MONSTER_DEAD && mCurrentFrame == mDeadFrames - 1);
}

bool Monster::checkCollisionWithPlayer(SDL_Rect playerHitbox) {
    SDL_Rect monsterHitbox = {
        mPosX + mHitboxOffsetX,
        mPosY + mHitboxOffsetY,
        mHitboxWidth,
        mHitboxHeight
    };
    
    // Kiểm tra va chạm giữa hai đối tượng dựa trên bounding box
    return (monsterHitbox.x < playerHitbox.x + playerHitbox.w && 
            monsterHitbox.x + monsterHitbox.w > playerHitbox.x && 
            monsterHitbox.y < playerHitbox.y + playerHitbox.h && 
            monsterHitbox.y + monsterHitbox.h > playerHitbox.y);
}

void Monster::takeDamage(int damage) {
    // Nếu đang trong trạng thái cooldown hoặc đã chết, không nhận thêm sát thương
    if (mHasBeenHit || mCurrentState == MONSTER_DEAD) {
        return;
    }
    
    // Giảm máu
    mCurrentHealth -= damage;
    
    // Kiểm tra nếu hết máu
    if (mCurrentHealth <= 0) {
        mCurrentHealth = 0;
        setState(MONSTER_DEAD);
    } else {
        // Nếu còn sống, chuyển sang trạng thái bị thương
        setState(MONSTER_HURT);
        
        // Thiết lập cooldown để tránh bị đánh liên tục
        mHasBeenHit = true;
        mHitCooldown = 30; // 30 frame (nửa giây ở 60fps)
    }
}