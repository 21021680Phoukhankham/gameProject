#include "TileMap.h"
#include <iostream>
#include <fstream>

TileMap::TileMap(SDL_Renderer* renderer) {
    mRenderer = renderer;
    mTileWidth = 32;  // Giá trị mặc định có thể được thay đổi khi tải tile
    mTileHeight = 32; // Giá trị mặc định có thể được thay đổi khi tải tile
    mMapWidth = 0;
    mMapHeight = 0;
}

TileMap::~TileMap() {
    for (size_t i = 0; i < mTileTextures.size(); i++) {
        if (mTileTextures[i] != nullptr) {
            delete mTileTextures[i];
            mTileTextures[i] = nullptr;
        }
    }
    mTileTextures.clear();
}

bool TileMap::loadTiles(std::string basePath) {
    // Giải phóng các textures cũ nếu có
    for (size_t i = 0; i < mTileTextures.size(); i++) {
        if (mTileTextures[i] != nullptr) {
            delete mTileTextures[i];
            mTileTextures[i] = nullptr;
        }
    }
    mTileTextures.clear();
    
    // Đặt giá trị mặc định để đảm bảo có tile 0
    Texture* defaultTexture = new Texture(mRenderer);
    mTileTextures.push_back(defaultTexture);
    
    // Tải các tiles từ 1-100 (hoặc cho đến khi không còn file)
    int tileCount = 1; // Bắt đầu từ 1 vì đã có tile 0 mặc định
    for (int i = 1; i <= 100; i++) {
        std::string filePath = basePath + std::to_string(i) + ".png";
        
        // Kiểm tra xem file có tồn tại không (phương pháp nguyên thủy)
        FILE* file = fopen(filePath.c_str(), "r");
        if (file) {
            fclose(file);
            
            Texture* texture = new Texture(mRenderer);
            if (texture->loadFromFile(filePath)) {
                mTileTextures.push_back(texture);
                tileCount++;
                
                // Cập nhật kích thước tile dựa trên kích thước của texture đầu tiên
                if (tileCount == 2) {
                    mTileWidth = texture->getWidth();
                    mTileHeight = texture->getHeight();
                }
            } else {
                delete texture;
            }
        } else {
            // Không tìm thấy file, thoát khỏi vòng lặp
            break;
        }
    }
    
    std::cout << "Đã tải " << tileCount << " tiles." << std::endl;
    return tileCount > 0;
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
                if (tileType >= 0 && tileType < (int)mTileTextures.size() && mTileTextures[tileType] != nullptr) {
                    // Xác định vị trí chính xác để vẽ tile
                    int renderX = x * mTileWidth;
                    int renderY = y * mTileHeight;
                    
                    // Render tile
                    mTileTextures[tileType]->render(renderX, renderY);
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
                if (tileType >= 0 && tileType < (int)mTileTextures.size() && mTileTextures[tileType] != nullptr) {
                    // Xác định vị trí chính xác để vẽ tile
                    int renderX = x * mTileWidth - camera->x;
                    int renderY = y * mTileHeight - camera->y;
                    
                    // Render tile
                    mTileTextures[tileType]->render(renderX, renderY);
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