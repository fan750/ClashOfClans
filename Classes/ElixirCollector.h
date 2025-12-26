// ElixirCollector.h
#ifndef __ELIXIR_COLLECTOR_H__
#define __ELIXIR_COLLECTOR_H__

#include "Building.h"

class ElixirCollector : public Building
{
public:
    static ElixirCollector* create();

protected:
    virtual void initBuildingProperties() override;
    virtual void updateLogic(float dt) override;
    virtual int collectResource() override; // 实现收集逻辑
};

#endif // __ELIXIR_COLLECTOR_H__