#include "Game.h"
#include <iostream>

Game::Game() {
    mWindow = nullptr;
    mRenderer = nullptr;
    mTileMap = nullptr;
    mCollisionMap = nullptr;
    mPlayer = nullptr;
    mIsRunning = false;
    mScreenWidth = 1024;
    mScreenHeight = 768;
    mScale = 2.0f;
    mCameraX = 0;
    mCameraY = 0;
}

Game::~Game() {
    clean();
}

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cout << "SDL_image không thể khởi tạo! SDL_image Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    // Tạo cửa sổ có kích thước cố định
    mWindow = SDL_CreateWindow("Game Tile Map SDL2", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        mScreenWidth, mScreenHeight, SDL_WINDOW_SHOWN);
    
    if (mWindow == nullptr) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED);
    if (mRenderer == nullptr) {
        std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(mWindow);
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    mTileMap = new TileMap(mRenderer);
    mCollisionMap = new CollisionMap();
    mPlayer = new Player(mRenderer);
    mIsRunning = true;
    return true;
}

bool Game::loadMedia() {
    // Tải tileset
    if (!mTileMap->loadTileSheet("map\\Map2\\map.png")) {
        std::cout << "Không thể tải tileset!" << std::endl;
        return false;
    }
    
    // Tải map
    if (!mTileMap->loadMap("map\\Map2\\map1.txt")) {
        std::cout << "Không thể tải map!" << std::endl;
        return false;
    }
    
    // Tải collision map
    if (!mCollisionMap->loadCollisionMap("map\\Map2\\collision.txt")) {
        std::cout << "Không thể tải collision map!" << std::endl;
        return false;
    }
    
    // Tải sprite sheet người chơi
    if (!mPlayer->loadMedia("img\\player.png")) {
        std::cout << "Không thể tải sprite sheet người chơi!" << std::endl;
        return false;
    }
    
    // Đặt vị trí ban đầu cho người chơi tại tọa độ (100, 100)
    mPlayer->setPosition(100, 100);
    
    return true;
}

void Game::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            mIsRunning = false;
        }
        
        // Xử lý event cho người chơi (bao gồm cả phím R để hồi sinh)
        mPlayer->handleEvent(e);
    }
    
    // Nếu người chơi đã chết và ở frame cuối, không xử lý input di chuyển
    if (mPlayer->getState() == DEAD && mPlayer->getDeathAnimationFinished()) {
        return;
    }
    
    // Kiểm tra trạng thái phím hiện tại để cập nhật liên tục
    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
    
    // Biến tạm để theo dõi thay đổi vận tốc
    bool velocityChanged = false;
    
    // Kiểm tra phím di chuyển
    if (currentKeyStates[SDL_SCANCODE_W] || currentKeyStates[SDL_SCANCODE_UP]) {
        mPlayer->setVelocityY(-2);
        mPlayer->setState(MOVING);
        velocityChanged = true;
    } 
    else if (currentKeyStates[SDL_SCANCODE_S] || currentKeyStates[SDL_SCANCODE_DOWN]) {
        mPlayer->setVelocityY(2);
        mPlayer->setState(MOVING);
        velocityChanged = true;
    } 
    else {
        mPlayer->setVelocityY(0);
    }
    
    if (currentKeyStates[SDL_SCANCODE_A] || currentKeyStates[SDL_SCANCODE_LEFT]) {
        mPlayer->setVelocityX(-2);
        mPlayer->setState(MOVING);
        mPlayer->setDirection(LEFT); // Đặt hướng quay trái
        velocityChanged = true;
    } 
    else if (currentKeyStates[SDL_SCANCODE_D] || currentKeyStates[SDL_SCANCODE_RIGHT]) {
        mPlayer->setVelocityX(2);
        mPlayer->setState(MOVING);
        mPlayer->setDirection(RIGHT); // Đặt hướng quay phải
        velocityChanged = true;
    } 
    else {
        mPlayer->setVelocityX(0);
    }
    
    // Kiểm tra phím Space cho tấn công
    if (currentKeyStates[SDL_SCANCODE_SPACE]) {
        if (mPlayer->getState() != ATTACKING) {
            mPlayer->setState(ATTACKING);
        }
    }
    
    // Kiểm tra phím Enter cho trạng thái chết
    if (currentKeyStates[SDL_SCANCODE_RETURN]) {
        if (mPlayer->getState() != DEAD) {
            mPlayer->setState(DEAD);
        }
    }
    
    // Kiểm tra phím R cho hồi sinh (thêm ở đây để có thể phát hiện được phím nhấn liên tục)
    if (currentKeyStates[SDL_SCANCODE_R]) {
        if (mPlayer->getState() == DEAD) {
            mPlayer->forceSetState(IDLE);
        }
    }
    
    // Nếu không có phím di chuyển nào được nhấn và không đang tấn công hoặc chết
    if (!velocityChanged && mPlayer->getState() == MOVING) {
        // Chỉ chuyển về IDLE khi không di chuyển và đang ở trạng thái MOVING
        if (mPlayer->getVelocityX() == 0 && mPlayer->getVelocityY() == 0) {
            mPlayer->setState(IDLE);
        }
    }
}

