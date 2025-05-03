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
    
    // Khởi tạo hướng mặc định
    mDirection = MONSTER_RIGHT;
    
    mWidth = 0;
    mHeight = 0;
    
    // Khởi tạo giá trị mặc định cho hitbox (sẽ được cập nhật sau trong loadMedia)
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
    
    // Khởi tạo các biến cho quá trình tấn công
    mAttackPhase = ATTACK_COOLDOWN;
    mAttackTimer = 0;
    mAttackDuration = 30; // 0.5 giây ở 60fps
    mAttackCooldown = 60; // 1 giây ở 60fps
    
    mStartPosX = 0;
    mStartPosY = 0;
    mTargetPosX = 0;
    mTargetPosY = 0;
    mOriginalPosX = 0;
    mOriginalPosY = 0;
    mHasDealtDamage = false;
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
    
    // Tính toán kích thước hitbox ở giữa frame
    int hitboxSizeScale = 2; // Quái vật được phóng to 2 lần khi render
    
    // Hitbox nhỏ hơn frame gốc để phù hợp với hình dáng quái vật
    // Chỉ mở rộng sang bên phải và xuống dưới
    mHitboxWidth = mWidth / 2 + 20; // Mở rộng 20px sang bên phải
    mHitboxHeight = mHeight / 2 + 5; // Mở rộng 5px xuống dưới
    
    // Offset để đặt hitbox ở giữa frame khi render
    // Không dịch sang trái thêm như trước đây
    mHitboxOffsetX = (mWidth * hitboxSizeScale - mHitboxWidth) / 2; // Không dịch sang trái nữa
    mHitboxOffsetY = (mHeight * hitboxSizeScale - mHitboxHeight) / 2 + 5; // Dịch xuống dưới 5px
    
    std::cout << "Kích thước sprite sheet quái vật: " << sheetWidth << "x" << sheetHeight << std::endl;
    std::cout << "Kích thước mỗi frame: " << mWidth << "x" << mHeight << std::endl;
    std::cout << "Hitbox size: " << mHitboxWidth << "x" << mHitboxHeight << std::endl;
    std::cout << "Hitbox offset: " << mHitboxOffsetX << "," << mHitboxOffsetY << std::endl;
    
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
    
    // Nếu đang trong trạng thái tấn công, cập nhật quá trình tấn công
    if (mCurrentState == MONSTER_ATTACKING) {
        // Quá trình tấn công được xử lý bởi MonsterManager
        // Do đó không tự cập nhật vị trí hoặc frame ở đây
    } 
    // Đã loại bỏ việc tự cập nhật vị trí trong Monster::update
    // Việc cập nhật vị trí được xử lý bởi MonsterManager
    
    // Nếu đang trong trạng thái bị thương, giảm thời gian cooldown
    if (mHasBeenHit) {
        mHitCooldown--;
        if (mHitCooldown <= 0) {
            mHasBeenHit = false;
        }
    }
    
    // Cập nhật frame animation nếu không đang trong trạng thái tấn công
    if (mCurrentState != MONSTER_ATTACKING) {
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
                default:
                    break;
            }
            
            // Cập nhật frame
            if (maxFrames > 0) {
                mCurrentFrame = (mCurrentFrame + 1) % maxFrames;
            }
        }
    }
}

// Thêm getter và setter cho hướng
void Monster::setDirection(MonsterDirection dir) {
    mDirection = dir;
}

