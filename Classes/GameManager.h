#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

#include "cocos2d.h"

class GameManager {
public:
    static GameManager* getInstance();

    // 初始化初始资源
    void initAccount(int gold, int elixir);

    // 增加/减少资源
    void addGold(int amount);
    void addElixir(int amount);

    // 获取当前资源
    int getGold() const { return m_gold; }
    int getElixir() const { return m_elixir; }

private:
    GameManager();
    static GameManager* s_instance;

    int m_gold;
    int m_elixir;
};

#endif // __GAME_MANAGER_H__