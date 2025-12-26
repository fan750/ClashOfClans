// GoldStorage.cpp
#include "GoldStorage.h"
#include "GameManager.h"

USING_NS_CC;

GoldStorage* GoldStorage::create()
{
    GoldStorage* pRet = new(std::nothrow) GoldStorage();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

void GoldStorage::initBuildingProperties()
{
    // 1. 外观
    std::string filename = "GoldStorage.png";
    this->setTexture(filename);

    // 2. 属性
    int hp = 600;
    this->setProperties(hp, CampType::PLAYER);

    // 3. 血条设置
    m_hpBarWidth = 5.0f;
    m_hpBarHeight = 4.0f;
    this->setHpBarOffsetX(300.0f);
    this->setHpBarOffsetY(880.0f);

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

    // 注意：储金罐初始化时应该增加初始容量吗？
    // 原代码在 Building.cpp 的 initBuildingProperties 中没有增加容量，
    // 而是在 GameManager 的 initAccount 里给了 1000。
    // 如果你想让它建造时自带容量，可以在这里调用 modifyMaxGold。
}

void GoldStorage::onUpgradeFinished()
{
    // 1. 执行基类的通用升级逻辑（回血、变大、播放动画）
    Building::onUpgradeFinished();

    // 2. 执行特有逻辑：增加金币容量
    int current_max = GameManager::getInstance()->getMaxGold();
    GameManager::getInstance()->modifyMaxGold(current_max + 500);

    CCLOG("Gold Storage Upgraded. Max Gold: %d", GameManager::getInstance()->getMaxGold());
}