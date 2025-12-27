// Dragon.h
#ifndef __DRAGON_H__
#define __DRAGON_H__
#include "Troop.h"

class Dragon : public Troop
{
public:
    Dragon();
    ~Dragon() {}

    void initProperties() override;
    void initAnimations() override;
    std::string getTroopName() const override { return "Dragon"; }
};

#endif