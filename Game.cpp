#include "Game.h"
#include <iostream>

Game::Game() {
    mWindow = nullptr;
    mRenderer = nullptr;
    mTileMap = nullptr;
    mIsRunning = false;
    mScreenWidth = 1024;
    mScreenHeight = 768;
    mScale = 2.0f;  // Phóng to mặc định 3.0 lần vì tile size là 16x16
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
    
    // Không thay đổi kích thước cửa sổ, giữ nguyên kích thước cố định
    
    return true;
}

void Game::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            mIsRunning = false;
        }
        // Thêm phím điều khiển camera
        else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_UP:
                    mCameraY -= 10;
                    break;
                case SDLK_DOWN:
                    mCameraY += 10;
                    break;
                case SDLK_LEFT:
                    mCameraX -= 10;
                    break;
                case SDLK_RIGHT:
                    mCameraX += 10;
                    break;
            }
        }
    }
}

void Game::adjustCamera() {
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
    
    // Cập nhật màn hình
    SDL_RenderPresent(mRenderer);
}

void Game::clean() {
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