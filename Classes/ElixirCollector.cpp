#include "ElixirCollector.h"
#include "GameManager.h"

USING_NS_CC;

// 工厂方法：创建圣水收集器对象
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

// 初始化建筑属性
void ElixirCollector::initBuildingProperties()
{
    // 设置外观
    std::string filename = "elixir_anim_2.png";
    this->setTexture(filename);

    // 设置属性
    int hp = 600;
    this->setProperties(hp, CampType::PLAYER);

    // 设置血条样式与偏移
    m_hpBarWidth = 5.0f;
    m_hpBarHeight = 4.0f;
    this->setHpBarOffsetX(300.0f);
    this->setHpBarOffsetY(1080.0f);

    // 计算缩放比例
    float targetSize = 150.0f;
    Size contentSize = this->getContentSize();
    if (contentSize.width > 0)
    {
        this->setScale(targetSize / contentSize.width);
        m_baseScale = this->getScale();
    }

    // 配置生产属性
    m_productionRate = 10.0f;
    m_maxStorage = 100.0f;
    m_currentStored = 0.0f;
    m_productionAccumulator = 0.0f;

    // 播放工作动画
    this->playWorkAnimation();
}

// 更新逻辑：处理圣水生产累加
void ElixirCollector::updateLogic(float dt)
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

// 收集资源：将存储的圣水移至管理器
int ElixirCollector::collectResource()
{
    // 检查是否有资源可收集
    if (m_currentStored <= 0)
    {
        return 0;
    }

    int amountToCollect = (int)m_currentStored;

    // 调用管理器增加圣水
    GameManager::getInstance()->addElixir(amountToCollect);

    // 清空存储
    m_currentStored = 0.0f;
    m_productionAccumulator = 0.0f;

    return amountToCollect;
}