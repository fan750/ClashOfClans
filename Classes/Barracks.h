#ifndef __BARRACKS_H__
#define __BARRACKS_H__

#include "Building.h"
#include "Troop.h"
#include <map>

class Barracks : public Building
{
private:
    // 军营升级配置结构
    struct BarrackUpgradeConfig
    {
        int goldCost;            // 升级所需金币费用
        int maxCostLimit;        // 升级后的 Cost 上限
    };

    // 成员变量
    int m_barrackLevel;          // 当前军营等级 (0-3)
    int m_currentCostUsed;       // 当前已使用的 Cost
    int m_maxCostLimit;          // 当前允许的最大 Cost 上限

    // 私有静态辅助函数
    static const std::map<int, BarrackUpgradeConfig>& getBarrackUpgradeConfigs(); // 获取升级配置表

protected:
    virtual void initBuildingProperties() override; // 初始化建筑属性

public:
    Barracks();                                    // 构造函数
    virtual ~Barracks() = default;                 // 析构函数
    static Barracks* create();                     // 工厂方法：创建军营对象
    void activateBuilding() override;              // 激活建筑

    // 获取属性接口
    int getBarrackLevel() const { return m_barrackLevel; }         // 获取军营等级
    int getCurrentCostUsed() const { return m_currentCostUsed; }   // 获取当前 Cost 使用量
    int getMaxCostLimit() const { return m_maxCostLimit; }         // 获取 Cost 上限

    // 设置属性接口
    void setBarrackLevel(int level);    // 设置军营等级（用于存档恢复）
    void updateCurrentCostUsed();       // 更新当前 Cost 使用量

    // 升级逻辑接口
    bool canUpgradeBarrack() const;     // 检查是否可以升级军营
    void upgradeBarrack();              // 执行军营升级逻辑
};

#endif // __BARRACKS_H__