#ifndef __BATTLE_SCENE_H__
#define __BATTLE_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Troop.h"
#include "Building.h" // 确保包含
#include <vector>

class BattleScene : public cocos2d::Scene
{
private:
    // UI 辅助函数
    void createSelectButton(const std::string& title, cocos2d::Color3B color, TroopType type, int index); // 创建兵种选择按钮
    void initTroopCountLabels();                                                                          // 初始化兵种数量显示标签
    void updateTroopCountLabels();                                                                        // 更新兵种数量显示标签

    // 战斗结束处理函数
    void onBattleEnd();        // 战斗结束逻辑
    void checkBattleEnd();     // 胜负判定函数

    // 状态成员变量
    int m_levelIndex;                                        // 记录当前关卡索引，用于结算奖励
    TroopType m_selectedType;                                // 当前选中的兵种类型
    cocos2d::Label* m_infoLabel;                             // 顶部信息提示标签
    std::map<TroopType, cocos2d::Label*> m_troopCountLabels; // 兵种数量显示标签映射
    cocos2d::DrawNode* m_deployAreaVisual;                   // 投放区域可视化节点（如禁放区红框）

public:
    static cocos2d::Scene* createScene(int levelIndex);              // 创建战斗场景（带关卡索引）
    virtual bool init() override;                                    // 初始化场景
    virtual void update(float dt) override;                          // 每帧更新
    void loadLevel(int levelIndex);                                  // 加载指定关卡数据
    virtual void onExit() override;                                  // 场景退出时调用
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event); // 触摸事件回调

    CREATE_FUNC(BattleScene);
};

#endif // __BATTLE_SCENE_H__