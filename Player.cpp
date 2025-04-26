#include "Player.h"
#include <iostream>

Player::Player(SDL_Renderer* renderer) {
    mSpriteSheet = new Texture(renderer);
    mRenderer = renderer;
    
    mPosX = 0;
    mPosY = 0;
    mVelX = 0;
    mVelY = 0;
    
    mCurrentState = IDLE;
    mCurrentFrame = 0;
    mFrameCount = 6;
    mFrameDelay = 8;
    mFrameTimer = 0;
    
    mWidth = 0;  // Sẽ được cập nhật sau khi tải sprite sheet
    mHeight = 0;
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
    
    std::cout << "Kich thuoc sprite sheet: " << sheetWidth << "x" << sheetHeight << std::endl;
    std::cout << "Kich thuoc moi frame: " << mWidth << "x" << mHeight << std::endl;
    
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
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
        switch (e.key.keysym.sym) {
            case SDLK_w:
                mVelY = -2;
                break;
            case SDLK_s:
                mVelY = 2;
                break;
            case SDLK_a:
                mVelX = -2;
                setState(MOVING_LEFT);
                break;
            case SDLK_d:
                mVelX = 2;
                setState(MOVING_RIGHT);
                break;
            case SDLK_SPACE:
                setState(ATTACKING);
                break;
        }
    }
    else if (e.type == SDL_KEYUP && e.key.repeat == 0) {
        switch (e.key.keysym.sym) {
            case SDLK_w:
                mVelY = 0;
                break;
            case SDLK_s:
                mVelY = 0;
                break;
            case SDLK_a:
                mVelX = 0;
                if (mCurrentState == MOVING_LEFT)
                    setState(IDLE);
                break;
            case SDLK_d:
                mVelX = 0;
                if (mCurrentState == MOVING_RIGHT)
                    setState(IDLE);
                break;
        }
    }
}

void Player::update() {
    mPosX += mVelX;
    mPosY += mVelY;
    
    mFrameTimer++;
    if (mFrameTimer >= mFrameDelay) {
        mFrameTimer = 0;
        mCurrentFrame = (mCurrentFrame + 1) % mFrameCount;
    }
    
    if (mCurrentState == ATTACKING) {
        if (mCurrentFrame >= mFrameCount - 1) {
            setState(IDLE);
        }
    }
}

void Player::render(int camX, int camY) {
    int row = 0;
    
    switch (mCurrentState) {
        case MOVING_RIGHT:
            row = 0;
            break;
        case MOVING_LEFT:
            row = 1;
            break;
        case ATTACKING:
            row = 2;
            break;
        case DEAD:
            row = 4;
            break;
        case IDLE:
            row = 0;
            mCurrentFrame = 0;
            break;
    }
    
    int index = row * mFrameCount + mCurrentFrame;
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
        
        // Render texture
        SDL_RenderCopy(mRenderer, mSpriteSheet->getTexture(), currentClip, &renderQuad);
    }
}

void Player::setState(PlayerState state) {
    if (mCurrentState != state) {
        mCurrentState = state;
        mCurrentFrame = 0;
        mFrameTimer = 0;
    }
}

PlayerState Player::getState() {
    return mCurrentState;
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