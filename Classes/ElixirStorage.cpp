// ElixirStorage.cpp
#include "ElixirStorage.h"
#include "GameManager.h"

USING_NS_CC;

ElixirStorage* ElixirStorage::create()
{
    ElixirStorage* pRet = new(std::nothrow) ElixirStorage();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

void ElixirStorage::initBuildingProperties()
{
    // 1. 外观
    std::string filename = "ElixirStorage.png";
    this->setTexture(filename);

    // 2. 属性
    int hp = 1500; // 圣水瓶血量高一点
    this->setProperties(hp, CampType::PLAYER);

    // 3. 血条设置
    m_hpBarWidth = 5.0f;
    m_hpBarHeight = 4.0f;
    this->setHpBarOffsetX(300.0f);
    this->setHpBarOffsetY(780.0f);

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
}

void ElixirStorage::onUpgradeFinished()
{
    // 1. 基类通用逻辑
    Building::onUpgradeFinished();

    // 2. 增加圣水容量
    int current_max = GameManager::getInstance()->getMaxElixir();
    GameManager::getInstance()->modifyMaxElixir(current_max + 500);

    CCLOG("Elixir Storage Upgraded. Max Elixir: %d", GameManager::getInstance()->getMaxElixir());
}