#include "GameManager.h"
#include "Building.h"
#include "BattleManager.h"
#include <algorithm>
USING_NS_CC;

// 定义静态成员变量
GameManager* GameManager::s_instance = nullptr;

// 构造函数：初始化成员变量
GameManager::GameManager()
    : m_gold(0)
    , m_elixir(0)
    , m_isInitialized(false)
    , town_hall_level(1)
    , bar_level(1)
    , giant_level(1)
    , arc_level(1)
    , bom_level(1)
    , dragon_level(1)
{
    // 设置时间加速冷却时间点为最小值
    m_timeAccelerateCooldownEnd = std::chrono::steady_clock::time_point::min();

    // 初始化各兵种数量为0
    m_troopCounts[TroopType::BARBARIAN] = 0;
    m_troopCounts[TroopType::ARCHER] = 0;
    m_troopCounts[TroopType::GIANT] = 0;
    m_troopCounts[TroopType::BOMBERMAN] = 0;
    m_troopCounts[TroopType::DRAGON] = 0;
}

// 获取单例实例
GameManager* GameManager::getInstance()
{
    if (!s_instance)
    {
        s_instance = new GameManager();
    }
    return s_instance;
}

// 初始化账户资源
void GameManager::initAccount(int gold, int elixir)
{
    // 防止重置
    if (m_isInitialized)
    {
        return;
    }

    m_gold = gold;
    m_elixir = elixir;

    // 初始自带一定容量 (大本营自带的容量)
    m_maxGold = 1000;
    m_maxElixir = 1000;

    m_isInitialized = true;
}

// 增加金币 (带容量限制)
void GameManager::addGold(int amount)
{
    m_gold += amount;

    // 核心机制：安全存储-容量限制
    // 钱不能超过上限，也不能扣成负数
    if (m_gold > m_maxGold)
    {
        m_gold = m_maxGold;
    }
    if (m_gold < 0)
    {
        m_gold = 0;
    }

    // 发送事件通知UI更新
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_GOLD");
}

// 增加圣水 (带容量限制)
void GameManager::addElixir(int amount)
{
    m_elixir += amount;

    // 核心机制：安全存储-容量限制
    if (m_elixir > m_maxElixir)
    {
        m_elixir = m_maxElixir;
    }
    if (m_elixir < 0)
    {
        m_elixir = 0;
    }

    // 发送事件通知UI更新
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_ELIXIR");
}

// 修改金币上限
void GameManager::modifyMaxGold(int amount)
{
    m_maxGold = amount;
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_GOLD");
}

// 修改圣水上限
void GameManager::modifyMaxElixir(int amount)
{
    m_maxElixir = amount;
    // 发送事件更新UI，显示容量变化
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_ELIXIR");
}

// 保存建筑数据
void GameManager::addHomeBuilding(BuildingType type, Vec2 pos, int level)
{
    BuildingData data;
    data.type = type;
    data.position = pos;
    data.level = level;
    // 军营默认等级为1，其他建筑为0
    data.barrackLevel = (type == BuildingType::BARRACKS) ? 1 : 0;
    m_homeBuildings.push_back(data);
}

// 获取当前建筑列表
const std::vector<BuildingData>& GameManager::getHomeBuildings()
{
    return m_homeBuildings;
}

// 更新已保存建筑的等级（按 type+pos 精确匹配）
void GameManager::updateHomeBuildingLevel(BuildingType type, Vec2 pos, int level)
{
    for (auto& b : m_homeBuildings)
    {
        // 使用距离判断代替精确 equals，防止浮点数误差导致找不到建筑
        if (b.type == type && b.position.distance(pos) < 5.0f)
        {
            b.level = level;
            return;
        }
    }
    // 若没找到，追加一条记录以防止丢失
    addHomeBuilding(type, pos, level);
}

// 获取当前军营等级
int GameManager::getBarrackLevel() const
{
    // 查找军营建筑
    for (const auto& building : m_homeBuildings)
    {
        if (building.type == BuildingType::BARRACKS)
        {
            return building.barrackLevel;
        }
    }
    return 0; // 没有军营返回0
}

