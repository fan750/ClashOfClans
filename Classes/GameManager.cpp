#include "GameManager.h"

USING_NS_CC;

GameManager* GameManager::s_instance = nullptr;

GameManager::GameManager() : m_gold(0), m_elixir(0) {}

GameManager* GameManager::getInstance() {
    if (!s_instance) {
        s_instance = new GameManager();
    }
    return s_instance;
}

void GameManager::initAccount(int gold, int elixir) {
    m_gold = gold;
    m_elixir = elixir;
}

void GameManager::addGold(int amount) {
    m_gold += amount;
    CCLOG("Current Gold: %d", m_gold);

    // 【关键】发送通知：告诉全游戏“金币更新了”
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_GOLD");
}

void GameManager::addElixir(int amount) {
    m_elixir += amount;
    // 发送通知
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_ELIXIR");
}