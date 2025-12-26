// GoldMine.h
#ifndef __GOLD_MINE_H__
#define __GOLD_MINE_H__

#include "Building.h"

class GoldMine : public Building
{
public:
    static GoldMine* create();

protected:
    virtual void initBuildingProperties() override;
    virtual void updateLogic(float dt) override; // 实现每帧生产逻辑
    virtual int collectResource() override;      // 实现收集逻辑
};

#endif // __GOLD_MINE_H__