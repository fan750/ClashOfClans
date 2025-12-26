// ArcherTower.h
#ifndef __ARCHER_TOWER_H__
#define __ARCHER_TOWER_H__

#include "Building.h"

class ArcherTower : public Building
{
public:
    static ArcherTower* create();

protected:
    virtual void initBuildingProperties() override;
    virtual void updateLogic(float dt) override;
};

#endif // __ARCHER_TOWER_H__