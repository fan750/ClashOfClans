//BattleManager.h
#ifndef __BATTLE_MANAGER_H__
#define __BATTLE_MANAGER_H__

#include "cocos2d.h"
#include "Building.h"
#include "Troop.h"

class Troop;

class BattleManager
{
public:
    static BattleManager* getInstance();

    // 建筑管理
    void addBuilding(Building* building);
    void removeBuilding(Building* building);
    Building* findClosestBuilding(cocos2d::Vec2 position);
    Building* findClosestBuildingOfType(cocos2d::Vec2 position, BuildingType type);
    void dealAreaDamage(cocos2d::Vec2 center, float radius, int damage);
    Troop* findClosestTroopForBuilding(Building* building);

    // 兵种管理
    void addTroop(Troop* troop);
    void removeTroop(Troop* troop);
    Troop* findClosestTroop(cocos2d::Vec2 position, float range, TroopMovementType typeFilter);
    void initAvailableTroops(const std::map<TroopType, int>& availableTroops); // 初始化可投放兵种数量
    bool canDeployTroop(TroopType type);                                       // 检查是否还能投放指定兵种
    void deployTroop(TroopType type);                                          // 投放兵种（减少可用数量）
    int getAvailableTroopCount(TroopType type);                                // 获取指定兵种的剩余数量
    const std::map<TroopType, int>& getAllAvailableTroops() const;             // 获取所有可用兵种数量
    void onTroopDied(TroopType type);                                          // 记录士兵死亡
    const std::map<TroopType, int>& getDeployedTroops() const;                 // 获取已投放的兵种数量  
    std::map<TroopType, int> getDeathCounts() const;                           // 获取战斗中死亡的兵种数量
    const cocos2d::Vector<Troop*>& getTroops() const;                          // 获取场上所有士兵的引用（用于检查是否还有存活单位）
    void clear();

private:
    BattleManager();
    static BattleManager* s_instance;

    cocos2d::Vector<Building*> m_buildings;
    cocos2d::Vector<Troop*> m_troops;
    std::map<TroopType, int> m_availableTroops; // 存储可投放的兵种数量
    std::map<TroopType, int> m_deployedTroops;  // 记录已投放的兵种数量（用于同步到经营模式）
    std::map<TroopType, int> m_initialTroops;   // 记录战斗开始时的初始兵种数量（用于计算死亡数量）
};

#endif // __BATTLE_MANAGER_H__