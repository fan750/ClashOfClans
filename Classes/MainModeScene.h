//MainModeScene.h
#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Building.h" 
#include "BarracksUI.h"

struct ShopItem {
    std::string name;
    BuildingType type;
    int price;
    bool isGold; // true=金币买, false=圣水买
    std::string iconPath; // 【新增】存放图标文件名的变量
};

class MainMode : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init() override;

    // 触摸回调
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event); // 让建筑跟随手指移动

    // 关闭按钮回调
    void menuCloseCallback(cocos2d::Ref* pSender);
    void playCollectAnimation(int amount, cocos2d::Vec2 startPos, BuildingType type);
    // CREATE_FUNC 宏只能出现一次！它会自动生成 create() 函数
    CREATE_FUNC(MainMode);
    // 【新增】获取军营建筑
    Building* getBarracksBuilding() const;
    // 初始化商店UI
    void initShopUI();
    // 显示/隐藏商店
    void toggleShop();
    //初始化加速面板
    void initTime();
    //显示/隐藏加速面板
    void toggleTime();
    // 更新加速按钮状态
    void updateTimeButtonCooldown();

    // 尝试购买建筑
    void tryBuyBuilding(const ShopItem& item);

    // 选中建筑处理
    void selectBuilding(Building* building);

    // 商店的主面板
    cocos2d::ui::Layout* m_shopLayer;
    //加速的主面板
    cocos2d::ui::Layout* m_timeLayer;
    // 【关键】当前正在放置的“虚影”建筑
    // 如果这个指针不为空，说明玩家正在选位置
    Building* m_pendingBuilding;

    // 当前选中的建筑（用于显示升级按钮等）
    Building* m_selectedBuilding;

    // 记录正在放置的建筑要多少钱 (放置成功才扣费)
    int m_pendingCost;
    bool m_pendingIsGold;

    // 【新增】标记：是否正在等待确认放置
    bool m_isConfirming;

    // 【新增】存放“确定/取消”按钮的节点（方便统一删除）
    cocos2d::Node* m_confirmLayer;

    // 【新增】显示确认菜单
    void showConfirmationUI(cocos2d::Vec2 pos);

    // 【新增】点击“确定”后的逻辑
    void onConfirmPlacement();

    // 【新增】点击“取消”后的逻辑
    void onCancelPlacement();

    Building* getBarracksAtPosition(cocos2d::Vec2 pos); // 用于处理点击军营的逻辑
    cocos2d::ui::Button* m_barracksShopButton;          // 用于在建造后移除商店按钮
    BarracksUI* m_barracksUI;                           // 军营UI实例
    cocos2d::ui::Button* TimeBtn;
    cocos2d::ui::Button* m_accelerateBtn;
    void createShopItemButton(const ShopItem& item, cocos2d::Vec2 pos, float iconScale = 0.8f);

    // 【核心】游戏世界容器层
    // 背景图和所有建筑都加到这个层上，拖拽时只移动这个层
    cocos2d::Node* m_gameLayer;

    // 地图的总大小 (背景图的真实尺寸)
    cocos2d::Size m_mapSize;

    // 记录手指上一次触摸的位置
    cocos2d::Vec2 m_lastTouchPos;
};

#endif