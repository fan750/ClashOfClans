//Troop.h
#ifndef __TROOP_H__
#define __TROOP_H__

#include <map>
#include "GameEntity.h"
#include "Building.h"
#include <string>

// 兵种移动类型列举
enum class TroopMovementType
{
    GROUND, // 地面单位
    AIR     // 空中单位
};

// 兵种类型列举
enum class TroopType
{
    BARBARIAN,
    ARCHER,
    GIANT,
    BOMBERMAN,
    DRAGON
};

// 【新增】兵种配置结构
struct TroopConfig {
    int cost;               // 兵种cost值
    int minBarrackLevel;    // 最低需要的军营等级
};

class Troop : public GameEntity
{
public:
    Troop();
    virtual ~Troop();
    virtual bool init() override;
    virtual void updateLogic(float dt) override;
    virtual void onDeath() override;

    static Troop* create(TroopType type);
    static std::string getTroopName(TroopType type);
    TroopType getTroopType() const { return m_type; } // 获取兵种类型
    TroopMovementType getMovementType() const { return m_movementType; } // 获取移动类型

    void setTarget(Building* target);
    void initTroopProperties();
    void moveTowardsTarget(float dt);
    void attackTarget(float dt);

    // 新增：动画接口声明
    void playWalkAnimation();
    void playAttackAnimationOnce();

    // 资源文件名
    std::string m_walkPlist;
    std::string m_attackPlist;

    // 基础属性
    TroopType m_type;
    TroopMovementType m_movementType; // 移动类型属性
    float m_moveSpeed;
    float m_attackRange;
    int m_damage;
    float m_attackInterval;
    float m_attackTimer;
    Building* m_target;
    int m_level;
    // 保存基础缩放，用于在播放缩放动画时保持相对比例
    float m_baseScale;

    // 标记当前是否在播放攻击动画，防止重复触发
    bool m_isAttacking;

    // 【新增】静态配置表
    static const std::map<TroopType, TroopConfig> TROOP_CONFIGS;

    // 【新增】获取兵种cost
    static int getTroopCost(TroopType type);
    static int getMinBarrackLevel(TroopType type);

    // 【新增】按等级调整 hp/damage（其它属性保持不变）
    void rescaleStatsForLevel(int level);

private:
    int getLevelForType(TroopType type) const;
    float getLevelMultiplier(int level) const;

    int m_baseHp;
    int m_baseDamage;
};

#endif // __TROOP_H__