#ifndef __GAME_UI_H__
#define __GAME_UI_H__

#include "cocos2d.h"
#include "Troop.h"

class GameUI : public cocos2d::Layer
{
private:
    // UI 组件成员变量
    cocos2d::Label* m_goldLabel;                         // 金币标签
    cocos2d::Label* m_elixirLabel;                       // 圣水标签
    std::map<TroopType, cocos2d::Label*> m_troopLabels;  // 兵种标签
    std::map<TroopType, cocos2d::Label*> m_costLabels;   // 兵种cost标签
    cocos2d::ui::Layout* m_armyPanel;                    // 军队详情弹窗面板

    // 私有成员函数
    void showArmyPanel();                                 // 打开军队界面
    void hideArmyPanel();                                 // 关闭军队界面
    void updateArmyLabels();                              // 刷新兵种数量显示

public:
    // 生命周期函数
    virtual bool init() override;                         // 初始化
    void onEnter();                                       // 进入场景回调
    CREATE_FUNC(GameUI);                                  // 创建宏

    // 初始化函数
    void initResourceLabels();                            // 初始化资源标签
    void initTroopLabels();                               // 初始化兵种标签

    // 更新函数
    void updateLabels();                                  // 刷新显示的文字
    void updateCostDisplay();                             // 更新Cost显示
};

#endif // __GAME_UI_H__