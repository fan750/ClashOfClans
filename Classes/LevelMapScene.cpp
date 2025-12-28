#include "LevelMapScene.h"
#include "MainModeScene.h" // 为了能返回主菜单
#include "BattleScene.h"   // 为了进入战斗
#include <algorithm>
USING_NS_CC;

// 创建关卡选择场景
Scene* LevelMapScene::createScene()
{
    return LevelMapScene::create();
}

// 初始化场景
bool LevelMapScene::init()
{
    if (!Scene::init())
    {
        return false;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 预创建关卡按钮对象并暂存
    auto level1 = ui::Button::create("king1.png");
    auto level2 = ui::Button::create("king2.png");
    auto level3 = ui::Button::create("king3.png");
    level.push_back(level1);
    level.push_back(level2);
    level.push_back(level3);

    // 加载背景图
    auto bg = Sprite::create("option_background.jpg");
    if (bg)
    {
        bg->setPosition(visibleSize / 2);
        // 简单适配铺满
        float scaleX = visibleSize.width / bg->getContentSize().width;
        float scaleY = visibleSize.height / bg->getContentSize().height;
        bg->setScale(std::max(scaleX, scaleY));
        this->addChild(bg, -1);
    }

    // 创建标题
    auto title = Label::createWithSystemFont(u8"在这片富饶的土地上，有三个穷凶极恶的国王，分别是"
        "\n精通计算机组成原理的计组大王\n熟练掌握各种数据结构的数据结构大王"
        "\n专攻科学王冠的离散数学大王\n你敢直视他们吗?", "Microsoft YaHei", 28);
    title->setPosition(Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.75f));
    title->setTextColor(Color4B::WHITE);
    title->setAlignment(TextHAlignment::CENTER);
    title->setMaxLineWidth(static_cast<int>(visibleSize.width * 0.8f));
    this->addChild(title);

    // 创建关卡按钮
    // 关卡 1
    createLevelButton(1, Vec2(visibleSize.width * 0.3, visibleSize.height / 3));
    // 关卡 2
    createLevelButton(2, Vec2(visibleSize.width * 0.5, visibleSize.height / 3));
    // 关卡 3
    createLevelButton(3, Vec2(visibleSize.width * 0.7, visibleSize.height / 3));

    // 创建返回主菜单按钮
    auto backBtn = ui::Button::create("back.png");
    backBtn->setScale(0.3f);
    backBtn->setPosition(Vec2(50, visibleSize.height - 50));
    backBtn->addClickEventListener([=](Ref*)
        {
            Director::getInstance()->replaceScene(TransitionFade::create(0.5f, MainMode::createScene()));
        });
    this->addChild(backBtn);

    return true;
}

// 辅助函数：创建关卡按钮
void LevelMapScene::createLevelButton(int levelIndex, Vec2 pos)
{
    int idx = levelIndex - 1;
    if (idx < 0 || idx >= static_cast<int>(level.size()))
    {
        return;
    }

    auto btn = level[idx];
    if (!btn)
    {
        return;
    }

    // 设置按钮属性与位置
    btn->setScale(0.5f);
    btn->setPosition(pos);

    // 设置点击逻辑
    btn->addClickEventListener([=](Ref*)
        {
            // 跳转到战斗场景，并告诉它我要打第几关
            auto scene = BattleScene::createScene(levelIndex);
            Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        });

    this->addChild(btn);
}