// GoldMine.cpp
#include "GoldMine.h"
#include "GameManager.h"

USING_NS_CC;

GoldMine* GoldMine::create()
{
    GoldMine* pRet = new(std::nothrow) GoldMine();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

void GoldMine::initBuildingProperties()
{
    // 1. 外观
    std::string filename = "gold_anim_0.png";
    this->setTexture(filename);

    // 2. 属性
    int hp = 600;
    this->setProperties(hp, CampType::PLAYER);

    // 3. 血条设置
    m_hpBarWidth = 5.0f;
    m_hpBarHeight = 4.0f;
    this->setHpBarOffsetX(200.0f);
    this->setHpBarOffsetY(880.0f);

    // 4. 缩放
    float targetSize = 150.0f;
    Size contentSize = this->getContentSize();
    if (contentSize.width > 0)
    {
        this->setScale(targetSize / contentSize.width);
        m_baseScale = this->getScale();
    }

    // 5. 生产属性配置
    m_productionRate = 10.0f; // 每秒产10
    m_maxStorage = 100.0f;   // 容量100
    m_currentStored = 0.0f;
    m_productionAccumulator = 0.0f;

    // 6. 播放工作动画
    this->playWorkAnimation();
}

void GoldMine::updateLogic(float dt)
{
    // 1. 调用基类 updateLogic 处理加速计时器
    Building::updateLogic(dt);

    // 满额即停
    if (m_currentStored >= m_maxStorage)
    {
        m_currentStored = m_maxStorage;
        return;
    }

    // 累加产量
    // 应用加速倍率
    float effectiveRate = m_productionRate * m_rateMultiplier;
    m_productionAccumulator += effectiveRate * dt;

    if (m_productionAccumulator >= 1.0f)
    {
        int amountToAdd = (int)m_productionAccumulator;
        m_currentStored += amountToAdd;
        m_productionAccumulator -= amountToAdd;

        if (m_currentStored >= m_maxStorage) m_currentStored = m_maxStorage;
    }
}

int GoldMine::collectResource()
{
    if (m_currentStored <= 0) return 0;

    int amountToCollect = (int)m_currentStored;

    // 直接调用管理器增加金币
    GameManager::getInstance()->addGold(amountToCollect);

    // 清空存储
    m_currentStored = 0.0f;
    m_productionAccumulator = 0.0f;

    CCLOG("Collected %d Gold!", amountToCollect);
    return amountToCollect;
}