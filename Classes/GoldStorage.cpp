#include "GoldStorage.h"
#include "GameManager.h"

USING_NS_CC;

// 工厂方法：创建储金罐对象
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

// 初始化建筑属性
void GoldStorage::initBuildingProperties()
{
    // 设置外观
    std::string filename = "GoldStorage.png";
    this->setTexture(filename);

    // 设置属性
    int hp = 600;
    this->setProperties(hp, CampType::PLAYER);

    // 设置血条样式与偏移
    m_hpBarWidth = 5.0f;
    m_hpBarHeight = 4.0f;
    this->setHpBarOffsetX(300.0f);
    this->setHpBarOffsetY(880.0f);

    // 计算缩放比例
    float targetSize = 150.0f;
    Size contentSize = this->getContentSize();
    if (contentSize.width > 0)
    {
        this->setScale(targetSize / contentSize.width);
        m_baseScale = this->getScale();
    }

    // 设置生产速率为0（存储建筑不生产）
    m_productionRate = 0;

}

// 升级完成回调：增加全局金币容量
void GoldStorage::onUpgradeFinished()
{
    // 执行基类通用升级逻辑
    Building::onUpgradeFinished();

    // 增加金币容量
    int current_max = GameManager::getInstance()->getMaxGold();
    GameManager::getInstance()->modifyMaxGold(current_max + 500);
}