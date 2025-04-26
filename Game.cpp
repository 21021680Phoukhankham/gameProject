#include "Game.h"
#include <iostream>

Game::Game() {
    mWindow = nullptr;
    mRenderer = nullptr;
    mTileMap = nullptr;
    mPlayer = nullptr;  // Khởi tạo player
    mIsRunning = false;
    mScreenWidth = 1024;
    mScreenHeight = 768;
    mScale = 1.0f;
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
    mPlayer = new Player(mRenderer);  // Tạo đối tượng Player
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
    
    // Tải sprite sheet người chơi
    if (!mPlayer->loadMedia("img\\player.png")) {
        std::cout << "Không thể tải sprite sheet người chơi!" << std::endl;
        return false;
    }
    
    // Đặt vị trí ban đầu cho người chơi ở giữa map
    int mapWidthPixels = mTileMap->getMapWidth() * mTileMap->getTileWidth();
    int mapHeightPixels = mTileMap->getMapHeight() * mTileMap->getTileHeight();
    mPlayer->setPosition(mapWidthPixels / 2, mapHeightPixels / 2);
    
    return true;
}

void Game::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            mIsRunning = false;
        }
        
        // Xử lý event cho người chơi (vẫn giữ để xử lý các phím đặc biệt như SPACE và RETURN)
        mPlayer->handleEvent(e);
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
        velocityChanged = true;
    } 
    else if (currentKeyStates[SDL_SCANCODE_D] || currentKeyStates[SDL_SCANCODE_RIGHT]) {
        mPlayer->setVelocityX(2);
        mPlayer->setState(MOVING);
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

void Game::update() {
    // Cập nhật người chơi
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
    
    // Vẽ người chơi
    mPlayer->render(mCameraX, mCameraY);
    
    // Cập nhật màn hình
    SDL_RenderPresent(mRenderer);
}

void Game::clean() {
    if (mPlayer != nullptr) {
        delete mPlayer;
        mPlayer = nullptr;
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