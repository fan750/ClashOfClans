#ifndef __BATTLE_SCENE_H__
#define __BATTLE_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h" // 必须引入 GUI 头文件
#include "Troop.h" // 必须认识 TroopType

class BattleScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    virtual void onExit() override;
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);

    CREATE_FUNC(BattleScene);

private:
    // 【新增】初始化兵种选择UI的辅助函数
    void createSelectButton(const std::string& title, cocos2d::Color3B color, TroopType type, int index);

private:
    // 【新增】当前选中的兵种类型
    TroopType m_selectedType;

    // 【新增】显示当前状态的文字
    cocos2d::Label* m_infoLabel;
};

#endif // __BATTLE_SCENE_H__