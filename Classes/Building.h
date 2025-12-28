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
protected:
    // 基础属性成员变量
    BuildingType m_type;                           // 建筑类型
    int m_level;                                   // 建筑等级
    bool m_isActive;                               // 标记建筑是否已激活
    float m_baseScale;                             // 基础缩放比例
    float m_timer;                                 // 通用计时器

    // 资源属性成员变量（由资源类子类使用，防御类置0）
    float m_productionRate;                        // 生产速率
    float m_maxStorage;                            // 内部最大容量
    float m_currentStored;                         // 当前内部积压的资源
    float m_productionAccumulator;                 // 生产累加器

    // 加速属性成员变量
    float m_rateMultiplier;                        // 速率倍率
    float m_rateBoostTimer;                        // 速率加速计时

    // 防御属性成员变量（由防御类子类使用，其他置0）
    float m_attackRange;                           // 攻击范围
    int m_damage;                                  // 攻击力
    float m_attackInterval;                        // 攻击间隔

    // UI 组件成员变量
    cocos2d::ui::Button* m_upgradeBtn;            // 升级按钮
    cocos2d::EventListenerCustom* m_goldListener; // 金币变动监听器
    cocos2d::Sprite* m_constructionSprite;        // 用于播放施工动画的子精灵

    // 升级状态成员变量
    bool m_isUpgrading;                            // 是否正在升级
    float m_upgradeCheckTimer;                     // 用于定期检查升级状态
    float m_constructionSpeedMultiplier;           // 施工加速系数
    float m_constructionBoostTimer;                // 施工加速计时

public:
    // 构造与析构
    Building();                                    // 构造函数
    virtual ~Building();                           // 析构函数

    // 工厂与初始化
    static Building* create(BuildingType type);    // 工厂方法：根据类型创建具体的建筑子类
    virtual bool init() override;                  // 初始化
    virtual void activateBuilding();               // 激活建筑（开始工作）

    // 生命周期
    virtual void updateLogic(float dt) override;   // 更新逻辑
    virtual void onDeath() override;               // 死亡逻辑
    virtual void takeDamage(int damage) override;  // 受伤逻辑（播放音效）

    // 核心虚函数接口
    virtual void initBuildingProperties() = 0;     // 初始化外观、HP、位置等（子类必须实现）
    virtual int collectResource() { return 0; }    // 收集资源（默认返回0，子类可重写）
    virtual void onUpgradeFinished();              // 升级完成回调（处理视觉和通用数值）

    // 属性设置接口
    void setLevel(int level);                      // 设置等级（用于从存档恢复）
    void updateBuildingTexture();                  // 刷新外观

    // 升级逻辑接口
    void startUpgradeProcess();                    // 开始升级流程
    void applyConstructionBoost(float multiplier, float durationSec); // 应用施工加速
    int getUpgradeCost() const;                    // 获取升级费用
    void showConstructionAnimation();              // 显示施工动画

    // 生产逻辑接口
    void playWorkAnimation();                      // 播放工作动画
    void applyProductionBoost(float multiplier, float durationSec); // 应用生产加速

    // UI 交互接口
    void showUpgradeButton();                      // 显示升级按钮
    void showUpgradePanel();                       // 显示升级面板
    void hideUpgradeButton();                      // 隐藏升级按钮
    void updateUpgradeButtonVisibility();          // 根据金币自动显示/隐藏按钮

    // 获取属性接口
    BuildingType getBuildingType() const { return m_type; } // 获取建筑类型
    int getLevel() const { return m_level; }                // 获取建筑等级
    bool isUpgrading() const { return m_isUpgrading; }      // 判断是否正在升级
    float getAttackRange() const { return m_attackRange; }  // 获取攻击范围
};

#endif // __BUILDING_H__