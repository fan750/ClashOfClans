// Trap.cpp
#include "Trap.h"
#include "BattleManager.h"
USING_NS_CC;

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

void Trap::initBuildingProperties()
{
    // 1. 外观
    std::string filename = "Trap.png";
    this->setTexture(filename);

    // 2. 属性
    int hp = 200;
    this->setProperties(hp, CampType::PLAYER);

    // 3. 初始状态：透明（不可见），移除血条
    this->setOpacity(0);
    this->removeHpBar();

    // 4. 缩放
    // 陷阱通常比较小
    Size contentSize = this->getContentSize();
    if (contentSize.width > 0)
    {
        this->setScale(0.1f);
        m_baseScale = this->getScale();
    }
}

void Trap::onDeath()
{
    // 原代码中 Trap 不参与存储上限变更，也不可被摧毁（或死亡时不执行移除逻辑）
    // 如果你想让陷阱被摧毁后消失，可以调用基类 onDeath
    // 如果你想让它一直存在，就不做任何操作，或者只播放特效
    // 这里保留原代码逻辑：直接返回，不调用基类移除
    return;
}

void Trap::updateLogic(float dt)
{
    m_timer += dt;

    bool enemyInRange = false;

    // 1. 检测范围内是否有地面单位
    for (auto troop : BattleManager::getInstance()->getTroops())
    {
        if (!troop || troop->isDead()) continue;

        // 陷阱仅对地面单位有效
        if (troop->getMovementType() == Troop::MovementType::AIR) continue;

        float dist = this->getPosition().distance(troop->getPosition());
        if (dist <= m_triggerRange)
        {
            enemyInRange = true;
            break;
        }
    }

    // 2. 根据是否有敌人控制显隐
    this->setOpacity(enemyInRange ? 255 : 0);

    // 3. 触发伤害逻辑（每0.5秒触发一次）
    if (enemyInRange && m_timer >= 0.5f)
    {
        m_timer = 0.0f;

        // 造成范围伤害
        BattleManager::getInstance()->dealAreaDamage(this->getPosition(), m_triggerRange, m_trapDps);

        // 视觉特效：闪烁红色
        auto seq = Sequence::create
        (
            TintTo::create(0.1f, Color3B::RED),
            TintTo::create(0.1f, this->getColor()),
            nullptr
        );
        this->runAction(seq);
    }
}