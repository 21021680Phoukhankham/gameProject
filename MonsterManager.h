#pragma once
#include <SDL.h>
#include <vector>
#include <string>
#include "Monster.h"
#include "CollisionMap.h"

class MonsterManager {
private:
    std::vector<Monster*> mMonsters;
    SDL_Renderer* mRenderer;
    int mTileWidth;
    int mTileHeight;
    CollisionMap* mCollisionMap; // Thêm con trỏ đến collision map
    
public:
    MonsterManager(SDL_Renderer* renderer, int tileWidth = 16, int tileHeight = 16);
    ~MonsterManager();
    
    bool loadMonsterMap(std::string path);
    void update(SDL_Rect playerHitbox); // Cập nhật trạng thái của tất cả quái vật
    void render(int camX, int camY);
    
    // Thêm phương thức để thiết lập collision map
    void setCollisionMap(CollisionMap* collisionMap) { mCollisionMap = collisionMap; }
    
    // Kiểm tra va chạm với người chơi
    bool checkCollisionWithPlayer(SDL_Rect playerHitbox);
    
    // Kiểm tra va chạm với tấn công của người chơi
    void checkAttackCollision(SDL_Rect attackHitbox);
    
    // Thêm phương thức để kiểm tra va chạm của quái vật với collision map
    bool handleMonsterCollision(Monster* monster, int nextX, int nextY);
    
    // Phương thức để cập nhật tấn công của quái vật
    void updateMonsterAttack(Monster* monster);
};