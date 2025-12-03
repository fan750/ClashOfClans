#include "BattleScene.h"
#include "HelloWorldScene.h" // 为了能回城
#include "BattleManager.h"
#include "GameManager.h" // 如果想在战斗里显示资源
#include "Building.h"
#include "Troop.h"
#include "ui/CocosGUI.h"
USING_NS_CC;

Scene* BattleScene::createScene() {
    return BattleScene::create();
}

bool BattleScene::init() {
    if (!Scene::init()) return false;

    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 【重要】进战斗前，先清空 BattleManager 的旧数据
    BattleManager::getInstance()->clear();

    // 2. 布置敌人的阵地 (Hardcode 一个关卡)
    // 比如：中间放个大本营，周围放墙和炮
    auto enemyTown = Building::create(BuildingType::TOWN_HALL);
    enemyTown->setPosition(visibleSize / 2);
    this->addChild(enemyTown);

    auto enemyCannon = Building::create(BuildingType::CANNON);
    // 直接把偏移量加在坐标里
    enemyCannon->setPosition(Vec2(visibleSize.width / 2 + 100, visibleSize.height / 2));
    this->addChild(enemyCannon);

    // 3. 添加一个“撤退”按钮 (回城)
    auto backBtn = ui::Button::create("CloseNormal.png");
    backBtn->setTitleText("Retreat");
    backBtn->setPosition(Vec2(100, visibleSize.height - 50));
    backBtn->addClickEventListener([=](Ref* sender) {
        // 切换回家园场景
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, HelloWorld::createScene()));
        });
    this->addChild(backBtn);

    // 4. 设置触摸监听 (点击屏幕放兵)
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(BattleScene::onTouchBegan, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

bool BattleScene::onTouchBegan(Touch* touch, Event* event) {
    // 获取点击位置
    Vec2 touchLoc = touch->getLocation();

    // 在点击处生成一个野蛮人 (或者你可以做个UI选兵种)
    auto troop = Troop::create(TroopType::BARBARIAN);
    troop->setPosition(touchLoc);
    this->addChild(troop);

    return true;
}

void BattleScene::onExit() {
    Scene::onExit();
    // 离开战斗时，再次清理，防止把敌人的建筑带回家
    BattleManager::getInstance()->clear();
}