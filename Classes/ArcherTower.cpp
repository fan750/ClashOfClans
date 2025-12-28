#include "ArcherTower.h"
#include "BattleManager.h"
USING_NS_CC;

// 工厂方法：创建箭塔对象
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

// 初始化建筑属性
void ArcherTower::initBuildingProperties()
{
    // 设置外观与基础属性
    std::string filename = "ArcherTower.png";
    this->setTexture(filename);

    int hp = 700;
    this->setProperties(hp, CampType::PLAYER);

    // 设置血条样式与偏移
    m_hpBarWidth = 5.0f;
    m_hpBarHeight = 4.0f;
    this->setHpBarOffsetY(280.0f); // 箭塔比加农炮高

    // 计算缩放比例以匹配目标大小
    float targetSize = 150.0f;
    Size contentSize = this->getContentSize();
    if (contentSize.width > 0)
    {
        this->setScale(targetSize / contentSize.width);
        m_baseScale = this->getScale();
    }

    // 配置攻击属性
    m_attackRange = 350.0f;
    m_damage = 60;
    m_attackInterval = 1.0f;

    // 设置生产速率为0（防御建筑不生产资源）
    m_productionRate = 0;
}

// 更新逻辑：自动攻击范围内最近的目标
void ArcherTower::updateLogic(float dt)
{
    // 累加计时器
    m_timer += dt;

    if (m_timer >= m_attackInterval)
    {
        // 寻找目标（自动包含空军）
        Troop* target = BattleManager::getInstance()->findClosestTroopForBuilding(this);

        if (target)
        {
            m_timer = 0;
            // 对目标造成伤害
            target->takeDamage(m_damage);

            // 播放视觉反馈动画（闪烁绿色）
            auto seq = Sequence::create
            (
                TintTo::create(0.1f, Color3B::GREEN), // 变绿
                TintTo::create(0.1f, this->getColor()), // 恢复原色
                nullptr
            );
            this->runAction(seq);
        }
    }
}