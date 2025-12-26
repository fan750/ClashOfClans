// Trap.h
#ifndef __TRAP_H__
#define __TRAP_H__

#include "Building.h"

class Trap : public Building
{
public:
    static Trap* create();

protected:
    virtual void initBuildingProperties() override;
    virtual void updateLogic(float dt) override;
    virtual void onDeath() override; // 重写死亡逻辑（陷阱可能有特殊表现）

    const float m_triggerRange = 50.0f; // 触发范围
    const int m_trapDps = 30;          // 触发伤害
};

#endif // __TRAP_H__