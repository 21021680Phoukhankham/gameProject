#include "MonsterManager.h"
#include <iostream>
#include <fstream>
#include <sstream>

MonsterManager::MonsterManager(SDL_Renderer* renderer, int tileWidth, int tileHeight) {
    mRenderer = renderer;
    mTileWidth = tileWidth;
    mTileHeight = tileHeight;
}

MonsterManager::~MonsterManager() {
    // Giải phóng bộ nhớ của tất cả quái vật
    for (auto monster : mMonsters) {
        delete monster;
    }
    mMonsters.clear();
}

bool MonsterManager::loadMonsterMap(std::string path) {
    std::ifstream mapFile(path);
    if (!mapFile.is_open()) {
        std::cout << "Không thể mở file monster map: " << path << std::endl;
        return false;
    }
    
    // Xóa danh sách quái vật cũ
    for (auto monster : mMonsters) {
        delete monster;
    }
    mMonsters.clear();
    
    // Đọc file map
    std::string line;
    int row = 0;
    
    while (std::getline(mapFile, line)) {
        std::istringstream iss(line);
        int value;
        int col = 0;
        
        // Đọc các số nguyên từ mỗi dòng
        while (iss >> value) {
            // Nếu giá trị là 1, tạo một quái vật Slime tại vị trí này
            if (value == 1) {
                Monster* monster = new Monster(mRenderer, SLIME);
                
                // Tải sprite sheet
                if (!monster->loadMedia("img\\slime.png")) {
                    std::cout << "Không thể tải sprite sheet cho quái vật Slime" << std::endl;
                    delete monster;
                    continue;
                }
                
                // Đặt vị trí ban đầu (chuyển đổi từ tọa độ tile sang tọa độ pixel)
                int posX = col * mTileWidth;
                int posY = row * mTileHeight;
                monster->setPosition(posX, posY);
                
                // Thiết lập hitbox cho quái vật
                monster->setHitboxOffset(mTileWidth / 4, mTileHeight / 4);
                monster->setHitboxSize(mTileWidth / 2, mTileHeight / 2);
                
                // Thêm quái vật vào danh sách
                mMonsters.push_back(monster);
                
                std::cout << "Đã tạo quái vật Slime tại vị trí (" << col << ", " << row << ")" << std::endl;
            }
            col++;
        }
        row++;
    }
    
    mapFile.close();
    std::cout << "Đã tải " << mMonsters.size() << " quái vật từ file map." << std::endl;
    return true;
}

void MonsterManager::update(SDL_Rect playerHitbox) {
    // Cập nhật trạng thái của tất cả quái vật
    for (auto monster : mMonsters) {
        // Cập nhật animation
        monster->update();
        
        // Nếu quái vật đã chết, bỏ qua xử lý AI
        if (monster->getState() == MONSTER_DEAD) {
            continue;
        }
        
        // Tính toán hướng từ quái vật đến người chơi
        int monsterCenterX = monster->getPosX() + monster->getWidth() / 2;
        int monsterCenterY = monster->getPosY() + monster->getHeight() / 2;
        
        int playerCenterX = playerHitbox.x + playerHitbox.w / 2;
        int playerCenterY = playerHitbox.y + playerHitbox.h / 2;
        
        // Khoảng cách giữa quái vật và người chơi
        int distanceX = playerCenterX - monsterCenterX;
        int distanceY = playerCenterY - monsterCenterY;
        
        // Khoảng cách bình phương (để so sánh mà không cần tính căn bậc hai)
        int distanceSquared = distanceX * distanceX + distanceY * distanceY;
        
        // Phạm vi phát hiện người chơi (300 pixel)
        int detectionRange = 300 * 300;
        
        // Phạm vi tấn công (100 pixel)
        int attackRange = 100 * 100;
        
        if (distanceSquared <= detectionRange) {
            // Nếu người chơi trong phạm vi phát hiện
            
            if (distanceSquared <= attackRange) {
                // Nếu người chơi trong phạm vi tấn công
                if (monster->getState() != MONSTER_ATTACKING && monster->getState() != MONSTER_HURT) {
                    // Dừng di chuyển và tấn công
                    monster->setVelocityX(0);
                    monster->setVelocityY(0);
                    monster->setState(MONSTER_ATTACKING);
                }
            } else {
                // Nếu người chơi trong phạm vi phát hiện nhưng ngoài phạm vi tấn công
                if (monster->getState() != MONSTER_HURT && monster->getState() != MONSTER_ATTACKING) {
                    // Di chuyển về phía người chơi
                    monster->setState(MONSTER_MOVING);
                    
                    // Tính toán vận tốc (hướng đến người chơi)
                    int velX = (distanceX > 0) ? 1 : ((distanceX < 0) ? -1 : 0);
                    int velY = (distanceY > 0) ? 1 : ((distanceY < 0) ? -1 : 0);
                    
                    monster->setVelocityX(velX);
                    monster->setVelocityY(velY);
                }
            }
        } else {
            // Nếu người chơi ngoài phạm vi phát hiện, quái vật đứng yên
            if (monster->getState() != MONSTER_IDLE && monster->getState() != MONSTER_HURT) {
                monster->setState(MONSTER_IDLE);
                monster->setVelocityX(0);
                monster->setVelocityY(0);
            }
        }
    }
}

void MonsterManager::render(int camX, int camY) {
    // Render tất cả quái vật
    for (auto monster : mMonsters) {
        monster->render(camX, camY);
    }
}

bool MonsterManager::checkCollisionWithPlayer(SDL_Rect playerHitbox) {
    // Kiểm tra va chạm giữa người chơi và tất cả quái vật
    for (auto monster : mMonsters) {
        // Chỉ kiểm tra va chạm với quái vật còn sống và đang tấn công
        if (monster->getState() != MONSTER_DEAD && monster->getState() == MONSTER_ATTACKING) {
            if (monster->checkCollisionWithPlayer(playerHitbox)) {
                return true;
            }
        }
    }
    return false;
}

void MonsterManager::checkAttackCollision(SDL_Rect attackHitbox) {
    // Kiểm tra va chạm giữa tấn công của người chơi và tất cả quái vật
    for (auto monster : mMonsters) {
        // Chỉ kiểm tra va chạm với quái vật còn sống
        if (monster->getState() != MONSTER_DEAD) {
            SDL_Rect monsterHitbox = {
                monster->getPosX() + monster->getHitboxOffsetX(),
                monster->getPosY() + monster->getHitboxOffsetY(),
                monster->getHitboxWidth(),
                monster->getHitboxHeight()
            };
            
            // Kiểm tra va chạm
            bool hasCollision = (attackHitbox.x < monsterHitbox.x + monsterHitbox.w && 
                             attackHitbox.x + attackHitbox.w > monsterHitbox.x && 
                             attackHitbox.y < monsterHitbox.y + monsterHitbox.h && 
                             attackHitbox.y + attackHitbox.h > monsterHitbox.y);
            
            if (hasCollision) {
                // Quái vật bị tấn công
                monster->setState(MONSTER_HURT);
                monster->setVelocityX(0);
                monster->setVelocityY(0);
            }
        }
    }
}