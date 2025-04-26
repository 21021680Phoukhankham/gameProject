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
    mCurrentFrame = 0;
    mFrameCount = 6;      // 6 frame mỗi trạng thái
    mFrameDelay = 5;      // Đã giảm xuống để animation chạy nhanh hơn
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
                break;
            case SDLK_d:
            case SDLK_RIGHT:
                mVelX = 2;
                setState(MOVING);
                break;
            case SDLK_SPACE:
                setState(ATTACKING);
                break;
            case SDLK_RETURN:  // Phím Enter để kích hoạt trạng thái chết
                setState(DEAD);
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
    // Cập nhật vị trí
    mPosX += mVelX;
    mPosY += mVelY;
    
    // Cập nhật frame animation
    mFrameTimer++;
    if (mFrameTimer >= mFrameDelay) {
        mFrameTimer = 0;
        
        // Xử lý số frame khác nhau cho mỗi trạng thái
        int maxFrames = 6; // Mặc định là 6 frame
        
        if (mCurrentState == ATTACKING) {
            maxFrames = 4; // Trạng thái chém chỉ có 4 frame
        } else if (mCurrentState == DEAD) {
            maxFrames = 3; // Trạng thái chết chỉ có 3 frame
        }
        
        mCurrentFrame = (mCurrentFrame + 1) % maxFrames;
        
        // Kiểm tra hoàn thành animation
        if (mCurrentState == ATTACKING && mCurrentFrame >= 3) {
            setState(IDLE); // Chuyển về trạng thái đứng yên sau khi hoàn thành 4 frame
        }
    }
    
    // Trạng thái chết không tự động chuyển về đứng yên, cần người chơi tác động
}

void Player::render(int camX, int camY) {
    int row = 0;
    
    switch (mCurrentState) {
        case IDLE:        // Đứng yên - hàng 1 (index 0)
            row = 0;
            break;
        case MOVING:      // Di chuyển - hàng 2 (index 1)
            row = 1;
            break;
        case ATTACKING:   // Chém - hàng 3 (index 2)
            row = 2;
            // Giới hạn frame của trạng thái chém
            if (mCurrentFrame > 3) mCurrentFrame = 0;
            break;
        case DEAD:        // Chết - hàng 5 (index 4)
            row = 4;
            // Giới hạn frame của trạng thái chết
            if (mCurrentFrame > 2) mCurrentFrame = 0;
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