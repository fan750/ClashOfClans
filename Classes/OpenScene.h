#ifndef __LOAD_SCENE_H__
#define __LOAD_SCENE_H__

#include "cocos2d.h"

class Load : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();              // 创建场景
    virtual bool init();                               // 初始化场景
    void menuCloseCallback(cocos2d::Ref* pSender);     // 关闭菜单回调函数
    void onAssaultMenuCallback(cocos2d::Ref* pSender); // 进攻菜单回调函数
    void enterMainMode(float dt);                      // 定时器回调：进入主场景

    CREATE_FUNC(Load);
};

#endif