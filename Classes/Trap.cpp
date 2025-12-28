#include "Trap.h"
#include "BattleManager.h"
USING_NS_CC;

// 工厂方法：创建陷阱对象
Trap* Trap::create()
{
    Trap* pRet = new(std::nothrow) Trap();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

// 初始化建筑属性
void Trap::initBuildingProperties()
{
    // 设置外观
    std::string filename = "Trap.png";
    this->setTexture(filename);

    // 设置属性
    int hp = 200;
    this->setProperties(hp, CampType::PLAYER);

    // 初始状态：透明（不可见），移除血条
    this->setOpacity(0);
    this->removeHpBar();

    // 设置缩放（陷阱通常比较小）
    Size contentSize = this->getContentSize();
    if (contentSize.width > 0)
    {
        this->setScale(0.1f);
        m_baseScale = this->getScale();
    }
}

// 死亡逻辑 (陷阱死亡无额外表现)
void Trap::onDeath()
{
    return;
}

// 更新逻辑：检测敌军并触发陷阱
void Trap::updateLogic(float dt)
{
    // 累加计时器
    m_timer += dt;

    bool enemyInRange = false;

    // 检测范围内是否有地面单位
    for (auto troop : BattleManager::getInstance()->getTroops())
    {
        if (!troop || troop->isDead())
        {
            continue;
        }

        // 陷阱仅对地面单位有效
        if (troop->getMovementType() == Troop::MovementType::AIR)
        {
            continue;
        }

        float dist = this->getPosition().distance(troop->getPosition());
        if (dist <= m_triggerRange)
        {
            enemyInRange = true;
            break;
        }
    }

    // 根据是否有敌人控制显隐
    this->setOpacity(enemyInRange ? 255 : 0);

    // 触发伤害逻辑（每0.5秒触发一次）
    if (enemyInRange && m_timer >= 0.5f)
    {
        m_timer = 0.0f;

        // 造成范围伤害
        BattleManager::getInstance()->dealAreaDamage(this->getPosition(), m_triggerRange, m_trapDps);

        // 播放视觉特效（闪烁红色）
        auto seq = Sequence::create
        (
            TintTo::create(0.1f, Color3B::RED), // 变红
            TintTo::create(0.1f, this->getColor()), // 恢复原色
            nullptr
        );
        this->runAction(seq);
    }
}