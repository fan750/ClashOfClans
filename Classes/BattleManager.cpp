#include "BattleManager.h"
#include "Troop.h" // 【关键修改】在 cpp 里引用 Troop

USING_NS_CC;

BattleManager* BattleManager::s_instance = nullptr;

BattleManager::BattleManager() {
}

BattleManager* BattleManager::getInstance() {
    if (!s_instance) {
        s_instance = new BattleManager();
    }
    return s_instance;
}

void BattleManager::addBuilding(Building* building) {
    if (!m_buildings.contains(building)) {
        m_buildings.pushBack(building);
    }
}

void BattleManager::removeBuilding(Building* building) {
    if (m_buildings.contains(building)) {
        m_buildings.eraseObject(building);
    }
}

Building* BattleManager::findClosestBuilding(Vec2 position) {
    Building* closestNode = nullptr;
    float minDistance = 999999.0f;

    for (auto building : m_buildings) {
        if (building->isDead()) continue;
        float distance = position.distance(building->getPosition());
        if (distance < minDistance) {
            minDistance = distance;
            closestNode = building;
        }
    }
    return closestNode;
}

Building* BattleManager::findClosestBuildingOfType(Vec2 position, BuildingType type) {
    Building* closestNode = nullptr;
    float minDistance = 999999.0f;

    for (auto building : m_buildings) {
        if (building->isDead()) continue;
        if (building->getBuildingType() == type) {
            float distance = position.distance(building->getPosition());
            if (distance < minDistance) {
                minDistance = distance;
                closestNode = building;
            }
        }
    }
    return closestNode;
}

void BattleManager::addTroop(Troop* troop) {
    if (!m_troops.contains(troop)) {
        m_troops.pushBack(troop);
    }
}

void BattleManager::removeTroop(Troop* troop) {
    if (m_troops.contains(troop)) {
        m_troops.eraseObject(troop);
    }
}

Troop* BattleManager::findClosestTroop(Vec2 position, float range) {
    Troop* closestTroop = nullptr;
    float minDistance = range;

    for (auto troop : m_troops) {
        if (troop->isDead()) continue;
        float distance = position.distance(troop->getPosition());
        if (distance < minDistance) {
            minDistance = distance;
            closestTroop = troop;
        }
    }
    return closestTroop;
}

void BattleManager::dealAreaDamage(Vec2 center, float radius, int damage) {
    // 【关键修复】先收集，后扣血，防止迭代器崩溃
    std::vector<Building*> targetsToHit;

    for (auto building : m_buildings) {
        if (building->isDead()) continue;
        if (building->getPosition().distance(center) <= radius) {
            targetsToHit.push_back(building);
        }
    }

    for (auto building : targetsToHit) {
        if (!building->isDead()) {
            building->takeDamage(damage);
        }
    }
}

void BattleManager::clear() {
    m_buildings.clear();
    m_troops.clear();
}