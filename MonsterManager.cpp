#include "MonsterManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib> // Cho hàm rand()

MonsterManager::MonsterManager(SDL_Renderer* renderer, int tileWidth, int tileHeight) {
    mRenderer = renderer;
    mTileWidth = tileWidth;
    mTileHeight = tileHeight;
    mCollisionMap = nullptr; // Khởi tạo nullptr
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

// Phương thức kiểm tra va chạm cho quái vật
bool MonsterManager::handleMonsterCollision(Monster* monster, int nextX, int nextY) {
    // Nếu không có collision map, trả về false (không có va chạm)
    if (mCollisionMap == nullptr) {
        return false;
    }
    
    // Tạo SDL_Rect cho quái vật tại vị trí mới
    SDL_Rect monsterRect = {
        nextX + monster->getHitboxOffsetX(),
        nextY + monster->getHitboxOffsetY(),
        monster->getHitboxWidth(),
        monster->getHitboxHeight()
    };
    
    // Sử dụng phương thức kiểm tra va chạm từ CollisionMap
    return mCollisionMap->checkObjectWithMap(monsterRect);
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
        
        // Nếu quái vật đang trong trạng thái tấn công, cập nhật tấn công
        if (monster->getState() == MONSTER_ATTACKING) {
            updateMonsterAttack(monster);
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
        
        // Cập nhật hướng quái vật dựa vào hướng di chuyển
        if (monster->getVelocityX() > 0) {
            monster->setDirection(MONSTER_RIGHT);
        } else if (monster->getVelocityX() < 0) {
            monster->setDirection(MONSTER_LEFT);
        }
        
        if (distanceSquared <= attackRange) {
            // Nếu người chơi trong phạm vi tấn công
            if (monster->getState() != MONSTER_ATTACKING && monster->getState() != MONSTER_HURT) {
                // Tính toán vị trí tấn công
                int targetX = playerCenterX;
                int targetY = playerCenterY;
                
                // Tạo hitbox tạm thời để kiểm tra va chạm tại vị trí tấn công
                SDL_Rect attackHitbox = {
                    targetX + monster->getHitboxOffsetX() - monster->getHitboxWidth() / 2,
                    targetY + monster->getHitboxOffsetY() - monster->getHitboxHeight() / 2,
                    monster->getHitboxWidth(),
                    monster->getHitboxHeight()
                };
                
                // Kiểm tra xem vị trí tấn công có va chạm với map không
                if (!mCollisionMap->checkObjectWithMap(attackHitbox)) {
                    // Nếu không có va chạm, bắt đầu tấn công
                    monster->startAttack(targetX, targetY);
                } else {
                    // Nếu có va chạm, vẫn tấn công nhưng sẽ dừng trước vị trí va chạm
                    // Tìm vị trí gần nhất không có va chạm
                    int dirX = (targetX > monster->getPosX()) ? 1 : -1;
                    int dirY = (targetY > monster->getPosY()) ? 1 : -1;
                    
                    int testX = monster->getPosX();
                    int testY = monster->getPosY();
                    
                    // Di chuyển từng bước nhỏ cho đến khi phát hiện va chạm
                    for (int i = 0; i < 50; i++) {  // Giới hạn số bước
                        testX += dirX;
                        testY += dirY;
                        
                        SDL_Rect testHitbox = {
                            testX + monster->getHitboxOffsetX(),
                            testY + monster->getHitboxOffsetY(),
                            monster->getHitboxWidth(),
                            monster->getHitboxHeight()
                        };
                        
                        if (mCollisionMap->checkObjectWithMap(testHitbox)) {
                            // Nếu phát hiện va chạm, lùi lại một bước
                            testX -= dirX;
                            testY -= dirY;
                            break;
                        }
                    }
                    
                    // Sử dụng vị trí tìm được
                    monster->startAttack(testX, testY);
                }
            }
        } else if (distanceSquared <= detectionRange) {
            // Nếu người chơi trong phạm vi phát hiện nhưng ngoài phạm vi tấn công
            if (monster->getState() != MONSTER_HURT && monster->getState() != MONSTER_ATTACKING) {
                // Di chuyển về phía người chơi
                monster->setState(MONSTER_MOVING);
                
                // Tính toán vận tốc (hướng đến người chơi)
                int velX = (distanceX > 0) ? 1 : ((distanceX < 0) ? -1 : 0);
                int velY = (distanceY > 0) ? 1 : ((distanceY < 0) ? -1 : 0);
                
                monster->setVelocityX(velX);
                monster->setVelocityY(velY);
                
                // Tính vị trí mới theo từng trục
                int currentX = monster->getPosX();
                int currentY = monster->getPosY();
                int nextX = currentX + monster->getVelocityX();
                int nextY = currentY + monster->getVelocityY();
                
                // Kiểm tra va chạm theo trục X
                bool hasCollisionX = false;
                if (monster->getVelocityX() != 0) {
                    hasCollisionX = handleMonsterCollision(monster, nextX, currentY);
                    if (!hasCollisionX) {
                        // Nếu không có va chạm, cập nhật vị trí X
                        monster->setPosition(nextX, currentY);
                    } else {
                        // Nếu có va chạm, dừng di chuyển theo X
                        monster->setVelocityX(0);
                    }
                }
                
                // Lấy vị trí hiện tại sau khi đã cập nhật X (nếu có)
                currentX = monster->getPosX();
                
                // Kiểm tra va chạm theo trục Y
                bool hasCollisionY = false;
                if (monster->getVelocityY() != 0) {
                    hasCollisionY = handleMonsterCollision(monster, currentX, nextY);
                    if (!hasCollisionY) {
                        // Nếu không có va chạm, cập nhật vị trí Y
                        monster->setPosition(currentX, nextY);
                    } else {
                        // Nếu có va chạm, dừng di chuyển theo Y
                        monster->setVelocityY(0);
                    }
                }
                
                // Nếu không thể di chuyển theo cả X và Y, thử tìm đường đi khác
                if (hasCollisionX && hasCollisionY) {
                    // Thử di chuyển theo một hướng ngẫu nhiên
                    int randomDir = rand() % 4; // 0: lên, 1: phải, 2: xuống, 3: trái
                    switch (randomDir) {
                        case 0: monster->setVelocityY(-1); monster->setVelocityX(0); break;
                        case 1: monster->setVelocityX(1); monster->setVelocityY(0); break;
                        case 2: monster->setVelocityY(1); monster->setVelocityX(0); break;
                        case 3: monster->setVelocityX(-1); monster->setVelocityY(0); break;
                    }
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

void MonsterManager::updateMonsterAttack(Monster* monster) {
    if (monster->getState() != MONSTER_ATTACKING) {
        return;
    }
    
    monster->updateAttackTimer();
    
    switch(monster->getAttackPhase()) {
        case ATTACK_CHARGE:
            // Giai đoạn chuẩn bị - không cần kiểm tra va chạm
            if (monster->getAttackTimer() >= monster->getAttackDuration() / 2) {
                monster->setAttackPhase(ATTACK_LUNGE);
                monster->resetAttackTimer();
                std::cout << "Quái vật chuyển sang giai đoạn lao vào người chơi!" << std::endl;
            }
            break;
            
        case ATTACK_LUNGE:
            {
                // Tính toán tỷ lệ hoàn thành (0.0 đến 1.0)
                float progress = (float)monster->getAttackTimer() / monster->getAttackDuration();
                if (progress > 1.0f) progress = 1.0f;
                
                // Tính toán vị trí mới dựa trên tỷ lệ hoàn thành
                int newPosX = monster->getStartPosX() + (int)((monster->getTargetPosX() - monster->getStartPosX()) * progress);
                int newPosY = monster->getStartPosY() + (int)((monster->getTargetPosY() - monster->getStartPosY()) * progress);
                
                // Kiểm tra va chạm tại vị trí mới
                SDL_Rect monsterRect = {
                    newPosX + monster->getHitboxOffsetX(),
                    newPosY + monster->getHitboxOffsetY(),
                    monster->getHitboxWidth(),
                    monster->getHitboxHeight()
                };
                
                bool hasCollision = mCollisionMap->checkObjectWithMap(monsterRect);
                
                if (!hasCollision) {
                    // Nếu không có va chạm, cập nhật vị trí
                    monster->setPosition(newPosX, newPosY);
                } else {
                    // Nếu có va chạm, dừng tại vị trí hiện tại và chuyển sang giai đoạn lùi lại
                    monster->setAttackPhase(ATTACK_RETREAT);
                    monster->resetAttackTimer();
                    std::cout << "Quái vật va chạm với map và chuyển sang giai đoạn lùi lại!" << std::endl;
                    
                    // Đổi vị trí bắt đầu và mục tiêu để lùi lại
                    monster->setStartAndTargetPos(monster->getPosX(), monster->getPosY(), 
                                                monster->getOriginalPosX(), monster->getOriginalPosY());
                }
                
                // Khi tiến triển quá 70%, có thể gây sát thương
                if (progress > 0.7f && !monster->hasDealtDamage()) {
                    monster->setHasDealtDamage(true);
                }
                
                // Khi hoàn thành giai đoạn lao vào, chuyển sang giai đoạn lùi lại
                if (monster->getAttackTimer() >= monster->getAttackDuration()) {
                    monster->setAttackPhase(ATTACK_RETREAT);
                    monster->resetAttackTimer();
                    std::cout << "Quái vật chuyển sang giai đoạn lùi lại!" << std::endl;
                    
                    // Đổi vị trí bắt đầu và mục tiêu để lùi lại
                    monster->setStartAndTargetPos(monster->getPosX(), monster->getPosY(), 
                                                monster->getOriginalPosX(), monster->getOriginalPosY());
                }
            }
            break;
            
        case ATTACK_RETREAT:
            {
                // Tính toán tỷ lệ hoàn thành (0.0 đến 1.0)
                float progress = (float)monster->getAttackTimer() / monster->getAttackDuration();
                if (progress > 1.0f) progress = 1.0f;
                
                // Tính toán vị trí mới dựa trên tỷ lệ hoàn thành
                int newPosX = monster->getStartPosX() + (int)((monster->getTargetPosX() - monster->getStartPosX()) * progress);
                int newPosY = monster->getStartPosY() + (int)((monster->getTargetPosY() - monster->getStartPosY()) * progress);
                
                // Kiểm tra va chạm tại vị trí mới
                SDL_Rect monsterRect = {
                    newPosX + monster->getHitboxOffsetX(),
                    newPosY + monster->getHitboxOffsetY(),
                    monster->getHitboxWidth(),
                    monster->getHitboxHeight()
                };
                
                bool hasCollision = mCollisionMap->checkObjectWithMap(monsterRect);
                
                if (!hasCollision) {
                    // Nếu không có va chạm, cập nhật vị trí
                    monster->setPosition(newPosX, newPosY);
                } else {
                    // Nếu có va chạm, dừng tại vị trí hiện tại và chuyển sang giai đoạn cooldown
                    monster->setAttackPhase(ATTACK_COOLDOWN);
                    monster->resetAttackTimer();
                    std::cout << "Quái vật va chạm với map khi lùi và chuyển sang giai đoạn nghỉ!" << std::endl;
                }
                
                // Khi hoàn thành giai đoạn lùi lại, chuyển sang giai đoạn nghỉ
                if (monster->getAttackTimer() >= monster->getAttackDuration()) {
                    monster->setAttackPhase(ATTACK_COOLDOWN);
                    monster->resetAttackTimer();
                    std::cout << "Quái vật chuyển sang giai đoạn nghỉ!" << std::endl;
                }
            }
            break;
            
        case ATTACK_COOLDOWN:
            // Giai đoạn nghỉ - quái vật đứng yên trước khi có thể tấn công tiếp
            // Animation sẽ hiển thị trạng thái đứng yên
            if (monster->getAttackTimer() >= monster->getAttackCooldown()) {
                // Chuyển về trạng thái đứng yên sau khi kết thúc cooldown
                monster->setState(MONSTER_IDLE);
                std::cout << "Quái vật đã sẵn sàng cho đợt tấn công tiếp theo!" << std::endl;
            }
            break;
    }
    
    // Cập nhật frame animation cho quái vật dựa vào giai đoạn tấn công
    monster->updateAttackAnimation();
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
        // Chỉ kiểm tra va chạm với quái vật còn sống và đang tấn công trong giai đoạn lao vào
        if (monster->getState() != MONSTER_DEAD && 
            monster->getState() == MONSTER_ATTACKING && 
            monster->getAttackPhase() == ATTACK_LUNGE) {
            
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
                // Gây sát thương cho quái vật
                monster->takeDamage(25); // Gây 25 sát thương mỗi lần tấn công
                
                // Dừng di chuyển khi bị tấn công
                monster->setVelocityX(0);
                monster->setVelocityY(0);
            }
        }
    }
}