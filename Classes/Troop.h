#ifndef __TROOP_H__
#define __TROOP_H__

#include "GameEntity.h"
#include "Building.h"

enum class TroopType {
    BARBARIAN,
    ARCHER,
    GIANT,
    BOMBERMAN
};

class Troop : public GameEntity {
public:
    Troop();
    virtual ~Troop();
    static Troop* create(TroopType type);

    virtual bool init() override;
    virtual void updateLogic(float dt) override;
    virtual void onDeath() override;
    TroopType getTroopType() const { return m_type; } // 获取兵种类型

    void setTarget(Building* target);

protected:
    void initTroopProperties();
    void moveTowardsTarget(float dt);
    void attackTarget(float dt);

protected:
    TroopType m_type;
    float m_moveSpeed;
    float m_attackRange;
    int m_damage;
    float m_attackInterval;
    float m_attackTimer;
    Building* m_target;

    // 保存基础缩放，用于在播放缩放动画时保持相对比例
    float m_baseScale;

    // 标记当前是否在播放攻击动画，防止重复触发
    bool m_isAttacking;

    // store resource filenames for this troop instance
    std::string m_walkPlist;
    std::string m_attackPlist;
};

#endif // __TROOP_H__