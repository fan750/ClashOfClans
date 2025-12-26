// Barbarian.h
#ifndef __BARBARIAN_H__
#define __BARBARIAN_H__
#include "Troop.h"

class Barbarian : public Troop
{
public:
    Barbarian();
    ~Barbarian() {}

    void initProperties() override;
    void initAnimations() override;
    std::string getTroopName() const override { return "Barbarian"; }

    // 使用基类的默认 acquireTarget 和 performAttackBehavior
};

#endif