#ifndef __GOLD_MINE_H__
#define __GOLD_MINE_H__

#include "Building.h"

class GoldMine : public Building
{
protected:
    virtual void initBuildingProperties() override; // 初始化建筑属性（外观、HP、生产速率等）
    virtual void updateLogic(float dt) override;    // 更新逻辑（金币生产累加）
    virtual int collectResource() override;         // 收集资源（金币）

public:
    static GoldMine* create();                       // 工厂方法：创建金矿对象
};

#endif // __GOLD_MINE_H__