// TownHall.h
#ifndef __TOWN_HALL_H__
#define __TOWN_HALL_H__

#include "Building.h"

class TownHall : public Building
{
public:
    static TownHall* create(); // 工厂方法
    virtual bool init() override;

protected:
    // 重写初始化属性函数
    virtual void initBuildingProperties() override;

    // 重写升级完成函数，处理大本营特有的升级逻辑（提升全局大本营等级）
    virtual void onUpgradeFinished() override;
};

#endif // __TOWN_HALL_H__