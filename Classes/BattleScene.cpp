#include "BattleScene.h"
#include "HelloWorldScene.h"
#include "BattleManager.h"
#include "Building.h"
#include "Troop.h"

USING_NS_CC;
using namespace ui; // 使用 UI 命名空间

Scene* BattleScene::createScene() {
    return BattleScene::create();
}

bool BattleScene::init() {
    if (!Scene::init()) return false;

    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 初始化数据
    BattleManager::getInstance()->clear();
    m_selectedType = TroopType::BARBARIAN; // 默认选野蛮人

    // 2. 布置敌人阵地 (保持不变)
    auto enemyTown = Building::create(BuildingType::TOWN_HALL);
    enemyTown->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    this->addChild(enemyTown);

    // 加几个墙给炸弹人炸
    for (int i = 0; i < 5; i++) {
        auto wall = Building::create(BuildingType::WALL);
        wall->setPosition(Vec2(visibleSize.width / 2 - 100 + i * 40, visibleSize.height / 2 - 100));
        this->addChild(wall);
    }

    // 3. 【新增】创建提示文字
    m_infoLabel = Label::createWithSystemFont("Selected: Barbarian", "Arial", 24);
    m_infoLabel->setPosition(Vec2(visibleSize.width / 2, 50)); // 放在底部中间
    this->addChild(m_infoLabel, 10);

    // 4. 【新增】创建一排选择按钮
    // 参数：名字，颜色，类型，第几个(用于排版)
    createSelectButton("Barb", Color3B::GREEN, TroopType::BARBARIAN, 0);
    createSelectButton("Arch", Color3B::MAGENTA, TroopType::ARCHER, 1);
    createSelectButton("Giant", Color3B::ORANGE, TroopType::GIANT, 2);
    createSelectButton("Bomb", Color3B::GRAY, TroopType::BOMBERMAN, 3); // 白色按钮文字看不清，用灰色代替背景

    // 5. 撤退按钮 (保持不变)
    auto backBtn = Button::create("CloseNormal.png");
    backBtn->setTitleText("Back");
    backBtn->setPosition(Vec2(50, visibleSize.height - 50));
    backBtn->addClickEventListener([=](Ref*) {
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, HelloWorld::createScene()));
        });
    this->addChild(backBtn);

    // 6. 触摸监听
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(BattleScene::onTouchBegan, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

// 【新增】辅助函数：快速创建按钮
void BattleScene::createSelectButton(const std::string& title, Color3B color, TroopType type, int index) {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 创建按钮
    auto btn = Button::create("CloseNormal.png");
    btn->setColor(color); // 染成兵种的颜色
    btn->setTitleText(title);
    btn->setTitleFontSize(20);

    // 排列在右下角
    // index * 60 意味着每个按钮间隔 60 像素
    btn->setPosition(Vec2(visibleSize.width - 250 + index * 60, 50));

    // 点击逻辑
    btn->addClickEventListener([=](Ref*) {
        // 1. 更新选中的类型
        m_selectedType = type;

        // 2. 更新文字提示
        m_infoLabel->setString("Selected: " + title);
        m_infoLabel->setColor(color);
        });

    this->addChild(btn, 10); // ZOrder设为10，防止被兵种遮挡
}

bool BattleScene::onTouchBegan(Touch* touch, Event* event) {
    Vec2 touchLoc = touch->getLocation();

    // 如果点击到了按钮区域（屏幕下方 80 像素内），就不放兵，防止误触
    if (touchLoc.y < 80) return false;

    // 【修改】使用当前选中的类型 (m_selectedType)
    auto troop = Troop::create(m_selectedType);
    troop->setPosition(touchLoc);
    this->addChild(troop);

    return true;
}

void BattleScene::onExit() {
    Scene::onExit();
    BattleManager::getInstance()->clear();
}