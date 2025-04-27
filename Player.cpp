#include "Player.h"
#include <iostream>

Player::Player(SDL_Renderer* renderer) {
    mSpriteSheet = new Texture(renderer);
    mRenderer = renderer;
    
    mPosX = 0;
    mPosY = 0;
    mVelX = 0;
    mVelY = 0;
    
    mCurrentState = IDLE;  // Trạng thái ban đầu là đứng yên
    mDirection = RIGHT;    // Hướng mặc định là phải
    mCurrentFrame = 0;
    mFrameCount = 6;      // 6 frame mỗi trạng thái
    mFrameDelay = 5;      // Đã giảm xuống để animation chạy nhanh hơn
    mFrameTimer = 0;
    
    mWidth = 0;  // Sẽ được cập nhật sau khi tải sprite sheet
    mHeight = 0;
    
    // Khởi tạo giá trị mặc định cho hitbox
    mHitboxOffsetX = 0;
    mHitboxOffsetY = 2; // Di chuyển hitbox xuống 2px như yêu cầu
    mHitboxWidth = 0;   // Sẽ được cập nhật sau khi tải sprite
    mHitboxHeight = 0;  // Sẽ được cập nhật sau khi tải sprite
    
    // Khởi tạo giá trị máu
    mMaxHealth = 100;   // Máu tối đa là 100
    mCurrentHealth = mMaxHealth;  // Khởi đầu với máu đầy
    mIsInvincible = false;
    mInvincibleTimer = 0;
}

Player::~Player() {
    if (mSpriteSheet != nullptr) {
        delete mSpriteSheet;
        mSpriteSheet = nullptr;
    }
    mClips.clear();
}

bool Player::loadMedia(std::string path) {
    if (!mSpriteSheet->loadFromFile(path)) {
        std::cout << "Khong the tai sprite sheet nguoi choi: " << path << std::endl;
        return false;
    }
    
    mClips.clear();
    
    // Lấy kích thước thực tế của sprite sheet
    int sheetWidth = mSpriteSheet->getWidth();
    int sheetHeight = mSpriteSheet->getHeight();
    
    // Số lượng frame theo chiều ngang và dọc
    int cols = 6; // 6 cột
    int rows = 5; // 5 hàng
    
    // Tính kích thước của mỗi frame
    mWidth = sheetWidth / cols;
    mHeight = sheetHeight / rows;
    
    // Thiết lập kích thước hitbox mặc định (một nửa kích thước render)
    mHitboxWidth = mWidth;
    mHitboxHeight = mHeight;
    
    std::cout << "Kich thuoc sprite sheet: " << sheetWidth << "x" << sheetHeight << std::endl;
    std::cout << "Kich thuoc moi frame: " << mWidth << "x" << mHeight << std::endl;
    std::cout << "Kich thuoc render (x2): " << mWidth * 2 << "x" << mHeight * 2 << std::endl;
    std::cout << "Hitbox size: " << mHitboxWidth << "x" << mHitboxHeight << std::endl;
    std::cout << "Hitbox offset: " << mHitboxOffsetX << "," << mHitboxOffsetY << std::endl;
    
    // Tạo các clip cho từng frame trong sprite sheet
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
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

void Player::handleEvent(SDL_Event& e) {
    // Xử lý phím R để hồi sinh
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_r) {
        if (mCurrentState == DEAD) {
            forceSetState(IDLE);  // Dùng phương thức mới để bỏ qua kiểm tra
            mCurrentHealth = mMaxHealth; // Phục hồi máu khi hồi sinh
            return;
        }
    }
    
    // Xử lý phím H để hồi máu (để test)
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_h) {
        heal(10); // Hồi 10 máu khi nhấn H
        return;
    }
    
    // Nếu đang trong trạng thái chết và đã ở frame cuối, không xử lý input khác
    if (mCurrentState == DEAD && mCurrentFrame == 2) {
        return;
    }
    
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
        switch (e.key.keysym.sym) {
            case SDLK_w:
            case SDLK_UP:
                mVelY = -2;
                setState(MOVING);
                break;
            case SDLK_s:
            case SDLK_DOWN:
                mVelY = 2;
                setState(MOVING);
                break;
            case SDLK_a:
            case SDLK_LEFT:
                mVelX = -2;
                setState(MOVING);
                setDirection(LEFT); // Đặt hướng quay trái
                break;
            case SDLK_d:
            case SDLK_RIGHT:
                mVelX = 2;
                setState(MOVING);
                setDirection(RIGHT); // Đặt hướng quay phải
                break;
            case SDLK_SPACE:
                setState(ATTACKING);
                break;
            case SDLK_RETURN:  // Phím Enter để kích hoạt trạng thái chết
                takeDamage(mCurrentHealth); // Gây sát thương bằng máu hiện tại để chết
                break;
        }
    }
    else if (e.type == SDL_KEYUP && e.key.repeat == 0) {
        switch (e.key.keysym.sym) {
            case SDLK_w:
            case SDLK_UP:
                mVelY = 0;
                if (mVelX == 0) setState(IDLE);
                break;
            case SDLK_s:
            case SDLK_DOWN:
                mVelY = 0;
                if (mVelX == 0) setState(IDLE);
                break;
            case SDLK_a:
            case SDLK_LEFT:
                mVelX = 0;
                if (mVelY == 0) setState(IDLE);
                break;
            case SDLK_d:
            case SDLK_RIGHT:
                mVelX = 0;
                if (mVelY == 0) setState(IDLE);
                break;
        }
    }
}

