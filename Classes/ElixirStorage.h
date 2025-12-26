// ElixirStorage.h
#ifndef __ELIXIR_STORAGE_H__
#define __ELIXIR_STORAGE_H__

#include "Building.h"

class ElixirStorage : public Building
{
public:
    static ElixirStorage* create();

protected:
    virtual void initBuildingProperties() override;
    virtual void onUpgradeFinished() override;
};

#endif // __ELIXIR_STORAGE_H__