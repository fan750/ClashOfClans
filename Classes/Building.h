//Building.h
#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "GameEntity.h" // 确保路径正确
#include "ui/CocosGUI.h"

// 建筑类型枚举
enum class BuildingType
{
    TOWN_HALL,      // 大本营
    GOLD_MINE,      // 金矿 (生产资源)
    ELIXIR_COLLECTOR, // 圣水收集器
    CANNON,         // 加农炮 (防御)
    ARCHER_TOWER,   // 箭塔
    WALL,            // 围墙
    ELIXIR_STORAGE,   // 圣水瓶 (存储) [新增]
    GOLD_STORAGE,      // 储金罐 (存储) [新增]
    BARRACKS,          // 军营
    TRAP               // 陷阱 (默认隐藏, 靠近敌人时显现, 范围伤害持续)
};

class Building : public GameEntity
{
public:
    Building();
    virtual ~Building();

    // 静态创建函数，参数为建筑类型
    static Building* create(BuildingType type);

    virtual bool init() override;
    virtual void onDeath() override;

    // 重写基类的 updateLogic，用于处理建筑特有逻辑（如生产资源、索敌）
    virtual void updateLogic(float dt) override;

    // 新增：被攻击时播放音效
    virtual void takeDamage(int damage) override;

    void activateBuilding();                                // 激活建筑，使其开始工作
    // 【核心功能】
    // 升级建筑
    void upgrade();

    // 获取建筑类型
    BuildingType getBuildingType() const { return m_type; }
    int getLevel() const { return m_level; }

    // 新增：设置等级（用于从存档恢复）
    void setLevel(int level);

    // 【新增】尝试收集资源
    // 返回值：收集到了多少资源 (用于显示飘字)
    int collectResource();

    // 【新增】获取内部存储状态 (用于UI显示，比如满了显示图标)
    bool isFull() const;

    // 升级相关 UI
    void showUpgradeButton();
    void hideUpgradeButton();
    int getUpgradeCost() const;

    // 根据当前金币自动显示/隐藏升级按钮（会监听 EVENT_UPDATE_GOLD）
    void updateUpgradeButtonVisibility();

    void playWorkAnimation();

protected:
    // 初始化特定类型的属性（血量、图片等）
    void initBuildingProperties();

protected:
    BuildingType m_type;
    int m_level;
    bool m_isActive;               // 标记建筑是否已激活（开始工作）

    // 生产/攻击计时器
    float m_timer;

    // 【新增】资源相关属性
    float m_productionRate;      // 生产速率 (每秒产出多少)
    float m_maxStorage;          // 内部最大容量 (存满即停)
    float m_currentStored;       // 当前内部积压的资源 (还没被收集的)

    // 生产计时器累加器
    float m_productionAccumulator;

    // 升级按钮
    cocos2d::ui::Button* m_upgradeBtn;

    // 监听金币变动的 listener（用于自动显示/隐藏）
    cocos2d::EventListenerCustom* m_goldListener;

    // 新增：记录初始化时的基础缩放，保证从等级恢复时大小可复现
    float m_baseScale;
};

#endif // __BUILDING_H__