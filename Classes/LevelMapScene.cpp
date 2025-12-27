//LevelMapScene.cpp
#include "LevelMapScene.h"
#include "MainModeScene.h" // 为了能返回主菜单
#include "BattleScene.h"     // 为了进入战斗
#include <algorithm>
USING_NS_CC;

Scene* LevelMapScene::createScene() {
    return LevelMapScene::create();
}

bool LevelMapScene::init() {
    if (!Scene::init()) return false;

    Size visibleSize = Director::getInstance()->getVisibleSize();
    auto level1 = ui::Button::create("king1.png");
    auto level2 = ui::Button::create("king2.png");
    auto level3 = ui::Button::create("king3.png");
    level.push_back(level1);
    level.push_back(level2);
    level.push_back(level3);
    // 1. 背景 (可以用你的 Background.jpg)
    auto bg = Sprite::create("option_background.jpg");
    if (bg) {
        bg->setPosition(visibleSize / 2);
        // 简单适配铺满
        float scaleX = visibleSize.width / bg->getContentSize().width;
        float scaleY = visibleSize.height / bg->getContentSize().height;
        bg->setScale(std::max(scaleX, scaleY));
        this->addChild(bg, -1);
    }

    // 2. 标题
    auto title = Label::createWithSystemFont("In this fertile land, there are three fearsome kings:"
        "\nthe Computer Architecture King, who masters computer organization principles\nthe Data Structure King, skilled in all kinds of data structures"
        "\nand the Discrete Math King, who specializes in the crown jewel of science.\nDo you dare to face them", "Arial", 28);
    title->setPosition(Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.75f));
    title->setTextColor(Color4B::WHITE);
    title->setAlignment(TextHAlignment::CENTER);
    title->setMaxLineWidth(static_cast<int>(visibleSize.width * 0.8f));
    this->addChild(title);

    // 3. 创建关卡按钮
    // 关卡 1
    createLevelButton(1, Vec2(visibleSize.width * 0.3, visibleSize.height / 3));
    // 关卡 2
    createLevelButton(2, Vec2(visibleSize.width * 0.5, visibleSize.height / 3));
    // 关卡 3
    createLevelButton(3, Vec2(visibleSize.width * 0.7, visibleSize.height / 3));

    // 4. 返回主菜单按钮
    auto backBtn = ui::Button::create("back.png");
    backBtn->setScale(0.3f);
    backBtn->setPosition(Vec2(50, visibleSize.height - 50));
    backBtn->addClickEventListener([=](Ref*) {
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, MainMode::createScene()));
        });
    this->addChild(backBtn);

    return true;
}

void LevelMapScene::createLevelButton(int levelIndex, Vec2 pos) {
    int idx = levelIndex - 1;
    if (idx < 0 || idx >= static_cast<int>(level.size())) return;

    auto btn = level[idx];
    if (!btn) return;
    btn->setScale(0.5f);
    btn->setPosition(pos);

    // 点击逻辑
    btn->addClickEventListener([=](Ref*) {
        // 【关键】跳转到战斗场景，并告诉它我要打第几关
        auto scene = BattleScene::createScene(levelIndex);
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        });

    this->addChild(btn);
}