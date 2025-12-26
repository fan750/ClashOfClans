//BattleScene.h
#ifndef __BATTLE_SCENE_H__
#define __BATTLE_SCENE_H__
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Troop.h"
#include "Building.h" // 确保包含
#include <vector>

class BattleScene : public cocos2d::Scene
{
private:
    // UI helpers
    void createSelectButton(const std::string& title, cocos2d::Color3B color, TroopType type, int index);
    void initTroopCountLabels();
    void updateTroopCountLabels();

    // battle end handler
    void onBattleEnd();
    // 胜负判定函数
    void checkBattleEnd();
    // state
    TroopType m_selectedType;
    cocos2d::Label* m_infoLabel;
    std::map<TroopType, cocos2d::Label*> m_troopCountLabels;
    cocos2d::DrawNode* m_deployAreaVisual; // 投放区域可视化

public:
    static cocos2d::Scene* createScene(int levelIndex);
    virtual bool init() override;
    // 重写 update 方法
    virtual void update(float dt) override;
    void loadLevel(int levelIndex);
    virtual void onExit() override;
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);

    CREATE_FUNC(BattleScene);
};

#endif // __BATTLE_SCENE_H__