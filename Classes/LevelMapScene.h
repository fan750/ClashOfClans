#ifndef __LEVEL_MAP_SCENE_H__
#define __LEVEL_MAP_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <vector>

class LevelMapScene : public cocos2d::Scene
{
private:
    // 成员变量
    std::vector<cocos2d::ui::Button*> level; // 关卡按钮列表

public:
    // 场景创建与初始化
    static cocos2d::Scene* createScene();   // 创建场景
    virtual bool init() override;           // 初始化场景

    // UI 辅助函数
    void createLevelButton(int levelIndex, cocos2d::Vec2 pos); // 创建关卡按钮

    CREATE_FUNC(LevelMapScene);
};

#endif // __LEVEL_MAP_SCENE_H__