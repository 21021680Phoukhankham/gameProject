#include "TileMap.h"
#include <iostream>
#include <fstream>

TileMap::TileMap(SDL_Renderer* renderer) {
    mRenderer = renderer;
    mTileSheet = new Texture(renderer);
    mTileWidth = 16;  // Kích thước tile là 16x16
    mTileHeight = 16;
    mMapWidth = 0;
    mMapHeight = 0;
}

TileMap::~TileMap() {
    if (mTileSheet != nullptr) {
        delete mTileSheet;
        mTileSheet = nullptr;
    }
    mTileClips.clear();
}

bool TileMap::loadTileSheet(std::string path) {
    // Tải texture
    if (!mTileSheet->loadFromFile(path)) {
        std::cout << "Không thể tải tileset: " << path << std::endl;
        return false;
    }
    
    // Xóa các clip cũ nếu có
    mTileClips.clear();
    
    // Lấy kích thước của tileset
    int tileSheetWidth = mTileSheet->getWidth();
    int tileSheetHeight = mTileSheet->getHeight();
    
    // Tính số lượng tile theo chiều ngang và dọc
    int numTilesX = tileSheetWidth / mTileWidth;
    int numTilesY = tileSheetHeight / mTileHeight;
    
    // Tạo các clip cho từng tile
    int tileIndex = 0;
    for (int y = 0; y < numTilesY; y++) {
        for (int x = 0; x < numTilesX; x++) {
            SDL_Rect clip = {
                x * mTileWidth,
                y * mTileHeight,
                mTileWidth,
                mTileHeight
            };
            
            mTileClips.push_back(clip);
            tileIndex++;
        }
    }
    
    std::cout << "Đã tải " << tileIndex << " tiles từ spritesheet." << std::endl;
    return true;
}

bool TileMap::loadMap(std::string path) {
    std::ifstream mapFile(path);
    if (!mapFile.is_open()) {
        std::cout << "Không thể mở file map: " << path << std::endl;
        return false;
    }
    
    mMap.clear();
    std::string line;
    
    while (std::getline(mapFile, line)) {
        std::vector<int> row;
        std::istringstream iss(line);
        int tileIndex;
        
        // Đọc các số nguyên từ mỗi dòng
        while (iss >> tileIndex) {
            row.push_back(tileIndex);
        }
        
        if (!row.empty()) {
            mMap.push_back(row);
        }
    }
    
    mapFile.close();
    
    mMapHeight = mMap.size();
    mMapWidth = mMap[0].size();
    
    std::cout << "Đã tải map kích thước " << mMapWidth << "x" << mMapHeight << std::endl;
    return true;
}

void TileMap::render(SDL_Rect* camera) {
    // Nếu không có camera, render toàn bộ map
    if (camera == nullptr) {
        for (int y = 0; y < mMapHeight; y++) {
            for (int x = 0; x < mMapWidth; x++) {
                int tileType = mMap[y][x];
                
                // Chỉ render tile hợp lệ
                if (tileType >= 0 && tileType < (int)mTileClips.size()) {
                    // Xác định vị trí chính xác để vẽ tile
                    int renderX = x * mTileWidth;
                    int renderY = y * mTileHeight;
                    
                    // Render tile từ spritesheet
                    mTileSheet->render(renderX, renderY, &mTileClips[tileType]);
                }
            }
        }
    } else {
        // Tính vị trí bắt đầu và kết thúc của tile trong camera
        int startX = camera->x / mTileWidth;
        int startY = camera->y / mTileHeight;
        
        int endX = (camera->x + camera->w) / mTileWidth + 1;
        int endY = (camera->y + camera->h) / mTileHeight + 1;
        
        // Giới hạn để không vượt quá kích thước map
        if (startX < 0) startX = 0;
        if (startY < 0) startY = 0;
        
        if (endX > mMapWidth) endX = mMapWidth;
        if (endY > mMapHeight) endY = mMapHeight;
        
        // Chỉ render các tile nằm trong camera
        for (int y = startY; y < endY; y++) {
            for (int x = startX; x < endX; x++) {
                int tileType = mMap[y][x];
                
                // Chỉ render tile hợp lệ
                if (tileType >= 0 && tileType < (int)mTileClips.size()) {
                    // Xác định vị trí chính xác để vẽ tile
                    int renderX = x * mTileWidth - camera->x;
                    int renderY = y * mTileHeight - camera->y;
                    
                    // Render tile từ spritesheet
                    mTileSheet->render(renderX, renderY, &mTileClips[tileType]);
                }
            }
        }
    }
}

int TileMap::getTileWidth() {
    return mTileWidth;
}

int TileMap::getTileHeight() {
    return mTileHeight;
}

int TileMap::getMapWidth() {
    return mMapWidth;
}

int TileMap::getMapHeight() {
    return mMapHeight;
}