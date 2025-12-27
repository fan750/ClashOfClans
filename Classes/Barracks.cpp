// Barracks.cpp
#include "Barracks.h"
#include "GameManager.h"
#include "BattleManager.h"
#include "Troop.h"
USING_NS_CC;

const std::map<int, Barracks::BarrackUpgradeConfig>& Barracks::getBarrackUpgradeConfigs() {
    static std::map<int, BarrackUpgradeConfig> configs = {
        {0, {0, 0}},                              // 未建造/初始化
        {1, {0, 10}},                             // 一级：免费，Cost上限 10
        {2, {500, 25}},                           // 二级：500金币，Cost上限 25
        {3, {1000, 50}}                           // 三级：1000金币，Cost上限 50
    };
    return configs;
}

Barracks::Barracks()
{
    // 【核心修复】确保 Troop 静态数据已初始化
    Troop::initStaticData();
    // 初始化成员
    m_barrackLevel = 1;  // 默认新建为1级
    m_maxCostLimit = 0;
    m_currentCostUsed = 0;
}

Barracks* Barracks::create()
{
    Barracks* pRet = new(std::nothrow) Barracks();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

// 重写 activateBuilding，在建筑完全放置到场景后再更新 Cost
void Barracks::activateBuilding()
{
    // 调用父类的激活方法
    Building::activateBuilding();

    // 在建筑完全激活后，再计算 Cost 使用情况并派发事件
    // updateCurrentCostUsed();
}

void Barracks::initBuildingProperties()
{
    // 1. 外观
    std::string filename = "Barracks.png";
    this->setTexture(filename);

    // 2. 属性
    int hp = 800;
    this->setProperties(hp, CampType::PLAYER);

    // 3. 血条设置
    m_hpBarWidth = 5.0f;
    m_hpBarHeight = 4.0f;
    this->setHpBarOffsetX(40.0f);
    this->setHpBarOffsetY(280.0f);

    // 4. 缩放
    float targetSize = 150.0f;
    Size contentSize = this->getContentSize();
    if (contentSize.width > 0)
    {
        this->setScale(targetSize / contentSize.width);
        m_baseScale = this->getScale();
    }

    // 5. 初始化军营数据
    // 默认创建为 1 级
    m_barrackLevel = 1;
    const auto& configs = getBarrackUpgradeConfigs(); // 使用函数获取配置
    if (configs.find(1) != configs.end()) {
        m_maxCostLimit = configs.at(1).maxCostLimit;
    }
    m_currentCostUsed = 0;
}

void Barracks::setBarrackLevel(int level)
{
    if (level < 0 || level > 3) return;
    m_barrackLevel = level;

    // 根据等级设置 Cost 上限
    const auto& configs = getBarrackUpgradeConfigs(); // 使用函数获取配置
    if (configs.find(level) != configs.end()) {
        m_maxCostLimit = configs.at(level).maxCostLimit;
    }

    // 重新计算 Cost
    updateCurrentCostUsed();
}

void Barracks::updateCurrentCostUsed()
{
    m_currentCostUsed = 0;
    auto gm = GameManager::getInstance();

    // 遍历所有兵种类型，计算 Cost
    // 注意：这里依赖 Troop::getStaticTroopCost 静态方法
    std::vector<TroopType> troopTypes = {
        TroopType::BARBARIAN,
        TroopType::ARCHER,
        TroopType::GIANT,
        TroopType::BOMBERMAN,
        TroopType::DRAGON
    };

    for (auto type : troopTypes) {
        int troopCost = Troop::getStaticTroopCost(type);
        int troopCount = gm->getTroopCount(type);
        m_currentCostUsed += troopCost * troopCount;
    }

    CCLOG("Barracks: Cost Used %d / %d", m_currentCostUsed, m_maxCostLimit);

    // 发送事件通知 UI 更新
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_COST_UPDATED");
}

bool Barracks::canUpgradeBarrack() const
{
    // 达到最高级 3
    if (m_barrackLevel >= 3) return false;

    // 检查下一级的配置
    int nextLevel = m_barrackLevel + 1;
    const auto& configs = getBarrackUpgradeConfigs(); // 使用函数获取配置
    if (configs.find(nextLevel) != configs.end()) {
        int upgradeCost = configs.at(nextLevel).goldCost;
        int currentGold = GameManager::getInstance()->getGold();
        return currentGold >= upgradeCost;
    }
    return false;
}

void Barracks::upgradeBarrack()
{
    if (!canUpgradeBarrack()) return;

    int nextLevel = m_barrackLevel + 1;
    const auto& configs = getBarrackUpgradeConfigs(); // 使用函数获取配置
    auto config = configs.at(nextLevel);

    // 1. 扣除升级费用
    GameManager::getInstance()->addGold(-config.goldCost);

    // 2. 更新内部等级和上限
    m_barrackLevel = nextLevel;
    m_maxCostLimit = config.maxCostLimit;

    CCLOG("Barracks upgraded to level %d, new cost limit: %d", m_barrackLevel, m_maxCostLimit);

    // 3. 重新计算当前 Cost（因为升级可能解锁了新兵种，虽然逻辑上通常是先解锁再招，但以防万一）
    updateCurrentCostUsed();

    // 4. 播放简单的视觉反馈
    // 这里可以调用基类的 onUpgradeFinished 播放 Q弹 效果，
    // 但要注意不要重复执行 Building 的通用逻辑（比如 m_level++），因为那是针对建筑本身的。
    // 如果军营外观需要改变，应该由 Building::onUpgradeFinished 处理（通过点击通用的升级按钮）。
    // 这里的 upgradeBarracks 仅处理功能升级。
    // 我们复用动画：
    auto scaleUp = ScaleTo::create(0.2f, this->getScale() * 1.2f);
    auto scaleDown = ScaleTo::create(0.2f, this->getScale());
    this->runAction(Sequence::create(scaleUp, scaleDown, nullptr));

    // 5. 发送事件通知 UI（按钮状态更新、解锁状态更新）
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_BARRACK_UPGRADED");
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_COST_UPDATED");
}