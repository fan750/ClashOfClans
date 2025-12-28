#ifndef __ELIXIR_COLLECTOR_H__
#define __ELIXIR_COLLECTOR_H__

#include "Building.h"

class ElixirCollector : public Building
{
protected:
    virtual void initBuildingProperties() override; // 初始化建筑属性（外观、HP、生产速率等）
    virtual void updateLogic(float dt) override;    // 更新逻辑（圣水生产累加）
    virtual int collectResource() override;         // 收集资源（圣水）

public:
    static ElixirCollector* create();               // 工厂方法：创建圣水收集器对象
};

#endif // __ELIXIR_COLLECTOR_H__