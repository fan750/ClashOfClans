#include "GameEntity.h"

USING_NS_CC;

GameEntity::GameEntity()
    : m_maxHp(100)
    , m_currentHp(100)
    , m_camp(CampType::NEUTRAL)
{
}

GameEntity::~GameEntity() {
}

bool GameEntity::init() {
    // 调用父类 Sprite 的 init
    if (!Sprite::init()) {
        return false;
    }

    // 初始化成功
    return true;
}

void GameEntity::setProperties(int maxHp, CampType camp) {
    this->m_maxHp = maxHp;
    this->m_currentHp = maxHp;
    this->m_camp = camp;
}

void GameEntity::takeDamage(int damage) {
    if (isDead()) return;

    m_currentHp -= damage;
    CCLOG("Entity Took Damage. Remaining: %d", m_currentHp);

    if (m_currentHp <= 0) {
        m_currentHp = 0;
        onDeath();
    }
}

void GameEntity::onDeath() {
    this->removeFromParent();
}

void GameEntity::updateLogic(float dt) {
    // 暂时留空
}
void GameEntity::update(float dt) {
    this->updateLogic(dt);
}