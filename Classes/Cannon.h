// Cannon.h
#ifndef __CANNON_H__
#define __CANNON_H__

#include "Building.h"

class Cannon : public Building
{
public:
    static Cannon* create();

protected:
    virtual void initBuildingProperties() override;
    virtual void updateLogic(float dt) override;
};

#endif // __CANNON_H__