void Player::update() {
    // Không tự cập nhật vị trí, đã được xử lý trong Game::update
    // mPosX += mVelX;
    // mPosY += mVelY;
    
    // Cập nhật trạng thái bất tử
    if (mIsInvincible) {
        mInvincibleTimer--;
        if (mInvincibleTimer <= 0) {
            mIsInvincible = false;
        }
    }
    
    // Cập nhật frame animation
    mFrameTimer++;
    if (mFrameTimer >= mFrameDelay) {
        mFrameTimer = 0;
        
        // Xử lý số frame khác nhau cho mỗi trạng thái
        int maxFrames = 6; // Mặc định là 6 frame
        
        if (mCurrentState == ATTACKING) {
            maxFrames = 4; // Trạng thái chém chỉ có 4 frame
            
            // Cập nhật frame cho trạng thái tấn công
            if (mCurrentFrame < maxFrames - 1) {
                mCurrentFrame++;
            } else {
                setState(IDLE); // Chuyển về trạng thái đứng yên sau khi hoàn thành 4 frame
            }
        } 
        else if (mCurrentState == DEAD) {
            maxFrames = 3; // Trạng thái chết chỉ có 3 frame
            
            // Cập nhật frame cho trạng thái chết, dừng ở frame cuối cùng
            if (mCurrentFrame < maxFrames - 1) {
                mCurrentFrame++;
            }
            // Không tăng frame nữa sau khi đạt frame cuối
        }
        else {
            // Các trạng thái khác lặp lại bình thường
            mCurrentFrame = (mCurrentFrame + 1) % maxFrames;
        }
    }
}

void Player::render(int camX, int camY) {
    int row = 0;
    
    switch (mCurrentState) {
        case IDLE:
            row = 0; // Hàng 1
            break;
        case MOVING:
            row = 1; // Hàng 2
            break;
        case ATTACKING:
            row = 2; // Hàng 3
            break;
        case DEAD:
            row = 4; // Hàng 5
            break;
    }
    
    int index = row * 6 + mCurrentFrame; // Luôn có 6 cột trong sprite sheet
    if (index >= 0 && index < (int)mClips.size()) {
        SDL_Rect* currentClip = &mClips[index];
        
        // Vị trí render trên màn hình
        int renderX = mPosX - camX;
        int renderY = mPosY - camY;
        
        // Phóng to kích thước của nhân vật khi hiển thị (x2)
        int renderWidth = mWidth * 2;
        int renderHeight = mHeight * 2;
        
        // Tạo vùng hiển thị
        SDL_Rect renderQuad = {renderX, renderY, renderWidth, renderHeight};
        
        // Hiệu ứng chớp khi bất tử
        if (mIsInvincible && (mInvincibleTimer / 5) % 2 == 0) {
            // Hiệu ứng nhấp nháy khi bất tử, bỏ qua render nhân vật
        } else {
            // Render texture dựa vào hướng
            if (mDirection == RIGHT) {
                // Render bình thường nếu nhìn sang phải
                SDL_RenderCopy(mRenderer, mSpriteSheet->getTexture(), currentClip, &renderQuad);
            } else {
                // Lật ngang nếu nhìn sang trái
                SDL_RenderCopyEx(mRenderer, mSpriteSheet->getTexture(), currentClip, &renderQuad, 
                                0, NULL, SDL_FLIP_HORIZONTAL);
            }
        }
        
        // Hiển thị thanh máu
        renderHealthBar(camX, camY);
    }
}

void Player::renderHealthBar(int camX, int camY) {
    // Vị trí thanh máu (ngay trên đầu người chơi)
    int barX = mPosX - camX;
    int barY = mPosY - camY - 15; // Dịch lên trên 15px
    
    // Kích thước thanh máu
    int barWidth = mWidth * 2; // Rộng bằng nhân vật
    int barHeight = 8; // Cao 8px
    
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

void Player::setState(PlayerState state) {
    // Nếu đang ở trạng thái chết và đã ở frame cuối, không chuyển đổi trạng thái
    if (mCurrentState == DEAD && mCurrentFrame == 2 && state != DEAD) {
        return;
    }
    
    if (mCurrentState != state) {
        mCurrentState = state;
        mCurrentFrame = 0;
        mFrameTimer = 0;
    }
}

// Thêm phương thức mới để bỏ qua kiểm tra khi hồi sinh
void Player::forceSetState(PlayerState state) {
    mCurrentState = state;
    mCurrentFrame = 0;
    mFrameTimer = 0;
    mVelX = 0;
    mVelY = 0;
}

PlayerState Player::getState() {
    return mCurrentState;
}

void Player::setDirection(Direction dir) {
    mDirection = dir;
}

Direction Player::getDirection() {
    return mDirection;
}

int Player::getPosX() {
    return mPosX;
}

int Player::getPosY() {
    return mPosY;
}

void Player::setPosition(int x, int y) {
    mPosX = x;
    mPosY = y;
}

// Phương thức nhận sát thương
bool Player::takeDamage(int damage) {
    // Nếu đang bất tử hoặc đã chết, không nhận sát thương
    if (mIsInvincible || mCurrentState == DEAD) {
        return false;
    }
    
    // Giảm máu
    mCurrentHealth -= damage;
    
    // Kiểm tra nếu hết máu
    if (mCurrentHealth <= 0) {
        mCurrentHealth = 0;
        setState(DEAD);
        return true;
    }
    
    // Nếu còn sống, bật trạng thái bất tử tạm thời
    mIsInvincible = true;
    mInvincibleTimer = 60; // Bất tử trong 60 frame (1 giây ở 60fps)
    
    return true;
}

// Phương thức hồi máu
void Player::heal(int amount) {
    // Không hồi máu nếu đã chết
    if (mCurrentState == DEAD) {
        return;
    }
    
    mCurrentHealth += amount;
    
    // Không vượt quá máu tối đa
    if (mCurrentHealth > mMaxHealth) {
        mCurrentHealth = mMaxHealth;
    }
}