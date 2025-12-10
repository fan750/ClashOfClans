//GameUI.h
#ifndef __GAME_UI_H__
#define __GAME_UI_H__

#include "cocos2d.h"
#include "Troop.h" // 添加Troop.h引用

class GameUI : public cocos2d::Layer {
public:
    virtual bool init() override;
    void initResourceLabels();                          // 初始化资源标签
    void initTroopLabels();                             // 初始化兵种标签
    CREATE_FUNC(GameUI);

    // 刷新显示的文字
    void updateLabels();

private:
    cocos2d::Label* m_goldLabel;
    cocos2d::Label* m_elixirLabel;
    std::map<TroopType, cocos2d::Label*> m_troopLabels; // 兵种标签
};

#endif // __GAME_UI_H__