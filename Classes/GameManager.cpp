//GameManager.cpp
#include "GameManager.h"
#include"Building.h"
#include "BattleManager.h"
#include <algorithm>
USING_NS_CC;

GameManager* GameManager::s_instance = nullptr;

// 构造函数里初始化标记为 false
GameManager::GameManager()
    : m_gold(0), m_elixir(0), m_isInitialized(false)
{
    m_timeAccelerateCooldownEnd = std::chrono::steady_clock::time_point::min();
    //初始化各兵种数量为0
    m_troopCounts[TroopType::BARBARIAN] = 0;
    m_troopCounts[TroopType::ARCHER] = 0;
    m_troopCounts[TroopType::GIANT] = 0;
    m_troopCounts[TroopType::BOMBERMAN] = 0;
    m_troopCounts[TroopType::DRAGON] = 0;
}

GameManager* GameManager::getInstance()
{
    if (!s_instance)
    {
        s_instance = new GameManager();
    }
    return s_instance;
}

void GameManager::initAccount(int gold, int elixir)
{
    if (m_isInitialized) return;

    m_gold = gold;
    m_elixir = elixir;

    // 初始自带一点容量 (比如大本营自带的容量)
    m_maxGold = 1000;
    m_maxElixir = 1000;

    m_isInitialized = true;
}

void GameManager::addGold(int amount) {
    m_gold += amount;
    // 核心机制：安全存储-容量限制
    // 钱不能超过上限
    if (m_gold > m_maxGold) m_gold = m_maxGold;
    if (m_gold < 0) m_gold = 0; // 消费时不能扣成负数

    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_GOLD");
}

void GameManager::addElixir(int amount) {
    m_elixir += amount;
    // 核心机制：安全存储-容量限制
    if (m_elixir > m_maxElixir) m_elixir = m_maxElixir;
    if (m_elixir < 0) m_elixir = 0;

    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_ELIXIR");
}

// 当建造了圣水瓶，上限增加
void GameManager::modifyMaxElixir(int amount) {
    m_maxElixir = amount;
    // 也可以发个事件更新UI，显示容量变化
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_ELIXIR");
}

void GameManager::modifyMaxGold(int amount) {
    m_maxGold = amount;
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_GOLD");
}

// 【新增实现】保存建筑
void GameManager::addHomeBuilding(BuildingType type, Vec2 pos, int level) {
    BuildingData data;
    data.type = type;
    data.position = pos;
    data.level = level;
    // 军营默认等级为1，其他建筑为0
    data.barrackLevel = (type == BuildingType::BARRACKS) ? 1 : 0;
    m_homeBuildings.push_back(data);
    CCLOG("Saved building: type=%d pos=(%f,%f) level=%d", (int)type, pos.x, pos.y, level);
}

const std::vector<BuildingData>& GameManager::getHomeBuildings() { return m_homeBuildings; } // 获取当前建筑列表

// 更新已保存建筑的等级（按 type+pos 精确匹配）
void GameManager::updateHomeBuildingLevel(BuildingType type, Vec2 pos, int level) {
    for (auto &b : m_homeBuildings) {
        if (b.type == type && b.position.equals(pos)) {
            b.level = level;
            CCLOG("Updated stored building level: type=%d pos=(%f,%f) level=%d", (int)type, pos.x, pos.y, level);
            return;
        }
    }
    // 如果没找到，追加一个记录，以防止丢失
    addHomeBuilding(type, pos, level);
}

// 获取当前军营等级
int GameManager::getBarrackLevel() const {
    // 查找军营建筑
    for (const auto& building : m_homeBuildings) {
        if (building.type == BuildingType::BARRACKS) {
            return building.barrackLevel;
        }
    }
    return 0; // 没有军营返回0
}

// 更新军营等级
void GameManager::updateBarrackLevel(Vec2 pos, int level) {
    // 更新存储的军营等级
    for (auto& building : m_homeBuildings) {
        if (building.type == BuildingType::BARRACKS && building.position.equals(pos)) {
            building.barrackLevel = level;
            CCLOG("Updated barrack level to %d at position (%f, %f)", level, pos.x, pos.y);
            return;
        }
    }

    // 如果没找到，可能是新建造的军营
    CCLOG("Barracks not found at position (%f, %f), adding new record", pos.x, pos.y);
    BuildingData newData;
    newData.type = BuildingType::BARRACKS;
    newData.position = pos;
    newData.level = 1;
    newData.barrackLevel = level;
    m_homeBuildings.push_back(newData);
}

// 【新增实现】获取列表
void GameManager::addTroops(TroopType type, int amount) // 添加各兵种数量
{
    m_troopCounts[type] += amount;
    if (m_troopCounts[type] < 0)
    {
        m_troopCounts[type] = 0; // 防止数量为负
    }
    // 发送事件通知军营更新cost
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_TROOPS");
}

void GameManager::consumeTroops(TroopType type, int amount) // 减少各兵种数量
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

int GameManager::getTroopCount(TroopType type) const // 获取指定兵种的数量
{
    auto it = m_troopCounts.find(type);
    if (it != m_troopCounts.end())
    {
        return it->second;
    }
    return 0;
}

void GameManager::setTroopCount(TroopType type, int count) // 设置兵种数量（用于战斗结束后同步）

{
    m_troopCounts[type] = count;
    if (m_troopCounts[type] < 0)
    {
        m_troopCounts[type] = 0;
    }
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_TROOPS");
}

