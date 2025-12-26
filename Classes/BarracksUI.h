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
    void updateBarrackInfo();        // 更新军营信息显示
    void showUpgradeFailureDialog(class Building* barracks);  // 显示升级失败对话框
    void showNoBarracksDialog();      // 显示无军营对话框

    cocos2d::ui::Layout* m_mainPanel; // 主面板
    cocos2d::Label* m_barrackLevelLabel;    // 军营等级标签
    cocos2d::Label* m_costLimitLabel;       // Cost上限标签
    cocos2d::Label* m_currentCostLabel;     // 当前Cost使用标签
public:
    virtual bool init() override;
    CREATE_FUNC(BarracksUI);

    void show(); // 显示军营UI
    void hide(); // 隐藏军营UI
    void updateUpgradeButton();
    void onEnter();
};

#endif