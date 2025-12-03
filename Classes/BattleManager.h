#ifndef __BATTLE_MANAGER_H__
#define __BATTLE_MANAGER_H__

#include "cocos2d.h"
#include "Building.h"

// 【关键修改】不要在这里 include Troop.h，防止死循环
class Troop;

class BattleManager {
public:
    static BattleManager* getInstance();

    // 建筑管理
    void addBuilding(Building* building);
    void removeBuilding(Building* building);
    Building* findClosestBuilding(cocos2d::Vec2 position);
    Building* findClosestBuildingOfType(cocos2d::Vec2 position, BuildingType type);
    void dealAreaDamage(cocos2d::Vec2 center, float radius, int damage);

    // 兵种管理
    void addTroop(Troop* troop);
    void removeTroop(Troop* troop);
    Troop* findClosestTroop(cocos2d::Vec2 position, float range);

    void clear();

private:
    BattleManager();
    static BattleManager* s_instance;

    cocos2d::Vector<Building*> m_buildings;
    cocos2d::Vector<Troop*> m_troops;
};

#endif // __BATTLE_MANAGER_H__