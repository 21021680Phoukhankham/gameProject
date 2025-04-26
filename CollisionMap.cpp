#include "CollisionMap.h"
#include <iostream>

CollisionMap::CollisionMap(int tileWidth, int tileHeight) {
    mTileWidth = tileWidth;
    mTileHeight = tileHeight;
    mMapWidth = 0;
    mMapHeight = 0;
}

CollisionMap::~CollisionMap() {
    mCollisionData.clear();
}

bool CollisionMap::loadCollisionMap(std::string path) {
    std::ifstream mapFile(path);
    if (!mapFile.is_open()) {
        std::cout << "Không thể mở file collision map: " << path << std::endl;
        return false;
    }
    
    mCollisionData.clear();
    std::string line;
    
    while (std::getline(mapFile, line)) {
        std::vector<int> row;
        std::istringstream iss(line);
        int value;
        
        // Đọc các số nguyên từ mỗi dòng
        while (iss >> value) {
            row.push_back(value);
        }
        
        if (!row.empty()) {
            mCollisionData.push_back(row);
        }
    }
    
    mapFile.close();
    
    mMapHeight = mCollisionData.size();
    if (mMapHeight > 0) {
        mMapWidth = mCollisionData[0].size();
    }
    
    std::cout << "Đã tải collision map kích thước " << mMapWidth << "x" << mMapHeight << std::endl;
    return true;
}

bool CollisionMap::checkCollision(int x, int y) {
    // Chuyển đổi tọa độ pixel sang tọa độ ô
    int tileX = x / mTileWidth;
    int tileY = y / mTileHeight;
    
    // Kiểm tra giới hạn map
    if (tileX < 0 || tileX >= mMapWidth || tileY < 0 || tileY >= mMapHeight) {
        return false;
    }
    
    // Giá trị khác 0 thường đại diện cho ô có va chạm
    return mCollisionData[tileY][tileX] != 0;
}

void CollisionMap::render(SDL_Renderer* renderer, SDL_Rect* camera, Texture* tileSheet) {
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
    
    // Chỉ render các tile nằm trong camera và có giá trị khác 0
    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            if (y < mCollisionData.size() && x < mCollisionData[y].size()) {
                int tileValue = mCollisionData[y][x];
                
                // Chỉ render các ô khác 0
                if (tileValue > 0) {
                    // Xác định vị trí chính xác để vẽ tile
                    int renderX = x * mTileWidth - camera->x;
                    int renderY = y * mTileHeight - camera->y;
                    
                    // Tính toán chỉ số trong tilesheet
                    int tilesPerRow = tileSheet->getWidth() / mTileWidth;
                    int row = tileValue / tilesPerRow;
                    int col = tileValue % tilesPerRow;
                    
                    SDL_Rect clip = {
                        col * mTileWidth,
                        row * mTileHeight,
                        mTileWidth,
                        mTileHeight
                    };
                    
                    // Render tile từ tilesheet
                    tileSheet->render(renderX, renderY, &clip);
                }
            }
        }
    }
}