#ifndef __BATTLE_SCENE_H__
#define __BATTLE_SCENE_H__

#include "cocos2d.h"

class BattleScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;

    // 退出战斗时清理垃圾
    virtual void onExit() override;

    // 触摸回调：用来放兵
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);

    CREATE_FUNC(BattleScene);
};

#endif // __BATTLE_SCENE_H__