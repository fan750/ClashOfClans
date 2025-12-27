// Giant.h
#ifndef __GIANT_H__
#define __GIANT_H__
#include "Troop.h"

class Giant : public Troop
{
public:
    Giant();
    ~Giant() {}

    void initProperties() override;
    void initAnimations() override;
    std::string getTroopName() const override { return "Giant"; }
};

#endif