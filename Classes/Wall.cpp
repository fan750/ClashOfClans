// Wall.cpp
#include "Wall.h"

USING_NS_CC;

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

void Wall::initBuildingProperties()
{
    // 1. 外观
    std::string filename = "Wall.png";
    this->setTexture(filename);

    // 2. 属性（高血量）
    int hp = 1000;
    this->setProperties(hp, CampType::PLAYER);

    // 3. 血条设置：墙很矮，血条位置要调整，可能还需要更细更短
    m_hpBarWidth = 2.0f;
    m_hpBarHeight = 2.0f;
    this->setHpBarOffsetY(-300.0f); // 向下偏移
    this->setHpBarOffsetX(-220.0f);

    // 4. 缩放
    float targetSize = 150.0f;
    Size contentSize = this->getContentSize();
    if (contentSize.width > 0)
    {
        this->setScale(targetSize / contentSize.width);
        m_baseScale = this->getScale();
    }

    // 5. 无生产，无攻击
    m_productionRate = 0;
    m_attackRange = 0;
}