// 更新军营等级
void GameManager::updateBarrackLevel(Vec2 pos, int level)
{
    // 更新存储的军营等级
    for (auto& building : m_homeBuildings)
    {
        // 使用距离判断代替精确 equals
        if (building.type == BuildingType::BARRACKS && building.position.distance(pos) < 5.0f)
        {
            building.barrackLevel = level;
            return;
        }
    }

    // 若没找到，则新增一个（逻辑上不应发生，除非是新建的军营）
    BuildingData newData;
    newData.type = BuildingType::BARRACKS;
    newData.position = pos;
    newData.level = 1;
    newData.barrackLevel = level;
    m_homeBuildings.push_back(newData);
}

// 增加兵种数量
void GameManager::addTroops(TroopType type, int amount)
{
    m_troopCounts[type] += amount;
    if (m_troopCounts[type] < 0)
    {
        m_troopCounts[type] = 0; // 防止数量为负
    }
    // 发送事件通知军营更新cost
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_TROOPS");
}

// 减少兵种数量
void GameManager::consumeTroops(TroopType type, int amount)
{
    auto it = m_troopCounts.find(type);
    if (it != m_troopCounts.end())
    {
        it->second -= amount;
        if (it->second < 0)
        {
            it->second = 0; // 防止数量为负
        }
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_TROOPS");
    }
}

// 获取指定兵种的数量
int GameManager::getTroopCount(TroopType type) const
{
    auto it = m_troopCounts.find(type);
    if (it != m_troopCounts.end())
    {
        return it->second;
    }
    return 0;
}

// 设置兵种数量（用于战斗结束后同步）
void GameManager::setTroopCount(TroopType type, int count)
{
    m_troopCounts[type] = count;
    if (m_troopCounts[type] < 0)
    {
        m_troopCounts[type] = 0;
    }
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_TROOPS");
}

// 获取所有兵种数量
const std::map<TroopType, int>& GameManager::getAllTroopCounts() const
{
    return m_troopCounts;
}

// 重置所有数据
void GameManager::clear()
{
    // 清空建筑列表
    m_homeBuildings.clear();

    // 重置金币和圣水
    m_gold = 500;
    m_elixir = 500;
    m_timeAccelerateCooldownEnd = std::chrono::steady_clock::time_point::min();
}

// 设置时间加速冷却结束时间点
void GameManager::setTimeAccelerateCooldownEnd(const std::chrono::steady_clock::time_point& end)
{
    m_timeAccelerateCooldownEnd = end;
}

// 获取时间加速冷却结束时间点
std::chrono::steady_clock::time_point GameManager::getTimeAccelerateCooldownEnd() const
{
    return m_timeAccelerateCooldownEnd;
}

// 获取时间加速冷却剩余时间（秒）
float GameManager::getTimeAccelerateCooldownRemaining() const
{
    auto now = std::chrono::steady_clock::now();
    if (now >= m_timeAccelerateCooldownEnd)
    {
        return 0.0f;
    }
    return std::chrono::duration_cast<std::chrono::duration<float>>(m_timeAccelerateCooldownEnd - now).count();
}

// 调度建筑升级任务
void GameManager::scheduleBuildingUpgrade(Building* building, float duration)
{
    if (!building || duration <= 0.0f)
    {
        return;
    }

    int taskId = ++m_nextUpgradeTaskId;
    PendingBuildingUpgrade task;
    task.id = taskId;
    task.type = building->getBuildingType();
    task.position = building->getPosition();
    // 计算升级结束时间点
    task.endTime = std::chrono::steady_clock::now() + std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<float>(duration));
    m_pendingUpgrades.push_back(task);

    ensureUpgradeTicker();
}

// 确保升级计时器已启动
void GameManager::ensureUpgradeTicker()
{
    if (!m_pendingUpgrades.empty())
    {
        Director::getInstance()->getScheduler()->unschedule("GameManagerUpgradeTicker", this);
        Director::getInstance()->getScheduler()->schedule([this](float dt)
            {
                this->updatePendingUpgrades(dt);
            }, this, 0.0f, false, "GameManagerUpgradeTicker");
        m_upgradeTickerScheduled = true;
    }
}

