#include "GoldMine.h"
#include "GameManager.h"

USING_NS_CC;

// 工厂方法：创建金矿对象
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

// 初始化建筑属性
void GoldMine::initBuildingProperties()
{
    // 设置外观
    std::string filename = "gold_anim_0.png";
    this->setTexture(filename);

    // 设置属性
    int hp = 600;
    this->setProperties(hp, CampType::PLAYER);

    // 设置血条样式与偏移
    m_hpBarWidth = 5.0f;
    m_hpBarHeight = 4.0f;
    this->setHpBarOffsetX(200.0f);
    this->setHpBarOffsetY(880.0f);

    // 计算缩放比例
    float targetSize = 150.0f;
    Size contentSize = this->getContentSize();
    if (contentSize.width > 0)
    {
        this->setScale(targetSize / contentSize.width);
        m_baseScale = this->getScale();
    }

    // 配置生产属性
    m_productionRate = 10.0f; // 每秒产10
    m_maxStorage = 100.0f;    // 容量100
    m_currentStored = 0.0f;
    m_productionAccumulator = 0.0f;

    // 播放工作动画
    this->playWorkAnimation();
}

// 更新逻辑：处理金币生产累加
void GoldMine::updateLogic(float dt)
{
    // 调用基类 updateLogic 处理加速计时器
    Building::updateLogic(dt);

    // 检查是否已满额
    if (m_currentStored >= m_maxStorage)
    {
        m_currentStored = m_maxStorage;
        return;
    }

    // 应用加速倍率并累加生产量
    float effectiveRate = m_productionRate * m_rateMultiplier;
    m_productionAccumulator += effectiveRate * dt;

    if (m_productionAccumulator >= 1.0f)
    {
        int amountToAdd = (int)m_productionAccumulator;
        m_currentStored += amountToAdd;
        m_productionAccumulator -= amountToAdd;

        // 限制不超过最大容量
        if (m_currentStored >= m_maxStorage)
        {
            m_currentStored = m_maxStorage;
        }
    }
}

// 收集资源：将存储的金币移至管理器
int GoldMine::collectResource()
{
    // 检查是否有资源可收集
    if (m_currentStored <= 0)
    {
        return 0;
    }

    int amountToCollect = (int)m_currentStored;

    // 调用管理器增加金币
    GameManager::getInstance()->addGold(amountToCollect);

    // 清空存储
    m_currentStored = 0.0f;
    m_productionAccumulator = 0.0f;

    return amountToCollect;
}