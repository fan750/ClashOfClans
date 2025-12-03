#ifndef __GAME_ENTITY_H__
#define __GAME_ENTITY_H__

#include "cocos2d.h"

enum class CampType {
    PLAYER,
    ENEMY,
    NEUTRAL
};

class GameEntity : public cocos2d::Sprite {
public:
    GameEntity();
    virtual ~GameEntity();

    virtual bool init() override;

    void setProperties(int maxHp, CampType camp);
    virtual void takeDamage(int damage);
    virtual void onDeath();
    virtual void updateLogic(float dt);
    virtual void update(float dt) override;

    bool isDead() const { return m_currentHp <= 0; }
    CampType getCamp() const { return m_camp; }
    int getCurrentHp() const { return m_currentHp; }
    int getMaxHp() const { return m_maxHp; }

    CREATE_FUNC(GameEntity);

protected:
    int m_maxHp;
    int m_currentHp;
    CampType m_camp;
};

#endif // __GAME_ENTITY_H__