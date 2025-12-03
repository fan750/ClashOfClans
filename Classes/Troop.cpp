#include "Troop.h"
#include "BattleManager.h" // 确保引用了 BattleManager

USING_NS_CC;

Troop::Troop()
    : m_type(TroopType::BARBARIAN)
    , m_moveSpeed(0.0f)
    , m_attackRange(0.0f)
    , m_damage(0)
    , m_attackInterval(1.0f)
    , m_attackTimer(0.0f)
    , m_target(nullptr)
{
}

Troop::~Troop() {
}

Troop* Troop::create(TroopType type) {
    Troop* pRet = new(std::nothrow) Troop();
    if (pRet) {
        pRet->m_type = type;
        if (pRet->init()) {
            pRet->autorelease();
            return pRet;
        }
    }
    delete pRet;
    return nullptr;
}

bool Troop::init() {
    if (!GameEntity::init()) return false;
    initTroopProperties();
    BattleManager::getInstance()->addTroop(this);
    this->scheduleUpdate();
    return true;
}

void Troop::onDeath() {
    BattleManager::getInstance()->removeTroop(this);
    GameEntity::onDeath();
}

void Troop::initTroopProperties() {
    // 默认设置
    this->setTextureRect(Rect(0, 0, 20, 20));

    if (m_type == TroopType::BARBARIAN) {
        this->setColor(Color3B::GREEN);
        setProperties(100, CampType::ENEMY);
        m_moveSpeed = 100.0f;
        m_attackRange = 40.0f;
        m_damage = 50;
        m_attackInterval = 1.0f;
    }
    else if (m_type == TroopType::ARCHER) {
        this->setColor(Color3B::MAGENTA);
        this->setTextureRect(Rect(0, 0, 15, 20));
        setProperties(60, CampType::ENEMY);
        m_moveSpeed = 120.0f;
        m_attackRange = 150.0f;
        m_damage = 25;
        m_attackInterval = 0.8f;
    }
    else if (m_type == TroopType::GIANT) {
        this->setColor(Color3B::ORANGE);
        this->setTextureRect(Rect(0, 0, 40, 40));
        setProperties(200, CampType::ENEMY);
        m_moveSpeed = 80.0f;
        m_attackRange = 40.0f;
        m_damage = 40;
        m_attackInterval = 1.5f;
    }
    else if (m_type == TroopType::BOMBERMAN) {
        this->setColor(Color3B::WHITE);
        this->setTextureRect(Rect(0, 0, 15, 15));
        setProperties(40, CampType::ENEMY);
        m_moveSpeed = 150.0f;
        m_attackRange = 20.0f;
        m_damage = 500;
        m_attackInterval = 0.1f;
    }
}

void Troop::setTarget(Building* target) {
    m_target = target;
}

void Troop::updateLogic(float dt) {
    // 索敌逻辑
    if (!m_target || m_target->isDead()) {
        m_target = nullptr;

        if (m_type == TroopType::BOMBERMAN) {
            // 炸弹人只找墙
            m_target = BattleManager::getInstance()->findClosestBuildingOfType(this->getPosition(), BuildingType::WALL);
        }
        else {
            // 其他人找最近的
            m_target = BattleManager::getInstance()->findClosestBuilding(this->getPosition());
        }

        if (!m_target) return;
    }

    Vec2 myPos = this->getPosition();
    Vec2 targetPos = m_target->getPosition();
    float distance = myPos.distance(targetPos);

    if (distance <= m_attackRange) {
        attackTarget(dt);
    }
    else {
        moveTowardsTarget(dt);
    }
}

void Troop::moveTowardsTarget(float dt) {
    if (!m_target) return;

    // 【回滚到最简单的逻辑】直接走直线，不搞那些复杂的防重叠了
    Vec2 myPos = this->getPosition();
    Vec2 targetPos = m_target->getPosition();

    Vec2 direction = targetPos - myPos;
    direction.normalize();

    Vec2 newPos = myPos + direction * m_moveSpeed * dt;
    this->setPosition(newPos);
}

void Troop::attackTarget(float dt) {
    m_attackTimer += dt;

    if (m_attackTimer >= m_attackInterval) {
        m_attackTimer = 0;

        if (m_type == TroopType::BOMBERMAN) {
            CCLOG("Bomberman Exploded!");
            Vec2 explosionCenter = this->getPosition();
            BattleManager::getInstance()->dealAreaDamage(explosionCenter, 100.0f, m_damage);

            auto seq = Sequence::create(
                Spawn::create(ScaleTo::create(0.1f, 3.0f), TintTo::create(0.1f, Color3B::RED), nullptr),
                RemoveSelf::create(),
                nullptr
            );
            this->runAction(seq);
            return;
        }

        // 普通攻击
        Vec2 targetPos = m_target->getPosition(); // 1. 先存位置
        m_target->takeDamage(m_damage);           // 2. 再伤害

        // 动画
        if (m_type == TroopType::ARCHER) {
            auto arrow = Sprite::create();
            arrow->setTextureRect(Rect(0, 0, 10, 2));
            arrow->setColor(Color3B::YELLOW);
            arrow->setPosition(this->getPosition());
            this->getParent()->addChild(arrow);

            float distance = this->getPosition().distance(targetPos);
            float duration = distance / 400.0f;
            arrow->runAction(Sequence::create(MoveTo::create(duration, targetPos), RemoveSelf::create(), nullptr));
        }
        else {
            this->runAction(Sequence::create(ScaleTo::create(0.1f, 1.2f), ScaleTo::create(0.1f, 1.0f), nullptr));
        }
    }
}