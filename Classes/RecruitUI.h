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
    int cost;
    bool isGoldCost; // true=花费金币, false=花费圣水
    std::string iconPath;
};

class RecruitUI : public cocos2d::Layer
{
private:
    void initUI();                    // 初始化UI元素
    cocos2d::ui::Layout* m_mainPanel; // 主面板
public:
    virtual bool init() override;
    CREATE_FUNC(RecruitUI);

    void show();                      // 显示招募UI
    void hide();                      // 隐藏招募UI
    virtual void onExit() override;   // 重写 onExit 以进行安全清理
    void createRecruitItemButton(const RecruitItem& item, cocos2d::Vec2 pos, Node* parentNode);
};

#endif // __RECRUIT_UI_H__
//RecruitUI.h