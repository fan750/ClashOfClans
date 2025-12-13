//GameManager.h
#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

#include "cocos2d.h"
#include "Building.h" // 需要引用 BuildingType
#include "Troop.h"    // 需要引用 TroopType

// 定义一个结构体，专门存建筑的“档案”
struct BuildingData
{
    BuildingType type;
    cocos2d::Vec2 position;
    int level; // 新增：记录建筑等级
};

class GameManager {
public:
    static GameManager* getInstance();

    // 初始化账户 (带防重置检查)
    void initAccount(int gold, int elixir);
    bool isInitialized() const { return m_isInitialized; }


    // 【新增】保存一个建筑
    void addHomeBuilding(BuildingType type, cocos2d::Vec2 pos, int level = 1);

    // 【新增】获取所有保存的建筑
    const std::vector<BuildingData>& getHomeBuildings();

    // 新增：更新已保存建筑的等级（用于升级时同步）
    void updateHomeBuildingLevel(BuildingType type, cocos2d::Vec2 pos, int level);

    // 【修改】增加资源 (带上限检查)
    void addGold(int amount);
    void addElixir(int amount);

    // 【新增】修改资源上限 (当建造/升级圣水瓶时调用)
    void modifyMaxGold(int amount);
    void modifyMaxElixir(int amount);

    int getGold() const { return m_gold; }
    int getElixir() const { return m_elixir; }

    // 获取上限 (用于UI显示，比如 1000/2000)
    int getMaxGold() const { return m_maxGold; }
    int getMaxElixir() const { return m_maxElixir; }

    void addTroops(TroopType type, int amount);                 // 增加各兵种数量
    int getTroopCount(TroopType type) const;                    // 获取指定兵种的数量
    void consumeTroops(TroopType type, int amount);             // 减少指定兵种数量（用于战斗死亡同步）
    void setTroopCount(TroopType type, int count);              // 设置兵种数量（用于战斗结束后同步）
    const std::map<TroopType, int>& getAllTroopCounts() const;  // 获取所有兵种数量
    void clear();
private:
    GameManager();
    static GameManager* s_instance;

    bool m_isInitialized; // 标记是否已经初始化过
    int m_gold;
    int m_elixir;

    // 【新增】资源上限
    int m_maxGold;
    int m_maxElixir;

    // 【新增】建筑数据列表
    std::vector<BuildingData> m_homeBuildings;
    std::map<TroopType, int> m_troopCounts;    // 存储各兵种数量
};

#endif // __GAME_MANAGER_H__