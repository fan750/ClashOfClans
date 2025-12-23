//OpenScene.h
#ifndef __LOAD_SCENE_H__
#define __LOAD_SCENE_H__

#include "cocos2d.h"

class Load : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();

    // 你的回调函数
    void menuCloseCallback(cocos2d::Ref* pSender);
    void onAssaultMenuCallback(cocos2d::Ref* pSender);

    // 【新增】用于定时器调用的函数
    void enterMainMode(float dt);

    CREATE_FUNC(Load);
};

#endif