#include "Wall.h"

USING_NS_CC;

// 工厂方法：创建围墙对象
Wall* Wall::create()
{
    Wall* pRet = new(std::nothrow) Wall();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

// 初始化建筑属性
void Wall::initBuildingProperties()
{
    // 设置外观
    std::string filename = "Wall.png";
    this->setTexture(filename);

    // 设置属性（高血量）
    int hp = 1000;
    this->setProperties(hp, CampType::PLAYER);

    // 设置血条样式（墙较矮，需调整位置和尺寸）
    m_hpBarWidth = 2.0f;
    m_hpBarHeight = 2.0f;
    this->setHpBarOffsetY(-300.0f); // 向下偏移
    this->setHpBarOffsetX(-220.0f);

    // 计算缩放比例
    float targetSize = 150.0f;
    Size contentSize = this->getContentSize();
    if (contentSize.width > 0)
    {
        this->setScale(targetSize / contentSize.width);
        m_baseScale = this->getScale();
    }

    // 设置无生产、无攻击
    m_productionRate = 0;
    m_attackRange = 0;
}