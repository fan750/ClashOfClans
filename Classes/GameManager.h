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
    BuildingType type;                         // 建筑类型
    cocos2d::Vec2 position;                    // 建筑位置
    int level;                                 // 建筑等级
    int barrackLevel;                          // 军营等级（仅对军营有效）
};

// 待升级建筑任务结构体
struct PendingBuildingUpgrade
{
    int id;                                        // 任务ID
    BuildingType type;                             // 建筑类型
    cocos2d::Vec2 position;                        // 建筑位置
    std::chrono::steady_clock::time_point endTime; // 升级结束时间点
};

class GameManager
{
private:
    // 内部辅助函数
    void ensureUpgradeTicker();                                                   // 确保升级计时器运行
    void updatePendingUpgrades(float dt);                                         // 更新待升级任务状态
    void incrementStoredBuildingLevel(BuildingType type, cocos2d::Vec2 position); // 增加存储中的建筑等级

    // 构造函数与单例
    GameManager();                              // 私有构造函数
    static GameManager* s_instance;             // 单例实例指针

    // 存档与数据管理成员变量
    std::vector<BuildingData> m_homeBuildings;  // 存储的主村庄建筑数据
    std::map<TroopType, int> m_troopCounts;     // 兵种数量统计
    bool m_isInitialized;                       // 账户是否已初始化标记

    // 资源成员变量
    int m_gold;                                 // 当前金币
    int m_elixir;                               // 当前圣水
    int m_maxGold;                              // 金币上限
    int m_maxElixir;                            // 圣水上限

    // 等级成员变量
    int town_hall_level;                        // 大本营等级
    int bar_level;                              // 野蛮人等级
    int giant_level;                            // 巨人等级
    int arc_level;                              // 弓箭手等级
    int bom_level;                              // 炸弹人等级
    int dragon_level;                           // 飞龙等级

    // 加速与升级任务成员变量
    std::chrono::steady_clock::time_point m_timeAccelerateCooldownEnd; // 时间加速冷却结束时间
    std::vector<PendingBuildingUpgrade> m_pendingUpgrades;             // 待升级建筑任务列表
    int m_nextUpgradeTaskId;                                           // 下一个升级任务ID
    bool m_upgradeTickerScheduled;                                     // 升级计时器是否已调度

public:
    // 单例获取
    static GameManager* getInstance();

    // 账户初始化
    void initAccount(int gold, int elixir);                // 初始化账户 (带防重置检查)
    bool isInitialized() const { return m_isInitialized; } // 判断是否已初始化

    // 建筑存档管理
    void addHomeBuilding(BuildingType type, cocos2d::Vec2 pos, int level = 1);     // 保存一个建筑
    const std::vector<BuildingData>& getHomeBuildings();                           // 获取所有保存的建筑
    void updateHomeBuildingLevel(BuildingType type, cocos2d::Vec2 pos, int level); // 更新已保存建筑的等级（用于升级时同步）

    // 军营管理
    int getBarrackLevel() const;                           // 获取当前军营等级
    void updateBarrackLevel(cocos2d::Vec2 pos, int level); // 更新军营等级

    // 资源管理
    void addGold(int amount);                    // 增加金币 (带上限检查)
    void addElixir(int amount);                  // 增加圣水 (带上限检查)
    void modifyMaxGold(int amount);              // 修改金币上限 (当建造/升级圣水瓶时调用)
    void modifyMaxElixir(int amount);            // 修改圣水上限

    int getGold() const { return m_gold; }           // 获取当前金币
    int getElixir() const { return m_elixir; }       // 获取当前圣水
    int getMaxGold() const { return m_maxGold; }     // 获取金币上限 (用于UI显示，比如 1000/2000)
    int getMaxElixir() const { return m_maxElixir; } // 获取圣水上限

    // 兵种库存管理
    void addTroops(TroopType type, int amount);                // 增加兵种
    int getTroopCount(TroopType type) const;                   // 获取指定兵种的数量
    void consumeTroops(TroopType type, int amount);            // 消耗兵种
    void setTroopCount(TroopType type, int count);             // 设置兵种数量
    const std::map<TroopType, int>& getAllTroopCounts() const; // 获取所有兵种数量
    void clear();                                              // 清空数据

    // 兵种等级管理
    int getTown_Hall_Level() { return town_hall_level; }            // 获取大本营等级
    void setTown_Hall_Level(int level) { town_hall_level = level; } // 设置大本营等级

    int getBarLevel() { return bar_level; }            // 获取野蛮人等级
    void setBarLevel(int level) { bar_level = level; } // 设置野蛮人等级

    int getGiantLevel() { return giant_level; }            // 获取巨人等级
    void setGiantLevel(int level) { giant_level = level; } // 设置巨人等级

    int getArcLevel() { return arc_level; }            // 获取弓箭手等级
    void setArcLevel(int level) { arc_level = level; } // 设置弓箭手等级

    int getBomLevel() { return bom_level; }            // 获取炸弹人等级
    void setBomLevel(int level) { bom_level = level; } // 设置炸弹人等级

    int getDragonLevel() { return dragon_level; }            // 获取飞龙等级
    void setDragonLevel(int level) { dragon_level = level; } // 设置飞龙等级

    // 时间加速管理
    void setTimeAccelerateCooldownEnd(const std::chrono::steady_clock::time_point& end); // 设置冷却结束时间
    std::chrono::steady_clock::time_point getTimeAccelerateCooldownEnd() const;          // 获取冷却结束时间
    float getTimeAccelerateCooldownRemaining() const;                                    // 获取冷却剩余时间

    // 建筑升级调度
    void scheduleBuildingUpgrade(Building* building, float duration);                                // 调度建筑升级任务
    void completeBuildingUpgrade(int taskId);                                                        // 完成建筑升级任务
    bool hasPendingUpgrade(BuildingType type, cocos2d::Vec2 position, float tolerance = 5.0f) const; // 检查是否有待升级任务

    // 建筑升级加速
    void applyBuildingUpgradeBoost(BuildingType type, cocos2d::Vec2 position, float multiplier, float duration); // 应用建筑升级加速
};

#endif // __GAME_MANAGER_H__