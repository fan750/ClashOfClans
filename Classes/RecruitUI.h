//RecruitUI.h
#ifndef __RECRUIT_UI_H__
#define __RECRUIT_UI_H__
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Troop.h" // 需要兵种类型

// 用于存储招募项信息的结构体
struct RecruitItem 
{
    std::string name;
    TroopType type;
    int cost;            // 招募花费（圣水）
    int troopCost;       // 【新增】兵种cost值
    bool isGoldCost;     // true=花费金币, false=花费圣水
    std::string iconPath;
};

class RecruitUI : public cocos2d::Layer
{
private:
    void initUI();                    // 初始化UI元素
    cocos2d::ui::Layout* m_mainPanel; // 主面板
    // 【新增】UI元素
    cocos2d::Label* m_costLimitLabel;    // Cost上限显示
    cocos2d::Label* m_currentCostLabel;  // 当前Cost使用显示

    // 【新增】辅助方法
    void updateCostDisplay();            // 更新cost显示
    bool canRecruitTroop(TroopType type); // 检查是否可以招募
    void updateRecruitButtonState(cocos2d::ui::Button* btn, TroopType type); // 更新按钮状态

public:
    virtual bool init() override;
    CREATE_FUNC(RecruitUI);

    void show();                      // 显示招募UI
    void hide();                      // 隐藏招募UI
    virtual void onExit() override;   // 重写 onExit 以进行安全清理
    void createRecruitItemButton(const RecruitItem& item, cocos2d::Vec2 pos, Node* parentNode);
    void showCannotRecruitDialog(const RecruitItem& item);
    void showNotEnoughResourcesDialog(const RecruitItem& item);
    void onEnter();
};

#endif // __RECRUIT_UI_H__
//RecruitUI.h