// 更新待升级任务列表
void GameManager::updatePendingUpgrades(float dt)
{
    auto now = std::chrono::steady_clock::now();
    std::vector<int> toComplete;
    toComplete.reserve(m_pendingUpgrades.size());

    // 查找所有已到期的任务
    for (const auto& task : m_pendingUpgrades)
    {
        if (now >= task.endTime)
        {
            toComplete.push_back(task.id);
        }
    }

    // 完成到期的任务
    for (int id : toComplete)
    {
        completeBuildingUpgrade(id);
    }

    // 如果没有待处理任务，停止计时器
    if (m_pendingUpgrades.empty())
    {
        Director::getInstance()->getScheduler()->unschedule("GameManagerUpgradeTicker", this);
        m_upgradeTickerScheduled = false;
    }
}

// 完成建筑升级任务
void GameManager::completeBuildingUpgrade(int taskId)
{
    auto it = std::find_if(m_pendingUpgrades.begin(), m_pendingUpgrades.end(), [taskId](const PendingBuildingUpgrade& item)
        {
        return item.id == taskId;
        });

    if (it == m_pendingUpgrades.end())
    {
        return;
    }

    BuildingType type = it->type;
    Vec2 position = it->position;
    m_pendingUpgrades.erase(it);

    // 尝试在当前场景中找到对应的建筑实体
    auto building = BattleManager::getInstance()->findBuildingAtPosition(position, type, 5.0f);
    if (building)
    {
        // 如果找到了，直接调用实体的升级完成逻辑（播放动画、更新数值等）
        building->onUpgradeFinished();
    }
    else
    {
        // 如果没找到（说明玩家不在主场景，或者建筑还没加载出来），则只更新后台数据
        // 这样下次进入场景时，建筑会直接以新等级加载
        incrementStoredBuildingLevel(type, position);
    }

    // 检查是否还有待处理任务
    if (m_pendingUpgrades.empty())
    {
        Director::getInstance()->getScheduler()->unschedule("GameManagerUpgradeTicker", this);
        m_upgradeTickerScheduled = false;
    }
}

// 检查是否有待升级任务
bool GameManager::hasPendingUpgrade(BuildingType type, cocos2d::Vec2 position, float tolerance) const
{
    for (const auto& task : m_pendingUpgrades)
    {
        if (task.type == type && task.position.distance(position) <= tolerance)
        {
            return true;
        }
    }
    return false;
}

// 增加存储中的建筑等级
void GameManager::incrementStoredBuildingLevel(BuildingType type, Vec2 position)
{
    for (auto& data : m_homeBuildings)
    {
        if (data.type == type && data.position.distance(position) <= 5.0f)
        {
            data.level++;
            updateHomeBuildingLevel(type, position, data.level);
            // 如果是大本营，增加全局大本营等级
            if (type == BuildingType::TOWN_HALL)
            {
                setTown_Hall_Level(getTown_Hall_Level() + 1);
            }
            return;
        }
    }
}

// 应用建筑升级加速
void GameManager::applyBuildingUpgradeBoost(BuildingType type, Vec2 position, float multiplier, float duration)
{
    for (auto& task : m_pendingUpgrades)
    {
        if (task.type == type && task.position.distance(position) <= 5.0f)
        {
            auto now = std::chrono::steady_clock::now();
            if (now >= task.endTime)
            {
                return;
            }

            // 计算剩余时间 (相当于剩余的工作量)
            float remaining = std::chrono::duration_cast<std::chrono::duration<float>>(task.endTime - now).count();

            // 计算在加速持续时间内能完成的最大工作量
            float maxBoostWork = duration * multiplier;
            float timeReduction = 0.0f;

            if (remaining <= maxBoostWork)
            {
                // 加速足以覆盖剩余的所有工作
                timeReduction = remaining * (1.0f - 1.0f / multiplier);
            }
            else
            {
                // 加速时间结束时工作还没做完
                // 我们加速了 duration 秒
                timeReduction = duration * (multiplier - 1.0f);
            }

            // 提前结束时间
            task.endTime -= std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<float>(timeReduction));

            // 重新触发 ticker 检查
            ensureUpgradeTicker();
            return;
        }
    }
}