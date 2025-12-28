#ifndef __WALL_H__
#define __WALL_H__

#include "Building.h"

class Wall : public Building
{
protected:
    virtual void initBuildingProperties() override; // 初始化建筑属性（外观、HP等）
    virtual void updateLogic(float dt) override {}  // 重写更新逻辑：围墙不需要每帧逻辑

public:
    static Wall* create(); // 工厂方法：创建围墙对象
};

#endif // __WALL_H__