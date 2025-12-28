#include "BattleManager.h"
#include "Troop.h"
#include "GameManager.h"
#include "Building.h"
USING_NS_CC;

// 定义静态成员变量
BattleManager* BattleManager::s_instance = nullptr;

// 构造函数
BattleManager::BattleManager() {}

// 获取单例实例
BattleManager* BattleManager::getInstance()
{
    if (!s_instance)
    {
        s_instance = new BattleManager();
    }
    return s_instance;
}

// 添加建筑到管理列表
void BattleManager::addBuilding(Building* building)
{
    if (!m_buildings.contains(building))
    {
        m_buildings.pushBack(building);
    }
}

// 从管理列表中移除建筑
void BattleManager::removeBuilding(Building* building)
{
    if (m_buildings.contains(building))
    {
        m_buildings.eraseObject(building);
    }
}

// 查找距离指定位置最近的建筑
Building* BattleManager::findClosestBuilding(Vec2 position)
{
    Building* closestNode = nullptr;
    float minDistance = 999999.0f;

    for (auto building : m_buildings)
    {
        if (building->isDead())
        {
            continue;
        }
        // 过滤掉陷阱，防止部队攻击陷阱
        if (building->getBuildingType() == BuildingType::TRAP)
        {
            continue;
        }

        float distance = position.distance(building->getPosition());
        if (distance < minDistance)
        {
            minDistance = distance;
            closestNode = building;
        }
    }
    return closestNode;
}

// 查找距离指定位置最近的指定类型建筑
Building* BattleManager::findClosestBuildingOfType(Vec2 position, BuildingType type)
{
    Building* closestNode = nullptr;
    float minDistance = 999999.0f;

    for (auto building : m_buildings)
    {
        if (building->isDead())
        {
            continue;
        }
        if (building->getBuildingType() == type)
        {
            float distance = position.distance(building->getPosition());
            if (distance < minDistance)
            {
                minDistance = distance;
                closestNode = building;
            }
        }
    }
    return closestNode;
}

// 添加兵种到管理列表
void BattleManager::addTroop(Troop* troop)
{
    if (!m_troops.contains(troop))
    {
        m_troops.pushBack(troop);
    }
}

// 从管理列表中移除兵种
void BattleManager::removeTroop(Troop* troop)
{
    if (m_troops.contains(troop))
    {
        m_troops.eraseObject(troop);
    }
}

// 查找指定范围内特定移动类型的最近兵种
Troop* BattleManager::findClosestTroop(Vec2 position, float range, Troop::MovementType typeFilter)
{
    Troop* closestTroop = nullptr;
    float minDistance = range;

    for (Troop* troop : m_troops)
    {
        if (!troop || troop->isDead())
        {
            continue; // 跳过已死亡士兵
        }
        if (troop->getMovementType() != typeFilter)
        {
            continue; // 关键：只找指定移动类型的兵种
        }
        float distance = position.distance(troop->getPosition());
        if (distance < minDistance)
        {
            minDistance = distance;
            closestTroop = troop;
        }
    }
    return closestTroop;
}

// 为防御建筑寻找最佳的攻击目标（根据建筑对空对地属性）
Troop* BattleManager::findClosestTroopForBuilding(Building* building)
{
    if (!building)
    {
        return nullptr;
    }

    float range = building->getAttackRange();  // 使用建筑的实际攻击范围
    Troop* closestTroop = nullptr;
    float minDistance = range;

    // 根据建筑类型决定可以攻击的目标
    bool canAttackAir = false;
    bool canAttackGround = true; // 默认都能打地面

    if (building->getBuildingType() == BuildingType::ARCHER_TOWER)
    {
        canAttackAir = true; // 箭塔可以打空中
    }
    else if (building->getBuildingType() == BuildingType::CANNON)
    {
        canAttackAir = false; // 加农炮不能打空中
    }

    for (Troop* troop : m_troops)
    {
        if (!troop || troop->isDead())
        {
            continue;
        }

        // 核心过滤逻辑
        Troop::MovementType troopType = troop->getMovementType();
        if ((troopType == Troop::MovementType::AIR && !canAttackAir) ||
            (troopType == Troop::MovementType::GROUND && !canAttackGround))
        {
            continue; // 跳过不能攻击的目标类型
        }

        float distance = building->getPosition().distance(troop->getPosition());
        if (distance < minDistance)
        {
            minDistance = distance;
            closestTroop = troop;
        }
    }

    return closestTroop;
}

// 对指定范围内的所有敌对单位造成伤害
void BattleManager::dealAreaDamage(Vec2 center, float radius, int damage)
{
    std::vector<Building*> targetsToHit;
    std::vector<Troop*> troopsToHit;

    // 筛选范围内的建筑
    for (auto building : m_buildings)
    {
        if (building->isDead())
        {
            continue;
        }
        if (building->getPosition().distance(center) <= radius)
        {
            targetsToHit.push_back(building);
        }
    }

    // 筛选范围内的兵种（排除空军）
    for (auto troop : m_troops)
    {
        if (!troop || troop->isDead())
        {
            continue;
        }
        if (troop->getMovementType() == Troop::MovementType::AIR)
        {
            continue;
        }
        if (troop->getPosition().distance(center) <= radius)
        {
            troopsToHit.push_back(troop);
        }
    }

    // 对建筑造成伤害
    for (auto building : targetsToHit)
    {
        if (!building->isDead())
        {
            building->takeDamage(damage);
        }
    }

    // 对兵种造成伤害
    for (auto troop : troopsToHit)
    {
        if (troop && !troop->isDead())
        {
            troop->takeDamage(damage);
        }
    }
}

