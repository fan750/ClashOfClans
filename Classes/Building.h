#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "GameEntity.h" // 确保路径正确

// 建筑类型枚举
enum class BuildingType {
    TOWN_HALL,      // 大本营
    GOLD_MINE,      // 金矿 (生产资源)
    ELIXIR_COLLECTOR, // 圣水收集器
    CANNON,         // 加农炮 (防御)
    ARCHER_TOWER,   // 箭塔
    WALL            // 围墙
};

class Building : public GameEntity {
public:
    Building();
    virtual ~Building();

    // 静态创建函数，参数为建筑类型
    static Building* create(BuildingType type);

    virtual bool init() override;
    virtual void onDeath() override;

    // 重写基类的 updateLogic，用于处理建筑特有逻辑（如生产资源、索敌）
    virtual void updateLogic(float dt) override;

    // 【核心功能】
    // 升级建筑
    void upgrade();

    // 获取建筑类型
    BuildingType getBuildingType() const { return m_type; }
    int getLevel() const { return m_level; }

protected:
    // 初始化特定类型的属性（血量、图片等）
    void initBuildingProperties();

protected:
    BuildingType m_type;
    int m_level;

    // 生产/攻击计时器
    float m_timer;
};

#endif // __BUILDING_H__