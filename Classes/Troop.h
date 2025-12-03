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
};

#endif // __TROOP_H__