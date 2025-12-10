//LevelMapScene.h
#ifndef __LEVEL_MAP_SCENE_H__
#define __LEVEL_MAP_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <vector>

class LevelMapScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;

    // 辅助函数：创建关卡按钮
    void createLevelButton(int levelIndex, cocos2d::Vec2 pos);

    CREATE_FUNC(LevelMapScene);
private:
    std::vector<cocos2d::ui::Button*> level;
};

#endif // __LEVEL_MAP_SCENE_H__