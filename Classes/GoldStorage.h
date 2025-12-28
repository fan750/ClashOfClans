#ifndef __GOLD_STORAGE_H__
#define __GOLD_STORAGE_H__

#include "Building.h"

class GoldStorage : public Building
{
protected:
    virtual void initBuildingProperties() override; // 初始化建筑属性（外观、HP等）
    virtual void onUpgradeFinished() override;      // 升级完成回调（增加金币容量）

public:
    static GoldStorage* create();                   // 工厂方法：创建储金罐对象
};

#endif // __GOLD_STORAGE_H__