#ifndef __BATTLE_MANAGER_H__
#define __BATTLE_MANAGER_H__

#include "cocos2d.h"
#include "Building.h"
#include "Troop.h"

class Troop;

class BattleManager
{
private:
    // 构造函数与单例
    BattleManager();                                   // 私有构造函数
    static BattleManager* s_instance;                  // 单例实例指针

    // 成员变量
    cocos2d::Vector<Building*> m_buildings;            // 场上所有建筑
    cocos2d::Vector<Troop*> m_troops;                  // 场上所有士兵
    std::map<TroopType, int> m_availableTroops;        // 存储可投放的兵种数量
    std::map<TroopType, int> m_deployedTroops;         // 记录已投放的兵种数量（用于同步到经营模式）
    std::map<TroopType, int> m_initialTroops;          // 记录战斗开始时的初始兵种数量（用于计算死亡数量）

public:
    // 单例获取
    static BattleManager* getInstance();               // 获取单例

    // 建筑管理
    void addBuilding(Building* building);                                                                // 添加建筑
    void removeBuilding(Building* building);                                                             // 移除建筑
    Building* findClosestBuilding(cocos2d::Vec2 position);                                               // 查找最近的建筑
    Building* findClosestBuildingOfType(cocos2d::Vec2 position, BuildingType type);                      // 查找最近的指定类型建筑
    Building* findBuildingAtPosition(cocos2d::Vec2 position, BuildingType type, float tolerance = 5.0f); // 查找指定位置的建筑
    void dealAreaDamage(cocos2d::Vec2 center, float radius, int damage);                                 // 对范围内敌军造成伤害
    Troop* findClosestTroopForBuilding(Building* building);                                              // 为建筑查找最近的攻击目标
    const cocos2d::Vector<Building*>& getBuildings() const;                                              // 获取场上所有建筑

    // 兵种管理
    void addTroop(Troop* troop);                                                                  // 添加兵种
    void removeTroop(Troop* troop);                                                               // 移除兵种
    Troop* findClosestTroop(cocos2d::Vec2 position, float range, Troop::MovementType typeFilter); // 查找最近的兵种
    void initAvailableTroops(const std::map<TroopType, int>& availableTroops);                    // 初始化可投放兵种数量
    bool canDeployTroop(TroopType type);                                                          // 检查是否还能投放指定兵种
    void deployTroop(TroopType type);                                                             // 投放兵种（减少可用数量）
    int getAvailableTroopCount(TroopType type);                                                   // 获取指定兵种的剩余数量
    const std::map<TroopType, int>& getAllAvailableTroops() const;                                // 获取所有可用兵种数量
    void onTroopDied(TroopType type);                                                             // 记录士兵死亡
    const std::map<TroopType, int>& getDeployedTroops() const;                                    // 获取已投放的兵种数量
    std::map<TroopType, int> getDeathCounts() const;                                              // 获取战斗中死亡的兵种数量
    const cocos2d::Vector<Troop*>& getTroops() const;                                             // 获取场上所有士兵的引用（用于检查是否还有存活单位）

    // 功能函数
    void clear();                                         // 清空所有数据
    void rescaleTroopsForType(TroopType type, int level); // 根据等级调整特定兵种的属性
};

#endif // __BATTLE_MANAGER_H__