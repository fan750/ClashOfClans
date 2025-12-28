#include "Cannon.h"
#include "BattleManager.h"
#include "GameManager.h"
USING_NS_CC;

// 工厂方法：创建加农炮对象
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

// 初始化建筑属性
void Cannon::initBuildingProperties()
{
    // 设置外观与基础属性
    std::string filename = "Cannon.png";
    this->setTexture(filename);

    int hp = 800;
    this->setProperties(hp, CampType::PLAYER);

    // 设置血条样式与偏移
    m_hpBarWidth = 5.0f;
    m_hpBarHeight = 4.0f;
    this->setHpBarOffsetY(240.0f);

    // 计算缩放比例以匹配目标大小
    float targetSize = 150.0f;
    Size contentSize = this->getContentSize();
    if (contentSize.width > 0)
    {
        this->setScale(targetSize / contentSize.width);
        m_baseScale = this->getScale();
    }

    // 配置攻击属性
    m_attackRange = 150.0f;
    m_damage = 100;          // 伤害值
    m_attackInterval = 1.0f; // 1秒攻击一次

    // 设置生产速率为0（防御建筑不生产资源）
    m_productionRate = 0;
    m_maxStorage = 0;
}

// 更新逻辑：自动攻击范围内最近的地面目标
void Cannon::updateLogic(float dt)
{
    // 累加计时器
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

            // 对目标造成伤害
            target->takeDamage(m_damage);

            // 播放视觉反馈动画（闪烁橙色）
            auto seq = Sequence::create
            (
                TintTo::create(0.1f, Color3B::ORANGE), // 变橙
                TintTo::create(0.1f, this->getColor()), // 恢复原色
                nullptr
            );
            this->runAction(seq);
        }
    }
}