// Archer.h
#ifndef __ARCHER_H__
#define __ARCHER_H__
#include "Troop.h"

class Archer : public Troop
{
public:
    Archer();
    ~Archer() {}

    void initProperties() override;
    void initAnimations() override;
    void performAttackBehavior() override; // Ô¶³Ì¹¥»÷
    std::string Archer::getTroopName() const override { return "Archer"; }
};

#endif