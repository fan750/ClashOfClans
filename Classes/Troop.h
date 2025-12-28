#ifndef __TROOP_H__
#define __TROOP_H__

#include "GameEntity.h"
#include "Building.h"
#include <string>

// 兵种类型枚举
enum class TroopType
{
    BARBARIAN,
    ARCHER,
    GIANT,
    BOMBERMAN,
    DRAGON
};

class Troop : public GameEntity
{
public:
    // 移动类型枚举
    enum MovementType
    {
        GROUND,
        AIR
    };

protected:
    // 兵种属性成员变量
    TroopType m_type;            // 兵种类型
    MovementType m_movementType; // 移动类型
    int m_cost;                  // 兵种Cost值
    int m_minBarrackLevel;       // 最低需要的军营等级
    float m_moveSpeed;           // 移动速度
    float m_attackRange;         // 攻击范围
    int m_damage;                // 攻击力
    float m_attackInterval;      // 攻击间隔
    float m_attackTimer;         // 攻击计时器
    Building* m_target;          // 当前目标建筑
    float m_baseScale;           // 基础缩放大小
    bool m_isAttacking;          // 是否正在攻击

    // 等级与属性相关成员变量
    int m_baseHp;              // 基础血量
    int m_baseDamage;          // 基础伤害

    // 资源文件名
    std::string m_walkPlist;   // 行走动画资源文件名
    std::string m_attackPlist; // 攻击动画资源文件名

    // 内部辅助函数
    int getLevelForType(TroopType type) const;   // 获取特定兵种的当前等级
    float getLevelMultiplier(int level) const;   // 根据等级获取属性加成倍率

public:
    // 静态数据成员
    static std::map<TroopType, std::string> s_staticNames;    // 静态配置表：兵种名称
    static std::map<TroopType, int> s_staticCosts;            // 静态配置表：Cost
    static std::map<TroopType, int> s_staticMinLevels;        // 静态配置表：最低军营等级

    // 静态成员函数
    static void initStaticData();                             // 初始化静态数据表
    static std::string getStaticTroopName(TroopType type);    // 获取静态兵种名称
    static int getStaticTroopCost(TroopType type);            // 获取静态兵种Cost
    static int getStaticTroopMinLevel(TroopType type);        // 获取静态兵种最低军营等级
    static Troop* create(TroopType type);                     // 工厂方法：创建兵种

    // 构造与析构
    Troop();                                                  // 构造函数
    virtual ~Troop();                                         // 析构函数

    // 生命周期函数
    virtual bool init() override;                             // 初始化
    virtual void updateLogic(float dt) override;              // 更新逻辑
    virtual void onDeath() override;                          // 死亡逻辑

    // 获取属性接口
    TroopType getTroopType() const { return m_type; }               // 获取兵种类型
    MovementType getMovementType() const { return m_movementType; } // 获取移动类型

    // 虚函数接口
    virtual int getStaticTroopMinLevel() const { return m_minBarrackLevel; } // 获取最低军营等级
    virtual std::string getTroopName() const = 0;                            // 获取兵种名称 (纯虚函数)
    virtual void initProperties() = 0;                                       // 初始化属性 (纯虚函数)
    virtual void initAnimations() = 0;                                       // 初始化动画 (纯虚函数)

    // 行为逻辑接口
    virtual void acquireTarget();          // 索敌逻辑 (虚函数)
    virtual void performAttackBehavior();  // 攻击行为 (虚函数)
    void moveTowardsTarget(float dt);      // 向目标移动
    void attackTarget(float dt);           // 攻击目标

    // 升级逻辑
    void rescaleStatsForLevel(int level);  // 根据等级重新计算属性
};

#endif // __TROOP_H__