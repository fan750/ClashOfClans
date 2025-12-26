// Troop.h
#ifndef __TROOP_H__
#define __TROOP_H__

#include "GameEntity.h"
#include "Building.h"
#include <string>

// 兵种类型列举（保留枚举，主要用于工厂模式create函数的索引，数据不再依赖于此）
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
    // 移动类型枚举 (移入类内或保持全局均可，这里保持全局方便外部判断)
    enum MovementType
    {
        GROUND,
        AIR
    };

    static Troop* create(TroopType type);
    virtual ~Troop();
    virtual bool init() override;
    virtual void updateLogic(float dt) override;
    virtual void onDeath() override;

    // 获取类型ID (由构造函数设置)
    TroopType getTroopType() const { return m_type; }
    MovementType getMovementType() const { return m_movementType; }

    // 获取具体数值的虚接口
    // 这样UI层如果拿到了一个具体的对象，可以直接调用这些方法
    virtual int getCost() const { return m_cost; }
    virtual int getStaticTroopMinLevel() const { return m_minBarrackLevel; }
    virtual std::string getTroopName() const = 0;

    // 【核心虚接口】子类必须实现
    virtual void initProperties() = 0;          // 初始化所有属性(数值/资源/Cost等)
    virtual void initAnimations() = 0;         // 初始化动画资源

    // 行为接口
    virtual void acquireTarget();               // 默认索敌
    virtual void performAttackBehavior();       // 默认攻击

    // 通用逻辑
    void moveTowardsTarget(float dt);
    void attackTarget(float dt);

    // 【新增】静态配置表（存储各兵种的基础数据）
    static std::map<TroopType, std::string> s_staticNames;
    static std::map<TroopType, int> s_staticCosts;
    static std::map<TroopType, int> s_staticMinLevels;

    // 初始化静态数据表（在构造函数中调用或通过静态初始化器）
    static void initStaticData();

    // 【保留】静态获取接口（现在改为查表，不再创建对象）
    static std::string getStaticTroopName(TroopType type);
    static int getStaticTroopCost(TroopType type);
    static int getStaticTroopMinLevel(TroopType type);

    void rescaleStatsForLevel(int level);

protected:
    Troop();

    // 属性定义 (由子类在 initProperties 中赋值)
    TroopType m_type;
    MovementType m_movementType;
    int m_cost;              // 兵种Cost值
    int m_minBarrackLevel;   // 最低需要的军营等级
    float m_moveSpeed;
    float m_attackRange;
    int m_damage;
    float m_attackInterval;
    float m_attackTimer;
    Building* m_target;

    float m_baseScale;
    bool m_isAttacking;

    // 资源名
    std::string m_walkPlist;
    std::string m_attackPlist;

    // 等级相关
    int m_baseHp;
    int m_baseDamage;

private:
    int getLevelForType(TroopType type) const;
    float getLevelMultiplier(int level) const;
};

#endif // __TROOP_H__