// 初始化可投放兵种数据
void BattleManager::initAvailableTroops(const std::map<TroopType, int>& availableTroops)
{
    m_availableTroops = availableTroops;

    // 初始化已投放兵种数量为0
    m_deployedTroops.clear();
    m_deployedTroops[TroopType::BARBARIAN] = 0;
    m_deployedTroops[TroopType::ARCHER] = 0;
    m_deployedTroops[TroopType::GIANT] = 0;
    m_deployedTroops[TroopType::BOMBERMAN] = 0;
    m_deployedTroops[TroopType::DRAGON] = 0;

    // 记录初始兵种数量（从经营模式获取的总量）
    m_initialTroops = availableTroops;
}

// 检查是否还能投放指定兵种
bool BattleManager::canDeployTroop(TroopType type)
{
    auto it = m_availableTroops.find(type);
    if (it != m_availableTroops.end())
    {
        return it->second > 0;
    }
    return false;
}

// 投放兵种（减少可用数量，增加已投放数量）
void BattleManager::deployTroop(TroopType type)
{
    auto it = m_availableTroops.find(type);
    if (it != m_availableTroops.end() && it->second > 0)
    {
        it->second--;
        // 增加已投放数量
        m_deployedTroops[type]++;
    }
}

// 记录士兵死亡（减少已投放数量）
void BattleManager::onTroopDied(TroopType type)
{
    auto it = m_deployedTroops.find(type);
    if (it != m_deployedTroops.end() && it->second > 0)
    {
        it->second--;
    }
}

// 获取已投放的兵种数量
const std::map<TroopType, int>& BattleManager::getDeployedTroops() const
{
    return m_deployedTroops;
}

// 获取战斗中死亡的兵种数量
std::map<TroopType, int> BattleManager::getDeathCounts() const
{
    std::map<TroopType, int> deathCounts;

    // 死亡数量 = 初始数量 - 剩余可投放数量 - 当前场上存活数量
    for (const auto& pair : m_initialTroops)
    {
        TroopType type = pair.first;
        int initialCount = pair.second;

        // 获取剩余可投放数量
        int availableCount = 0;
        auto availableIt = m_availableTroops.find(type);
        if (availableIt != m_availableTroops.end())
        {
            availableCount = availableIt->second;
        }

        // 获取当前场上存活数量
        int aliveCount = 0;
        for (auto troop : m_troops)
        {
            if (!troop->isDead() && troop->getTroopType() == type)
            {
                aliveCount++;
            }
        }

        // 计算死亡数量
        int deathCount = initialCount - availableCount - aliveCount;
        if (deathCount > 0)
        {
            deathCounts[type] = deathCount;
        }
    }

    return deathCounts;
}

// 获取指定兵种的剩余可投放数量
int BattleManager::getAvailableTroopCount(TroopType type)
{
    auto it = m_availableTroops.find(type);
    if (it != m_availableTroops.end())
    {
        return it->second;
    }
    return 0;
}

// 获取所有可用兵种数量
const std::map<TroopType, int>& BattleManager::getAllAvailableTroops() const
{
    return m_availableTroops;
}

// 获取场上所有兵种的引用
const Vector<Troop*>& BattleManager::getTroops() const
{
    return m_troops;
}

// 获取场上所有建筑的引用
const Vector<Building*>& BattleManager::getBuildings() const
{
    return m_buildings;
}

// 根据等级重新调整场上特定类型兵种的属性
void BattleManager::rescaleTroopsForType(TroopType type, int level)
{
    for (Troop* troop : m_troops)
    {
        if (!troop || troop->isDead())
        {
            continue;
        }
        if (troop->getTroopType() != type)
        {
            continue;
        }
        troop->rescaleStatsForLevel(level);
    }
}

// 清空所有数据（战斗结束或重置时调用）
void BattleManager::clear()
{
    m_buildings.clear();
    m_troops.clear();

    // 清空可部署列表
    m_availableTroops.clear();
    m_deployedTroops.clear();
    m_initialTroops.clear();
}

// 查找指定位置附近的指定类型建筑（用于存档恢复或升级任务匹配）
Building* BattleManager::findBuildingAtPosition(Vec2 position, BuildingType type, float tolerance)
{
    for (auto building : m_buildings)
    {
        if (!building || building->isDead())
        {
            continue;
        }
        if (building->getBuildingType() != type)
        {
            continue;
        }
        if (building->getPosition().distance(position) <= tolerance)
        {
            return building;
        }
    }
    return nullptr;
}