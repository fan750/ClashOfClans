// ArcherTower.cpp
#include "ArcherTower.h"
#include "BattleManager.h"
USING_NS_CC;

ArcherTower* ArcherTower::create()
{
    ArcherTower* pRet = new(std::nothrow) ArcherTower();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

void ArcherTower::initBuildingProperties()
{
    // 1. 外观与基础属性
    std::string filename = "ArcherTower.png";
    this->setTexture(filename);

    int hp = 700;
    this->setProperties(hp, CampType::PLAYER);

    // 2. 血条设置
    m_hpBarWidth = 5.0f;
    m_hpBarHeight = 4.0f;
    this->setHpBarOffsetY(280.0f); // 箭塔比加农炮高

    // 3. 缩放
    float targetSize = 150.0f;
    Size contentSize = this->getContentSize();
    if (contentSize.width > 0)
    {
        this->setScale(targetSize / contentSize.width);
        m_baseScale = this->getScale();
    }

    // 4. 攻击属性配置
    // 会根据 BuildingType::ARCHER_TOWER 自动判断它能攻击空军
    m_attackRange = 150.0f;
    m_damage = 80;
    m_attackInterval = 1.0f;

    // 5. 无资源生产
    m_productionRate = 0;
}

void ArcherTower::updateLogic(float dt)
{
    m_timer += dt;

    if (m_timer >= m_attackInterval)
    {
        // 寻找目标（自动包含空军）
        Troop* target = BattleManager::getInstance()->findClosestTroopForBuilding(this);

        if (target)
        {
            m_timer = 0;
            target->takeDamage(m_damage);

            // 视觉反馈
            auto seq = Sequence::create
            (
                TintTo::create(0.1f, Color3B::GREEN), // 箭塔闪烁绿色
                TintTo::create(0.1f, this->getColor()),
                nullptr
            );
            this->runAction(seq);
        }
    }
}