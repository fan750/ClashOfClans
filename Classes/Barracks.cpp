#include "Barracks.h"
#include "GameManager.h"
#include "BattleManager.h"
#include "Troop.h"
USING_NS_CC;

// 获取军营升级配置表
const std::map<int, Barracks::BarrackUpgradeConfig>& Barracks::getBarrackUpgradeConfigs()
{
    static std::map<int, BarrackUpgradeConfig> configs =
    {
        {0, {0, 0}},       // 未建造/初始化
        {1, {0, 10}},      // 一级：免费，Cost上限 10
        {2, {500, 25}},    // 二级：500金币，Cost上限 25
        {3, {1000, 50}}    // 三级：1000金币，Cost上限 50
    };
    return configs;
}

// 构造函数
Barracks::Barracks()
{
    Troop::initStaticData();
    // 初始化成员
    m_barrackLevel = 1;  // 默认新建为1级
    m_maxCostLimit = 0;
    m_currentCostUsed = 0;
}

// 工厂方法：创建军营对象
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

// 激活建筑
void Barracks::activateBuilding()
{
    // 调用父类的激活方法
    Building::activateBuilding();
}

// 初始化建筑属性
void Barracks::initBuildingProperties()
{
    // 设置外观
    std::string filename = "Barracks.png";
    this->setTexture(filename);

    // 设置属性
    int hp = 800;
    this->setProperties(hp, CampType::PLAYER);

    // 设置血条样式与偏移
    m_hpBarWidth = 5.0f;
    m_hpBarHeight = 4.0f;
    this->setHpBarOffsetX(40.0f);
    this->setHpBarOffsetY(280.0f);

    // 计算缩放比例
    float targetSize = 150.0f;
    Size contentSize = this->getContentSize();
    if (contentSize.width > 0)
    {
        this->setScale(targetSize / contentSize.width);
        m_baseScale = this->getScale();
    }

    // 初始化军营数据
    // 默认创建为 1 级
    m_barrackLevel = 1;
    const auto& configs = getBarrackUpgradeConfigs(); // 获取配置
    if (configs.find(1) != configs.end())
    {
        m_maxCostLimit = configs.at(1).maxCostLimit;
    }
    m_currentCostUsed = 0;

    // 尝试从 GameManager 恢复等级（如果已存在）
    // 使用距离判断来匹配建筑，防止浮点数误差
    auto gm = GameManager::getInstance();
    const auto& buildings = gm->getHomeBuildings();
    for (const auto& data : buildings)
    {
        if (data.type == BuildingType::BARRACKS && data.position.distance(this->getPosition()) < 5.0f)
        {
            setBarrackLevel(data.barrackLevel);
            break;
        }
    }
}

// 设置军营等级（用于存档恢复）
void Barracks::setBarrackLevel(int level)
{
    if (level < 0 || level > 3)
    {
        return;
    }
    m_barrackLevel = level;

    // 根据等级设置 Cost 上限
    const auto& configs = getBarrackUpgradeConfigs(); // 获取配置
    if (configs.find(level) != configs.end())
    {
        m_maxCostLimit = configs.at(level).maxCostLimit;
    }

    // 重新计算 Cost
    updateCurrentCostUsed();
}

// 更新当前 Cost 使用量
void Barracks::updateCurrentCostUsed()
{
    m_currentCostUsed = 0;
    auto gm = GameManager::getInstance();

    // 遍历所有兵种类型，计算 Cost
    std::vector<TroopType> troopTypes = 
    {
        TroopType::BARBARIAN,
        TroopType::ARCHER,
        TroopType::GIANT,
        TroopType::BOMBERMAN,
        TroopType::DRAGON
    };

    for (auto type : troopTypes)
    {
        int troopCost = Troop::getStaticTroopCost(type);
        int troopCount = gm->getTroopCount(type);
        m_currentCostUsed += troopCost * troopCount;
    }

    // 发送事件通知 UI 更新
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_COST_UPDATED");
}

// 检查是否可以升级军营
bool Barracks::canUpgradeBarrack() const
{
    // 达到最高级 3
    if (m_barrackLevel >= 3)
    {
        return false;
    }

    // 检查下一级的配置
    int nextLevel = m_barrackLevel + 1;
    const auto& configs = getBarrackUpgradeConfigs(); // 获取配置
    if (configs.find(nextLevel) != configs.end())
    {
        int upgradeCost = configs.at(nextLevel).goldCost;
        int currentGold = GameManager::getInstance()->getGold();
        return currentGold >= upgradeCost;
    }
    return false;
}

// 执行军营升级
void Barracks::upgradeBarrack()
{
    if (!canUpgradeBarrack())
    {
        return;
    }

    int nextLevel = m_barrackLevel + 1;
    const auto& configs = getBarrackUpgradeConfigs(); // 获取配置
    auto config = configs.at(nextLevel);

    // 扣除升级费用
    GameManager::getInstance()->addGold(-config.goldCost);

    // 更新内部等级和上限
    m_barrackLevel = nextLevel;
    m_maxCostLimit = config.maxCostLimit;

    // 更新 GameManager 中的存档数据
    GameManager::getInstance()->updateBarrackLevel(this->getPosition(), m_barrackLevel);

    // 重新计算当前 Cost
    updateCurrentCostUsed();

    // 播放简单的视觉反馈（缩放弹跳）
    auto scaleUp = ScaleTo::create(0.2f, this->getScale() * 1.2f);
    auto scaleDown = ScaleTo::create(0.2f, this->getScale());
    this->runAction(Sequence::create(scaleUp, scaleDown, nullptr));

    // 发送事件通知 UI（按钮状态更新、解锁状态更新）
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_BARRACK_UPGRADED");
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_COST_UPDATED");
}