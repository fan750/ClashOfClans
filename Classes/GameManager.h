//GameManager.h
#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

#include <chrono>
#include <map>
#include <vector>
#include <string>
#include "cocos2d.h"
#include "Building.h" // 需要引用 BuildingType
#include "Troop.h"    // 需要引用 TroopType

// 定义一个结构体，专门存建筑的“档案”
struct BuildingData
{
    BuildingType type;
    cocos2d::Vec2 position;
    int level; // 新增：记录建筑等级
    int barrackLevel;    // 军营等级（仅对军营有效）
};

struct PendingBuildingUpgrade
{
    int id;
    BuildingType type;
    cocos2d::Vec2 position;
    std::chrono::steady_clock::time_point endTime;
};

class GameManager {
public:
    static GameManager* getInstance();

    // 初始化账户 (带防重置检查)
    void initAccount(int gold, int elixir);
    bool isInitialized() const { return m_isInitialized; }


    // 保存一个建筑
    void addHomeBuilding(BuildingType type, cocos2d::Vec2 pos, int level = 1);

    // 获取所有保存的建筑
    const std::vector<BuildingData>& getHomeBuildings();

    // 新增：更新已保存建筑的等级（用于升级时同步）
    void updateHomeBuildingLevel(BuildingType type, cocos2d::Vec2 pos, int level);

    // 获取当前军营等级
    int getBarrackLevel() const;

    // 更新军营等级
    void updateBarrackLevel(cocos2d::Vec2 pos, int level);

    // 【修改】增加资源 (带上限检查)
    void addGold(int amount);
    void addElixir(int amount);

    // 修改资源上限 (当建造/升级圣水瓶时调用)
    void modifyMaxGold(int amount);
    void modifyMaxElixir(int amount);

    int getGold() const { return m_gold; }
    int getElixir() const { return m_elixir; }

    // 获取上限 (用于UI显示，比如 1000/2000)
    int getMaxGold() const { return m_maxGold; }
    int getMaxElixir() const { return m_maxElixir; }

    void addTroops(TroopType type, int amount);
    int getTroopCount(TroopType type) const;
    void consumeTroops(TroopType type, int amount);
    void setTroopCount(TroopType type, int count);
    const std::map<TroopType, int>& getAllTroopCounts() const;
    void clear();
    int getTown_Hall_Level() { return town_hall_level; }
    void setTown_Hall_Level(int level) { town_hall_level = level; }

    int  getBarLevel() { return bar_level; }
    void setBarLevel(int level) { bar_level = level; }
    int getGiantLevel() { return giant_level; }
    void setGiantLevel(int level) { giant_level = level; }
    int getArcLevel() { return arc_level; }
    void setArcLevel(int level) {  arc_level = level; }
    int getBomLevel() { return bom_level; }
    void setBomLevel(int level) { bom_level = level; }
    int getDragonLevel() { return dragon_level; }
    void setDragonLevel(int level) { dragon_level = level; }

    void setTimeAccelerateCooldownEnd(const std::chrono::steady_clock::time_point& end);
    std::chrono::steady_clock::time_point getTimeAccelerateCooldownEnd() const;
    float getTimeAccelerateCooldownRemaining() const;

    // 建筑升级调度
    void scheduleBuildingUpgrade(Building* building, float duration);
    void completeBuildingUpgrade(int taskId);
    bool hasPendingUpgrade(BuildingType type, cocos2d::Vec2 position, float tolerance = 5.0f) const;

    // 【新增】应用建筑升级加速
    void applyBuildingUpgradeBoost(BuildingType type, cocos2d::Vec2 position, float multiplier, float duration);

private:
    void ensureUpgradeTicker();
    void updatePendingUpgrades(float dt);

    GameManager();
    static GameManager* s_instance;
    std::vector<BuildingData> m_homeBuildings;
    std::map<TroopType, int> m_troopCounts;
    bool m_isInitialized = false;
    int m_gold = 0;
    int m_elixir = 0;
    int m_maxGold = 0;
    int m_maxElixir = 0;
    int town_hall_level = 1;
    int bar_level = 1;
    int giant_level = 1;
    int arc_level = 1;
    int bom_level = 1;
    int dragon_level = 1;
    std::chrono::steady_clock::time_point m_timeAccelerateCooldownEnd;

    std::vector<PendingBuildingUpgrade> m_pendingUpgrades;
    int m_nextUpgradeTaskId = 0;
    bool m_upgradeTickerScheduled = false;

    void incrementStoredBuildingLevel(BuildingType type, cocos2d::Vec2 position);
};

#endif // __GAME_MANAGER_H__