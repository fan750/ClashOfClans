#ifndef _UPGRADE_UI_
#define _UPGRADE_UI_

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Troop.h"
#include <map>
#include <string>

// 兵种升级信息结构体
struct UpgradeTroops
{
    std::string name;                              // 兵种名称
    int upgrade_cost;                              // 升级花费
    TroopType type;                                // 兵种类型
    std::string iconpath;                          // 图标路径
};

class UpgradeUi : public cocos2d::Layer
{
private:
    // 私有成员函数
    void refreshUpgradeCosts();                      // 刷新升级费用显示
    int calculateUpgradeCost(TroopType type) const;  // 计算特定兵种的升级费用

    // UI 组件成员变量
    cocos2d::ui::Layout* m_mainPanel;                   // 主面板
    std::map<TroopType, cocos2d::Label*> m_priceLabels; // 费用标签映射
    std::map<TroopType, cocos2d::Label*> m_levelLabels; // 等级标签映射

public:
    // 生命周期函数
    virtual bool init() override;                   // 初始化
    CREATE_FUNC(UpgradeUi);                         // 创建宏

    // 交互逻辑函数
    void show();                                    // 显示升级兵种UI
    void hide();                                    // 隐藏UI
    void onTroopUpgraded();                         // 兵种升级回调

    // UI 创建函数
    void createUpgradeButton(const UpgradeTroops& troops, cocos2d::Vec2 pos, cocos2d::Node* parentnode); // 创建升级按钮
};

#endif // _UPGRADE_UI_