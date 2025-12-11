//GameManager.cpp
#include "GameManager.h"
#include"Building.h"
USING_NS_CC;

GameManager* GameManager::s_instance = nullptr;

// 构造函数里初始化标记为 false
GameManager::GameManager()
    : m_gold(0), m_elixir(0), m_isInitialized(false)
{
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
    m_maxElixir += amount;
    // 也可以发个事件更新UI，显示容量变化
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_ELIXIR");
}

void GameManager::modifyMaxGold(int amount) {
    m_maxGold += amount;
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_GOLD");
}

// 【新增实现】保存建筑
void GameManager::addHomeBuilding(BuildingType type, Vec2 pos, int level) {
    BuildingData data;
    data.type = type;
    data.position = pos;
    data.level = level;
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

// 【新增实现】获取列表
void GameManager::addTroops(TroopType type, int amount) // 添加各兵种数量
{
    m_troopCounts[type] += amount;
    if (m_troopCounts[type] < 0)
    {
        m_troopCounts[type] = 0; // 防止数量为负
    }
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
}