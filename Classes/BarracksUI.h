#ifndef __BARRACKS_UI_H__
#define __BARRACKS_UI_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

class RecruitUI;
class BarracksUI : public cocos2d::Layer
{
private:
    // 私有成员函数
    void initButtons();                                      // 初始化按钮
    void updateBarrackInfo();                                // 更新军营信息显示
    void showUpgradeFailureDialog(class Building* barracks); // 显示升级失败对话框
    void showNoBarracksDialog();                             // 显示无军营对话框

    // UI 组件成员变量
    cocos2d::ui::Layout* m_mainPanel;                       // 主面板
    cocos2d::Label* m_barrackLevelLabel;                    // 军营等级标签
    cocos2d::Label* m_costLimitLabel;                       // Cost上限标签
    cocos2d::Label* m_currentCostLabel;                     // 当前Cost使用标签

public:
    // 生命周期函数
    virtual bool init() override;                           // 初始化
    void onEnter();                                         // 进入场景回调
    CREATE_FUNC(BarracksUI);                                // 创建宏

    // 交互逻辑函数
    void show();                                            // 显示军营UI
    void hide();                                            // 隐藏军营UI
    void updateUpgradeButton();                             // 更新升级按钮状态
};

#endif // __BARRACKS_UI_H__