void Game::adjustCamera() {
    // Cập nhật camera để theo dõi người chơi ở giữa màn hình
    mCameraX = mPlayer->getPosX() - mScreenWidth / mScale / 2;
    mCameraY = mPlayer->getPosY() - mScreenHeight / mScale / 2;
    
    // Tính kích thước map (pixel)
    int mapWidth = mTileMap->getMapWidth() * mTileMap->getTileWidth();
    int mapHeight = mTileMap->getMapHeight() * mTileMap->getTileHeight();
    
    // Giới hạn camera để không vượt quá biên của map
    if (mCameraX < 0) mCameraX = 0;
    if (mCameraY < 0) mCameraY = 0;
    
    int maxX = mapWidth - mScreenWidth / mScale;
    int maxY = mapHeight - mScreenHeight / mScale;
    
    if (maxX < 0) maxX = 0;
    if (maxY < 0) maxY = 0;
    
    if (mCameraX > maxX) mCameraX = maxX;
    if (mCameraY > maxY) mCameraY = maxY;
}

bool Game::handlePlayerCollision(int nextX, int nextY) {
    // Tạo SDL_Rect cho nhân vật tại vị trí mới
    SDL_Rect playerRect = {
        nextX,
        nextY,
        mPlayer->getWidth(),
        mPlayer->getHeight()
    };
    
    // Sử dụng phương thức kiểm tra va chạm mới
    return mCollisionMap->checkObjectWithMap(playerRect);
}

