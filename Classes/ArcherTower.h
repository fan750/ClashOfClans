#ifndef __ARCHER_TOWER_H__
#define __ARCHER_TOWER_H__

#include "Building.h"

class ArcherTower : public Building
{
protected:
    virtual void initBuildingProperties() override; // 初始化建筑属性（外观、HP、攻击力等）
    virtual void updateLogic(float dt) override;    // 更新逻辑（索敌与攻击）

public:
    static ArcherTower* create();                   // 工厂方法：创建箭塔对象
};

#endif // __ARCHER_TOWER_H__