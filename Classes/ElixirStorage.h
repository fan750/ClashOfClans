#ifndef __ELIXIR_STORAGE_H__
#define __ELIXIR_STORAGE_H__

#include "Building.h"

class ElixirStorage : public Building
{
protected:
    virtual void initBuildingProperties() override; // 初始化建筑属性（外观、HP等）
    virtual void onUpgradeFinished() override;      // 升级完成回调（增加圣水容量）

public:
    static ElixirStorage* create();                 // 工厂方法：创建圣水瓶对象
};

#endif // __ELIXIR_STORAGE_H__