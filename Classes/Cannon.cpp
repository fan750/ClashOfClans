// Cannon.cpp
#include "Cannon.h"
#include "BattleManager.h"
#include "GameManager.h"
USING_NS_CC;

Cannon* Cannon::create()
{
    Cannon* pRet = new(std::nothrow) Cannon();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

void Cannon::initBuildingProperties()
{
    // 1. 外观与基础属性
    std::string filename = "Cannon.png";
    this->setTexture(filename);

    int hp = 800;
    this->setProperties(hp, CampType::PLAYER);

    // 2. 血条设置
    m_hpBarWidth = 5.0f;
    m_hpBarHeight = 4.0f;
    this->setHpBarOffsetY(240.0f);

    // 3. 缩放
    float targetSize = 150.0f;
    Size contentSize = this->getContentSize();
    if (contentSize.width > 0)
    {
        this->setScale(targetSize / contentSize.width);
        m_baseScale = this->getScale();
    }

    // 4. 攻击属性配置
    m_attackRange = 150.0f;
    m_damage = 80;          // 伤害值
    m_attackInterval = 1.0f; // 1秒攻击一次

    // 5. 无资源生产
    m_productionRate = 0;
    m_maxStorage = 0;
}

void Cannon::updateLogic(float dt)
{
    m_timer += dt;

    // 判断是否到达攻击间隔
    if (m_timer >= m_attackInterval)
    {
        // 询问 BattleManager 寻找范围内的敌人
        // findClosestTroopForBuilding 会根据 Cannon 的类型自动筛选地面单位
        Troop* target = BattleManager::getInstance()->findClosestTroopForBuilding(this);

        if (target)
        {
            m_timer = 0; // 重置冷却

            // 造成伤害
            target->takeDamage(m_damage);

            // 视觉反馈：闪烁表示开火
            auto seq = Sequence::create
            (
                TintTo::create(0.1f, Color3B::ORANGE),
                TintTo::create(0.1f, this->getColor()),
                nullptr
            );
            this->runAction(seq);
        }
    }
}