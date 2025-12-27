// TownHall.cpp
#include "TownHall.h"
#include "GameManager.h"

USING_NS_CC;

TownHall* TownHall::create()
{
    TownHall* pRet = new(std::nothrow) TownHall();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool TownHall::init()
{
    if (!Building::init()) return false;
    return true;
}

void TownHall::initBuildingProperties()
{
    // 1. 设置外观和基础属性
    std::string filename = "TownHall.png";
    this->setTexture(filename);

    int hp = 2000; // 大本营基础血量
    this->setProperties(hp, CampType::PLAYER); // 设置HP和阵营

    // 2. 设置血条尺寸和偏移（原代码中的逻辑）
    m_hpBarWidth = 5.0f;
    m_hpBarHeight = 4.0f;
    this->setHpBarOffsetX(-40.0f);
    this->setHpBarOffsetY(180.0f);

    // 3. 设置缩放比例（假设标准大小为 150）
    float targetSize = 150.0f;
    Size contentSize = this->getContentSize();
    if (contentSize.width > 0)
    {
        this->setScale(targetSize / contentSize.width);
        m_baseScale = this->getScale();
    }

    // 资源生产相关置0
    m_productionRate = 0;
    m_maxStorage = 0;
    m_currentStored = 0;
}

void TownHall::onUpgradeFinished()
{
    // 1. 先调用基类的通用升级逻辑（播放动画、加血等）
    Building::onUpgradeFinished();

    // 2. 执行大本营特有的逻辑：提升全局大本营等级
    int currentLevel = GameManager::getInstance()->getTown_Hall_Level();
    GameManager::getInstance()->setTown_Hall_Level(currentLevel + 1);

    CCLOG("Town Hall upgraded to Level %d", currentLevel + 1);
}