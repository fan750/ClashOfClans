// GoldStorage.h
#ifndef __GOLD_STORAGE_H__
#define __GOLD_STORAGE_H__

#include "Building.h"

class GoldStorage : public Building
{
public:
    static GoldStorage* create();

protected:
    virtual void initBuildingProperties() override;

    // 重写升级回调，增加金币上限
    virtual void onUpgradeFinished() override;
};

#endif // __GOLD_STORAGE_H__