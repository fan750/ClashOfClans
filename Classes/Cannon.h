#ifndef __CANNON_H__
#define __CANNON_H__

#include "Building.h"

class Cannon : public Building
{
protected:
    virtual void initBuildingProperties() override; // 初始化建筑属性（外观、HP、攻击力等）
    virtual void updateLogic(float dt) override;    // 更新逻辑（索敌与攻击）

public:
    static Cannon* create();                        // 工厂方法：创建加农炮对象
};

#endif // __CANNON_H__