MonsterDirection Monster::getDirection() {
    return mDirection;
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
            // Khi đang trong giai đoạn cooldown, hiển thị animation đứng yên
            if (mAttackPhase == ATTACK_COOLDOWN) {
                row = 0; // Hàng 1 (IDLE)
            } else {
                row = 2; // Hàng 3 (ATTACKING)
            }
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
        
        // Render quái vật dựa vào hướng di chuyển
        if (mDirection == MONSTER_RIGHT) {
            // Render bình thường nếu nhìn sang phải
            SDL_RenderCopy(mRenderer, mSpriteSheet->getTexture(), currentClip, &renderQuad);
        } else {
            // Lật ngang nếu nhìn sang trái
            SDL_RenderCopyEx(mRenderer, mSpriteSheet->getTexture(), currentClip, &renderQuad, 
                             0, NULL, SDL_FLIP_HORIZONTAL);
        }
        
        // Render thanh máu (chỉ khi quái vật còn sống)
        if (mCurrentState != MONSTER_DEAD) {
            renderHealthBar(camX, camY);
        }
        
        // Debug: vẽ hitbox
        SDL_SetRenderDrawColor(mRenderer, 0, 255, 0, 255); // Màu xanh lá
        SDL_Rect hitboxRect = {
            mPosX + mHitboxOffsetX - camX,
            mPosY + mHitboxOffsetY - camY,
            mHitboxWidth,
            mHitboxHeight
        };
        SDL_RenderDrawRect(mRenderer, &hitboxRect);
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

// Phương thức bắt đầu tấn công mới
void Monster::startAttack(int targetX, int targetY) {
    // Nếu đang trong giai đoạn tấn công khác hoặc đã chết, không thực hiện
    if (mCurrentState == MONSTER_DEAD || 
       (mCurrentState == MONSTER_ATTACKING && mAttackPhase != ATTACK_COOLDOWN)) {
        return;
    }
    
    // Lưu vị trí ban đầu trước khi tấn công
    mOriginalPosX = mPosX;
    mOriginalPosY = mPosY;
    
    // Lưu vị trí hiện tại làm điểm bắt đầu
    mStartPosX = mPosX;
    mStartPosY = mPosY;
    
    // Lưu vị trí mục tiêu, điều chỉnh lên trên 20px để quái vật không bị lệch xuống dưới
    // Tính toán hướng di chuyển để lao sâu thêm 5px
    int dirX = (targetX > mPosX) ? 5 : -5; // Thêm 5px vào hướng di chuyển
    
    mTargetPosX = targetX + dirX; // Lao sâu thêm 5px theo hướng di chuyển
    mTargetPosY = targetY - 20; // Điều chỉnh mục tiêu lên trên 20px
    
    // Đặt hướng dựa vào mục tiêu
    if (targetX > mPosX) {
        mDirection = MONSTER_RIGHT;
    } else {
        mDirection = MONSTER_LEFT;
    }
    
    // Bắt đầu giai đoạn chuẩn bị tấn công
    mAttackPhase = ATTACK_CHARGE;
    mAttackTimer = 0;
    mCurrentState = MONSTER_ATTACKING;
    mCurrentFrame = 0; // Bắt đầu từ frame đầu tiên của animation tấn công
    mHasDealtDamage = false;
    
    // Dừng di chuyển trong quá trình tấn công
    mVelX = 0;
    mVelY = 0;
    
    std::cout << "Quái vật bắt đầu chuẩn bị tấn công!" << std::endl;
}

// Phương thức cập nhật quá trình tấn công đã được chuyển sang MonsterManager
// Giữ lại chỉ để tương thích nhưng nội dung trống
void Monster::updateAttack() {
    // Nội dung đã được chuyển sang MonsterManager::updateMonsterAttack
    // Hàm này chỉ giữ lại để tương thích với code cũ
}

// Phương thức cập nhật animation tấn công
void Monster::updateAttackAnimation() {
    mFrameTimer++;
    if (mFrameTimer >= mFrameDelay) {
        mFrameTimer = 0;
        
        // Xác định số frame tối đa cho animation tấn công
        int maxFrames = mAttackingFrames;
        
        if (mAttackPhase == ATTACK_CHARGE) {
            // Sử dụng các frame đầu của animation tấn công
            mCurrentFrame = (int)(mAttackTimer / (mAttackDuration / 2.0f) * (maxFrames / 2.0f));
            if (mCurrentFrame >= maxFrames / 2) mCurrentFrame = maxFrames / 2 - 1;
        } 
        else if (mAttackPhase == ATTACK_LUNGE) {
            // Sử dụng các frame giữa của animation tấn công
            mCurrentFrame = maxFrames / 2 + (int)(mAttackTimer / (float)mAttackDuration * (maxFrames / 2.0f));
            if (mCurrentFrame >= maxFrames) mCurrentFrame = maxFrames - 1;
        }
        else if (mAttackPhase == ATTACK_RETREAT) {
            // Sử dụng đúng các frame của animation tấn công theo thứ tự ngược lại
            // Lấy các frame từ maxFrames-1 về 0 để tạo hiệu ứng tua ngược
            // Đảm bảo tua từ frame cuối cùng của tấn công về frame đầu tiên
            mCurrentFrame = maxFrames - 1 - (int)((float)mAttackTimer / mAttackDuration * (maxFrames - 1));
            if (mCurrentFrame < 0) mCurrentFrame = 0;
        }
        else if (mAttackPhase == ATTACK_COOLDOWN) {
            // Chuyển sang dùng frame của trạng thái đứng yên (IDLE) thay vì frame tấn công
            // Tính toán frame hiện tại dựa trên số lượng frame của trạng thái IDLE
            int idleFrameCount = mIdleFrames;
            mCurrentFrame = (mAttackTimer / 10) % idleFrameCount; // Chuyển frame sau mỗi 10 đơn vị thời gian
        }
    }
}