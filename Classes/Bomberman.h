// Bomberman.h
#ifndef __BOMBERMAN_H__
#define __BOMBERMAN_H__
#include "Troop.h"

class Bomberman : public Troop
{
public:
    Bomberman();
    ~Bomberman() {}

    void initProperties() override;
    void initAnimations() override;
    std::string Bomberman::getTroopName() const override { return "Bomberman"; }
    // ¹Ø¼üÖØÐ´
    void acquireTarget() override;
    void performAttackBehavior() override;
};

#endif