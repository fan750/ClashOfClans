#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Building.h" 
#include "BarracksUI.h"

// 商店物品结构体
struct ShopItem
{
    std::string name;           // 商品名称
    BuildingType type;          // 建筑类型
    int price;                  // 价格
    bool isGold;                // 是否使用金币购买
    std::string iconPath;       // 图标文件路径
};

class MainMode : public cocos2d::Scene
{
protected:
    // UI 面板成员变量
    cocos2d::ui::Layout* m_shopLayer;              // 商店主面板
    cocos2d::ui::Layout* m_timeLayer;              // 加速主面板
    cocos2d::Node* m_confirmLayer;                 // 存放“确定/取消”按钮的节点
    BarracksUI* m_barracksUI;                      // 军营UI实例

    // 建筑相关成员变量
    Building* m_pendingBuilding;                        // 当前正在放置的“虚影”建筑
    Building* m_selectedBuilding;                       // 当前选中的建筑
    Building* getBarracksAtPosition(cocos2d::Vec2 pos); // 获取指定位置的军营

    // 放置逻辑成员变量
    int m_pendingCost;                             // 记录正在放置的建筑价格
    bool m_pendingIsGold;                          // 是否使用金币
    bool m_isConfirming;                           // 标记是否正在等待确认放置

    // 按钮成员变量
    cocos2d::ui::Button* m_barracksShopButton;     // 用于在建造后移除商店按钮
    cocos2d::ui::Button* TimeBtn;                  // 加速按钮
    cocos2d::ui::Button* m_accelerateBtn;          // 加速执行按钮

    // 游戏世界成员变量
    cocos2d::Node* m_gameLayer;                    // 游戏世界容器层（背景和建筑均加在此）
    cocos2d::Size m_mapSize;                       // 地图的总大小
    cocos2d::Vec2 m_lastTouchPos;                  // 记录手指上一次触摸的位置

public:
    // 场景生命周期
    static cocos2d::Scene* createScene();          // 创建场景
    virtual bool init() override;                  // 初始化

    // 触摸事件回调
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event); // 触摸开始
    void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event); // 触摸移动（让建筑跟随手指）

    // 菜单回调
    void menuCloseCallback(cocos2d::Ref* pSender); // 关闭按钮回调

    // UI 初始化与控制
    void initShopUI();                             // 初始化商店UI
    void toggleShop();                             // 显示/隐藏商店
    void initTime();                               // 初始化加速面板
    void toggleTime();                             // 显示/隐藏加速面板
    void updateTimeButtonCooldown();               // 更新加速按钮状态

    // 建筑购买与放置
    void tryBuyBuilding(const ShopItem& item);     // 尝试购买建筑
    void selectBuilding(Building* building);       // 选中建筑处理
    void showConfirmationUI(cocos2d::Vec2 pos);    // 显示确认菜单
    void onConfirmPlacement();                     // 点击“确定”后的逻辑
    void onCancelPlacement();                      // 点击“取消”后的逻辑
    void createShopItemButton(const ShopItem& item, cocos2d::Vec2 pos, float iconScale = 0.8f); // 创建商店物品按钮

    // 辅助功能
    void playCollectAnimation(int amount, cocos2d::Vec2 startPos, BuildingType type); // 播放收集资源动画
    Building* getBarracksBuilding() const;         // 获取军营建筑

    CREATE_FUNC(MainMode);
};

#endif