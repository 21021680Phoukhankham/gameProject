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
    
public:
    MonsterManager(SDL_Renderer* renderer, int tileWidth = 16, int tileHeight = 16);
    ~MonsterManager();
    
    bool loadMonsterMap(std::string path);
    void update(SDL_Rect playerHitbox); // Cập nhật trạng thái của tất cả quái vật
    void render(int camX, int camY);
    
    // Kiểm tra va chạm với người chơi
    bool checkCollisionWithPlayer(SDL_Rect playerHitbox);
    
    // Kiểm tra va chạm với tấn công của người chơi
    void checkAttackCollision(SDL_Rect attackHitbox);
};