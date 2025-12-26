// Barracks.h
#ifndef __BARRACKS_H__
#define __BARRACKS_H__

#include "Building.h"
#include "Troop.h"
#include <map>

class Barracks : public Building
{
public:
    // 军营升级配置结构
    struct BarrackUpgradeConfig {
        int goldCost;            // 升级费用
        int maxCostLimit;        // 升级后的 Cost 上限
    };

    Barracks();
    virtual ~Barracks() = default;

    static Barracks* create();

    void activateBuilding() override;

    // 获取军营功能数据
    int getBarrackLevel() const { return m_barrackLevel; }
    int getCurrentCostUsed() const { return m_currentCostUsed; }
    int getMaxCostLimit() const { return m_maxCostLimit; }

    // 设置军营等级（用于读取存档）
    void setBarrackLevel(int level);

    // 更新当前使用的 Cost（当招募/消耗兵种时调用）
    void updateCurrentCostUsed();

    // 检查是否可以升级军营
    bool canUpgradeBarrack() const;

    // 执行军营升级（消耗金币，提升 Cost 上限）
    void upgradeBarrack();

protected:
    virtual void initBuildingProperties() override;

private:
    int m_barrackLevel;          // 军营等级（0-3）
    int m_currentCostUsed;       // 当前使用的 Cost
    int m_maxCostLimit;          // 当前 Cost 上限

    static const std::map<int, BarrackUpgradeConfig>& getBarrackUpgradeConfigs();
};

#endif // __BARRACKS_H__