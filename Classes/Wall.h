// Wall.h
#ifndef __WALL_H__
#define __WALL_H__

#include "Building.h"

class Wall : public Building
{
public:
    static Wall* create();

protected:
    virtual void initBuildingProperties() override;
    // 重写 updateLogic，围墙不需要每帧逻辑（基类默认实现为空，不重写也可）
    virtual void updateLogic(float dt) override {}
};

#endif // __WALL_H__