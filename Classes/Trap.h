#ifndef __TRAP_H__
#define __TRAP_H__

#include "Building.h"

class Trap : public Building
{
protected:
    virtual void initBuildingProperties() override; // 初始化建筑属性
    virtual void updateLogic(float dt) override;    // 更新逻辑
    virtual void onDeath() override;                // 重写死亡逻辑（陷阱有特殊表现）

    const float m_triggerRange = 50.0f;             // 触发范围
    const int m_trapDps = 30;                       // 触发伤害

public:
    static Trap* create();                          // 工厂方法：创建陷阱对象
};

#endif // __TRAP_H__