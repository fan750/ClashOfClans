// BarracksUI.h
#ifndef __BARRACKS_UI_H__
#define __BARRACKS_UI_H__
#include "cocos2d.h"
#include "ui/CocosGUI.h"

class RecruitUI;
class BarracksUI : public cocos2d::Layer
{
private:
    void initButtons();               // 初始化按钮
    cocos2d::ui::Layout* m_mainPanel; // 主面板
public:
    virtual bool init() override;
    CREATE_FUNC(BarracksUI);

    void show(); // 显示军营UI
    void hide(); // 隐藏军营UI
};

#endif