void Game::update() {
    // Nếu người chơi đã chết và ở frame cuối, không cập nhật
    if (mPlayer->getState() == DEAD && mPlayer->getDeathAnimationFinished()) {
        return;
    }
    
    // Lưu vị trí hiện tại
    int currentX = mPlayer->getPosX();
    int currentY = mPlayer->getPosY();
    
    // Debug: Kiểm tra vị trí hiện tại của nhân vật
    int tileX = currentX / mTileMap->getTileWidth();
    int tileY = currentY / mTileMap->getTileHeight();
    if (tileX >= 0 && tileX < mCollisionMap->getMapWidth() && 
        tileY >= 0 && tileY < mCollisionMap->getMapHeight()) {
        // In ra tọa độ tile
        std::cout << "Player at tile (" << tileX << "," << tileY << ")" << std::endl;
    }
    
    // Tính vị trí tiếp theo dựa trên vận tốc
    int nextX = currentX + mPlayer->getVelocityX();
    int nextY = currentY + mPlayer->getVelocityY();
    
    // Kiểm tra va chạm cho chuyển động theo hướng X
    if (mPlayer->getVelocityX() != 0) {
        SDL_Rect nextPosX = {
            nextX,
            currentY,
            mPlayer->getWidth() / 2, // Giảm kích thước hitbox xuống một nửa
            mPlayer->getHeight() / 2
        };
        
        // Căn chỉnh vị trí hitbox vào giữa nhân vật
        nextPosX.x += mPlayer->getWidth() / 4;
        nextPosX.y += mPlayer->getHeight() / 4;
        
        bool hasCollisionX = mCollisionMap->checkObjectWithMap(nextPosX);
        
        if (!hasCollisionX) {
            // Không có va chạm theo hướng X
            mPlayer->setPosition(nextX, currentY);
        }
    }
    
    // Kiểm tra va chạm cho chuyển động theo hướng Y
    if (mPlayer->getVelocityY() != 0) {
        SDL_Rect nextPosY = {
            mPlayer->getPosX(), // Cập nhật lại vị trí X mới nếu đã di chuyển
            nextY,
            mPlayer->getWidth() / 2, // Giảm kích thước hitbox xuống một nửa
            mPlayer->getHeight() / 2
        };
        
        // Căn chỉnh vị trí hitbox vào giữa nhân vật
        nextPosY.x += mPlayer->getWidth() / 4;
        nextPosY.y += mPlayer->getHeight() / 4;
        
        bool hasCollisionY = mCollisionMap->checkObjectWithMap(nextPosY);
        
        if (!hasCollisionY) {
            // Không có va chạm theo hướng Y
            mPlayer->setPosition(mPlayer->getPosX(), nextY);
        }
    }
    
    // Cập nhật animation và trạng thái của người chơi
    mPlayer->update();
    
    // Điều chỉnh camera
    adjustCamera();
}

void Game::render() {
    // Xóa màn hình
    SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
    SDL_RenderClear(mRenderer);
    
    // Thiết lập viewport cho camera
    SDL_Rect viewport = {0, 0, mScreenWidth, mScreenHeight};
    SDL_RenderSetViewport(mRenderer, &viewport);
    
    // Thiết lập tỷ lệ render
    SDL_RenderSetScale(mRenderer, mScale, mScale);
    
    // Thiết lập vị trí camera
    SDL_RenderSetLogicalSize(mRenderer, mScreenWidth / mScale, mScreenHeight / mScale);
    
    // Tạo viewport cho camera
    SDL_Rect camera = {
        mCameraX, 
        mCameraY, 
        static_cast<int>(mScreenWidth / mScale), 
        static_cast<int>(mScreenHeight / mScale)
    };
    
    // Vẽ map với camera
    mTileMap->render(&camera);
    
    // Vẽ collision map lên trên cùng
    mCollisionMap->render(mRenderer, &camera, mTileMap->getTileSheet());
    
    // Vẽ người chơi
    mPlayer->render(mCameraX, mCameraY);
    
    // Vẽ hitbox của người chơi để debug (chỉ vẽ khi debug)
    SDL_SetRenderDrawColor(mRenderer, 255, 0, 0, 255); // Màu đỏ
    SDL_Rect hitbox = {
        mPlayer->getPosX() - mCameraX + mPlayer->getWidth()/4,
        mPlayer->getPosY() - mCameraY + mPlayer->getHeight()/4,
        mPlayer->getWidth()/2,
        mPlayer->getHeight()/2
    };
    SDL_RenderDrawRect(mRenderer, &hitbox);
    
    // Cập nhật màn hình
    SDL_RenderPresent(mRenderer);
}

void Game::clean() {
    if (mPlayer != nullptr) {
        delete mPlayer;
        mPlayer = nullptr;
    }
    
    if (mCollisionMap != nullptr) {
        delete mCollisionMap;
        mCollisionMap = nullptr;
    }
    
    if (mTileMap != nullptr) {
        delete mTileMap;
        mTileMap = nullptr;
    }
    
    if (mRenderer != nullptr) {
        SDL_DestroyRenderer(mRenderer);
        mRenderer = nullptr;
    }
    
    if (mWindow != nullptr) {
        SDL_DestroyWindow(mWindow);
        mWindow = nullptr;
    }
    
    IMG_Quit();
    SDL_Quit();
}

bool Game::isRunning() {
    return mIsRunning;
}