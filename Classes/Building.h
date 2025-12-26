// Building.h
#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "GameEntity.h"
#include "ui/CocosGUI.h"

// 建筑类型枚举
enum class BuildingType
{
    TOWN_HALL,         // 大本营
    GOLD_MINE,         // 金矿
    ELIXIR_COLLECTOR,  // 圣水收集器
    CANNON,            // 加农炮
    ARCHER_TOWER,      // 箭塔
    WALL,              // 围墙
    ELIXIR_STORAGE,    // 圣水瓶
    GOLD_STORAGE,      // 储金罐
    BARRACKS,          // 军营
    TRAP               // 陷阱
};

class Building : public GameEntity
{
public:
    Building();
    virtual ~Building();

    // 工厂方法：根据类型创建具体的建筑子类
    static Building* create(BuildingType type);

    virtual bool init() override;
    virtual void onDeath() override;

    // 重写基类的 updateLogic，具体逻辑由子类实现
    virtual void updateLogic(float dt) override;

    // 重写受击逻辑（播放音效）
    virtual void takeDamage(int damage) override;

    virtual void activateBuilding();                                // 激活建筑（仅军营需重写）
    virtual void onUpgradeFinished();                                // 升级完成回调（处理视觉和通用数值）

    // 【核心虚函数】子类必须实现
    virtual void initBuildingProperties() = 0;          // 初始化外观、HP、位置等

    // 以下为可选重写的虚函数
    virtual int collectResource() { return 0; }           // 收集资源（默认返回0）

    // 获取建筑类型
    BuildingType getBuildingType() const { return m_type; }
    int getLevel() const { return m_level; }

    // 设置等级（用于从存档恢复）
    void setLevel(int level);
    void updateBuildingTexture();                         // 刷新外观

    // 升级相关 UI
    void showUpgradeButton();
    void hideUpgradeButton();
    int getUpgradeCost() const;

    void updateUpgradeButtonVisibility();                 // 根据金币自动显示/隐藏按钮

    void playWorkAnimation();
    void applyProductionBoost(float multiplier, float durationSec);
    void startUpgradeProcess();
protected:
    BuildingType m_type;
    int m_level;
    bool m_isActive;               // 标记建筑是否已激活（开始工作）

    float m_timer;                 // 通用计时器

    // 【资源属性】（由资源类子类使用，防御类置0）
    float m_productionRate;      // 生产速率
    float m_maxStorage;          // 内部最大容量
    float m_currentStored;       // 当前内部积压的资源
    float m_productionAccumulator; // 生产累加器

    // 【加速属性】
    float m_rateMultiplier;      // 加速倍率
    float m_rateBoostTimer;      // 加速时间

    // 【防御属性】（由防御类子类使用，其他置0）
    float m_attackRange;
    int m_damage;
    float m_attackInterval;

    // UI 组件
    cocos2d::ui::Button* m_upgradeBtn;
    cocos2d::EventListenerCustom* m_goldListener;

    // 记录初始化时的基础缩放，保证从等级恢复时大小可复现
    float m_baseScale;

    // 升级相关
    bool m_isUpgrading = false;
    cocos2d::Sprite* m_constructionSprite = nullptr;
};

#endif // __BUILDING_H__