const std::map<TroopType, int>& GameManager::getAllTroopCounts() const // 获取所有兵种数量
{
    return m_troopCounts;
}

void GameManager::clear()
{
    // 清空建筑列表
    m_homeBuildings.clear();

    // (可选) 重置金币
    m_gold = 500;
    m_elixir = 500;
    m_timeAccelerateCooldownEnd = std::chrono::steady_clock::time_point::min();
}

void GameManager::setTimeAccelerateCooldownEnd(const std::chrono::steady_clock::time_point& end)
{
    m_timeAccelerateCooldownEnd = end;
}

std::chrono::steady_clock::time_point GameManager::getTimeAccelerateCooldownEnd() const
{
    return m_timeAccelerateCooldownEnd;
}

float GameManager::getTimeAccelerateCooldownRemaining() const
{
    auto now = std::chrono::steady_clock::now();
    if (now >= m_timeAccelerateCooldownEnd) return 0.0f;
    return std::chrono::duration_cast<std::chrono::duration<float>>(m_timeAccelerateCooldownEnd - now).count();
}

// 建筑升级相关代码
void GameManager::scheduleBuildingUpgrade(Building* building, float duration)
{
    if (!building || duration <= 0.0f) return;

    int taskId = ++m_nextUpgradeTaskId;
    PendingBuildingUpgrade task;
    task.id = taskId;
    task.type = building->getBuildingType();
    task.position = building->getPosition();
    task.endTime = std::chrono::steady_clock::now() + std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<float>(duration));
    m_pendingUpgrades.push_back(task);

    ensureUpgradeTicker();
}

void GameManager::ensureUpgradeTicker()
{
    if (!m_pendingUpgrades.empty())
    {
        Director::getInstance()->getScheduler()->unschedule("GameManagerUpgradeTicker", this);
        Director::getInstance()->getScheduler()->schedule([this](float dt) {
            this->updatePendingUpgrades(dt);
        }, this, 0.0f, false, "GameManagerUpgradeTicker");
        m_upgradeTickerScheduled = true;
    }
}

void GameManager::updatePendingUpgrades(float dt)
{
    auto now = std::chrono::steady_clock::now();
    std::vector<int> toComplete;
    toComplete.reserve(m_pendingUpgrades.size());

    for (const auto& task : m_pendingUpgrades)
    {
        if (now >= task.endTime)
        {
            toComplete.push_back(task.id);
        }
    }

    for (int id : toComplete)
    {
        completeBuildingUpgrade(id);
    }

    if (m_pendingUpgrades.empty())
    {
        Director::getInstance()->getScheduler()->unschedule("GameManagerUpgradeTicker", this);
        m_upgradeTickerScheduled = false;
    }
}

void GameManager::completeBuildingUpgrade(int taskId)
{
    auto it = std::find_if(m_pendingUpgrades.begin(), m_pendingUpgrades.end(), [taskId](const PendingBuildingUpgrade& item) {
        return item.id == taskId;
    });

    if (it == m_pendingUpgrades.end()) return;

    BuildingType type = it->type;
    Vec2 position = it->position;
    m_pendingUpgrades.erase(it);

    // 1. 尝试在当前场景中找到对应的建筑实体
    auto building = BattleManager::getInstance()->findBuildingAtPosition(position, type, 5.0f);
    if (building)
    {
        // 如果找到了，直接调用实体的升级完成逻辑（播放动画、更新数值等）
        building->onUpgradeFinished();
    }
    else
    {
        // 2. 如果没找到（说明玩家不在主场景，或者建筑还没加载出来），则只更新后台数据
        // 这样下次进入场景时，建筑会直接以新等级加载
        incrementStoredBuildingLevel(type, position);
    }

    if (m_pendingUpgrades.empty())
    {
        Director::getInstance()->getScheduler()->unschedule("GameManagerUpgradeTicker", this);
        m_upgradeTickerScheduled = false;
    }
}

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

void GameManager::incrementStoredBuildingLevel(BuildingType type, Vec2 position)
{
    for (auto& data : m_homeBuildings)
    {
        if (data.type == type && data.position.distance(position) <= 5.0f)
        {
            data.level++;
            updateHomeBuildingLevel(type, position, data.level);
            if (type == BuildingType::TOWN_HALL)
            {
                setTown_Hall_Level(getTown_Hall_Level() + 1);
            }
            return;
        }
    }
}

void GameManager::applyBuildingUpgradeBoost(BuildingType type, Vec2 position, float multiplier, float duration)
{
    for (auto& task : m_pendingUpgrades)
    {
        if (task.type == type && task.position.distance(position) <= 5.0f)
        {
            auto now = std::chrono::steady_clock::now();
            if (now >= task.endTime) return;

            // 计算剩余时间 (相当于剩余的工作量)
            float remaining = std::chrono::duration_cast<std::chrono::duration<float>>(task.endTime - now).count();
            
            // 计算在加速持续时间内能完成的最大工作量
            float maxBoostWork = duration * multiplier;
            float timeReduction = 0.0f;

            if (remaining <= maxBoostWork)
            {
                // 情况1：加速足以覆盖剩余的所有工作
                // 实际耗时 = remaining / multiplier
                // 节省时间 = remaining - (remaining / multiplier)
                timeReduction = remaining * (1.0f - 1.0f / multiplier);
            }
            else
            {
                // 情况2：加速时间结束时工作还没做完
                // 我们加速了 duration 秒
                // 节省时间 = duration * (multiplier - 1)
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