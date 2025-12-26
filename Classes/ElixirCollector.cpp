// ElixirCollector.cpp
#include "ElixirCollector.h"
#include "GameManager.h"

USING_NS_CC;

ElixirCollector* ElixirCollector::create()
{
    ElixirCollector* pRet = new(std::nothrow) ElixirCollector();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

void ElixirCollector::initBuildingProperties()
{
    // 1. 外观
    std::string filename = "elixir_anim_2.png";
    this->setTexture(filename);

    // 2. 属性
    int hp = 600;
    this->setProperties(hp, CampType::PLAYER);

    // 3. 血条设置
    m_hpBarWidth = 5.0f;
    m_hpBarHeight = 4.0f;
    this->setHpBarOffsetX(300.0f);
    this->setHpBarOffsetY(1080.0f);

    // 4. 缩放
    float targetSize = 150.0f;
    Size contentSize = this->getContentSize();
    if (contentSize.width > 0)
    {
        this->setScale(targetSize / contentSize.width);
        m_baseScale = this->getScale();
    }

    // 5. 生产属性配置
    m_productionRate = 10.0f;
    m_maxStorage = 100.0f;
    m_currentStored = 0.0f;
    m_productionAccumulator = 0.0f;

    // 6. 播放工作动画
    this->playWorkAnimation();
}

void ElixirCollector::updateLogic(float dt)
{
    // 逻辑与金矿一致
    if (m_currentStored >= m_maxStorage)
    {
        m_currentStored = m_maxStorage;
        return;
    }

    float effectiveRate = m_productionRate;
    m_productionAccumulator += effectiveRate * dt;

    if (m_productionAccumulator >= 1.0f)
    {
        int amountToAdd = (int)m_productionAccumulator;
        m_currentStored += amountToAdd;
        m_productionAccumulator -= amountToAdd;

        if (m_currentStored >= m_maxStorage) m_currentStored = m_maxStorage;
    }
}

int ElixirCollector::collectResource()
{
    if (m_currentStored <= 0) return 0;

    int amountToCollect = (int)m_currentStored;

    // 调用管理器增加圣水
    GameManager::getInstance()->addElixir(amountToCollect);

    // 清空存储
    m_currentStored = 0.0f;
    m_productionAccumulator = 0.0f;

    CCLOG("Collected %d Elixir!", amountToCollect);
    return amountToCollect;
}