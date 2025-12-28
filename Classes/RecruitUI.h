#ifndef __RECRUIT_UI_H__
#define __RECRUIT_UI_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Troop.h" // 需要兵种类型

// 用于存储招募项信息的结构体
struct RecruitItem
{
    std::string name;                             // 兵种名称
    TroopType type;                               // 兵种类型
    int cost;                                     // 招募花费（圣水）
    int troopCost;                                // 兵种cost值
    bool isGoldCost;                              // true=花费金币, false=花费圣水
    std::string iconPath;                         // 图标路径
};

class RecruitUI : public cocos2d::Layer
{
private:
    // UI 组件成员变量
    cocos2d::ui::Layout* m_mainPanel;            // 主面板
    cocos2d::Label* m_costLimitLabel;            // Cost上限显示
    cocos2d::Label* m_currentCostLabel;          // 当前Cost使用显示

    // 私有成员函数
    void initUI();                                                           // 初始化UI元素
    void updateCostDisplay();                                                // 更新cost显示
    bool canRecruitTroop(TroopType type);                                    // 检查是否可以招募
    void updateRecruitButtonState(cocos2d::ui::Button* btn, TroopType type); // 更新按钮状态

public:
    // 生命周期函数
    virtual bool init() override;                 // 初始化
    virtual void onExit() override;               // 重写 onExit 以进行安全清理
    void onEnter();                               // 进入场景回调
    CREATE_FUNC(RecruitUI);                       // 创建宏

    // 交互逻辑函数
    void show();                                  // 显示招募UI
    void hide();                                  // 隐藏招募UI
    void createRecruitItemButton(const RecruitItem& item, cocos2d::Vec2 pos, cocos2d::Node* parentNode); // 创建招募项按钮
    void showCannotRecruitDialog(const RecruitItem& item);                                               // 显示无法招募对话框
    void showNotEnoughResourcesDialog(const RecruitItem& item);                                          // 显示资源不足对话框
};

#endif // __RECRUIT_UI_H__