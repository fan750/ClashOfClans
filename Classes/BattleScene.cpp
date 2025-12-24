 //BattleScene.cpp
#include "BattleScene.h"
#include "MainModeScene.h"
#include "BattleManager.h"
#include "Building.h"
#include "Troop.h"
#include "GameManager.h"
#include "ui/CocosGUI.h"
USING_NS_CC;
using namespace ui; // 使用 UI 命名空间

Scene* BattleScene::createScene(int levelIndex) {
    // 1. 先创建场景对象 (内部调用 init)
    auto scene = BattleScene::create();

    // 2. 如果创建成功，加载对应关卡的数据
    // dynamic_cast 用于安全转换，确保 scene 是 BattleScene 类型
    if (auto battleScene = dynamic_cast<BattleScene*>(scene)) {
        battleScene->loadLevel(levelIndex);
    }

    return scene;
}

bool BattleScene::init()
{
    if (!Scene::init()) return false;

    Size visibleSize = Director::getInstance()->getVisibleSize();

    auto background = Sprite::create("battleback1.png");

    if (background)
    {
        // 2. 放在屏幕正中间
        background->setPosition(visibleSize / 2);

        // 3. 【关键】自动缩放以铺满屏幕
        // 获取图片原始大小
        Size bgSize = background->getContentSize();

        // 计算需要的缩放倍数
        float scaleX = visibleSize.width / bgSize.width;
        float scaleY = visibleSize.height / bgSize.height;

        // 取较大的那个缩放值，确保不留黑边 (Cover 模式)
        float finalScale = std::max(scaleX, scaleY);
        background->setScale(finalScale);

        // 4. 放在最底层 (ZOrder = -99)
        this->addChild(background, -99);
    }
    else {
        CCLOG("Error: Background image not found!");
    }

    // 添加投放区域可视化 - 中央禁放区域
    m_deployAreaVisual = DrawNode::create();

    // 计算中央禁放区域
    float leftEdge = visibleSize.width * 0.2f;
    float rightEdge = visibleSize.width * 0.8f;
    float bottomEdge = visibleSize.height * 0.2f;
    float topEdge = visibleSize.height * 0.8f;

    // 绘制半透明红色矩形表示禁放区域
    Vec2 vertices[] = {
        Vec2(leftEdge, bottomEdge),
        Vec2(rightEdge, bottomEdge),
        Vec2(rightEdge, topEdge),
        Vec2(leftEdge, topEdge)
    };

    // 使用红色表示禁放区域
    m_deployAreaVisual->drawPolygon(vertices, 4, Color4F(1.0f, 0.0f, 0.0f, 0.2f), 2, Color4F(1.0f, 0.0f, 0.0f, 0.5f));
    this->addChild(m_deployAreaVisual, -1); // 放在最底层

    // 1. 初始化数据
    BattleManager::getInstance()->clear();
    m_selectedType = TroopType::BARBARIAN; // 默认选野蛮人

    // 从GameManager获取兵种数量并初始化到BattleManager
    auto gm = GameManager::getInstance();
    std::map<TroopType, int> availableTroops;
    availableTroops[TroopType::BARBARIAN] = gm->getTroopCount(TroopType::BARBARIAN);
    availableTroops[TroopType::ARCHER] = gm->getTroopCount(TroopType::ARCHER);
    availableTroops[TroopType::GIANT] = gm->getTroopCount(TroopType::GIANT);
    availableTroops[TroopType::BOMBERMAN] = gm->getTroopCount(TroopType::BOMBERMAN);
    availableTroops[TroopType::DRAGON] = gm->getTroopCount(TroopType::DRAGON);
    BattleManager::getInstance()->initAvailableTroops(availableTroops);

    // 2. 布置敌人阵地 (保持不变)
    auto enemyTown = Building::create(BuildingType::TOWN_HALL);
    enemyTown->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    this->addChild(enemyTown);


    // 3. 【新增】创建提示文字
    m_infoLabel = Label::createWithSystemFont("Selected: Barbarian", "Arial", 24);
    m_infoLabel->setPosition(Vec2(visibleSize.width / 2, 50)); // 放在底部中间
    this->addChild(m_infoLabel, 10);

    // 4. 【新增】创建一排选择按钮
    // 参数：名字，颜色，类型，第几个(用于排版)
    createSelectButton("Barb", Color3B::GREEN, TroopType::BARBARIAN, 0);
    createSelectButton("Arch", Color3B::MAGENTA, TroopType::ARCHER, 1);
    createSelectButton("Giant", Color3B::ORANGE, TroopType::GIANT, 2);
    createSelectButton("Bomb", Color3B::GRAY, TroopType::BOMBERMAN, 3);
    createSelectButton("Dragon", Color3B::RED, TroopType::DRAGON, 4);

    // 初始化兵种数量显示
    initTroopCountLabels();
    updateTroopCountLabels();

    // 5. 撤退按钮 (保持不变)
    auto backBtn = Button::create("back.png");
    backBtn->setScale(0.3f);
    backBtn->setPosition(Vec2(50, visibleSize.height - 50));
    backBtn->addClickEventListener
    (
        [=](Ref*)
        {
            // 战斗结束同步逻辑
            this->onBattleEnd();
            Director::getInstance()->replaceScene(TransitionFade::create(0.5f, MainMode::createScene()));
        }
    );
    this->addChild(backBtn);

    // 6. 触摸监听
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(BattleScene::onTouchBegan, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 开启每帧更新
    this->scheduleUpdate();
    return true;
}

void BattleScene::update(float dt)
{
    // 每帧都调用胜负检查
    checkBattleEnd();
}

void BattleScene::checkBattleEnd()
{
    // 1. 检查胜利条件：敌方大本营是否被摧毁
    bool townHallDestroyed = true;
    for (auto building : BattleManager::getInstance()->getBuildings())
    {
        if (building->getBuildingType() == BuildingType::TOWN_HALL && !building->isDead())
        {
            townHallDestroyed = false;
            break;
        }
    }

    if (townHallDestroyed)
    {
        CCLOG("VICTORY!");
        this->unscheduleUpdate(); // 停止检查

        // 显示胜利提示
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto winSprite = Sprite::create("victory.png"); // 确保文件名正确
        if (winSprite)
        {
            winSprite->setPosition(visibleSize / 2);
            winSprite->setScale(0.0f); // 初始设为0，为了后面的弹出动画
            this->addChild(winSprite, 1000);

            // 添加一个 "Q弹" 的弹出动画效果
            // 0.3秒放大到1.2倍，然后0.1秒恢复到1.0倍
            winSprite->runAction(Sequence::create(
                ScaleTo::create(0.3f, 0.8f),
                ScaleTo::create(0.1f, 0.6f),
                nullptr
            ));
        }
        else
        {
            CCLOG("Error: victory.png not found!");
        }

        // 2秒后返回主界面
        this->scheduleOnce([=](float dt) {
            this->onBattleEnd(); // 同步死亡兵数
            Director::getInstance()->replaceScene(TransitionFade::create(1.0f, MainMode::createScene()));
            }, 2.0f, "ReturnToMain");
        return;
    }

    // 2. 检查失败条件
    // 条件：无可部署士兵 && 场上无存活士兵
    bool canDeployAny = false;
    auto availableTroops = BattleManager::getInstance()->getAllAvailableTroops();
    for (const auto& pair : availableTroops)
    {
        if (pair.second > 0)
        {
            canDeployAny = true;
            break;
        }
    }

    bool anyTroopAlive = false;
    for (auto troop : BattleManager::getInstance()->getTroops())
    {
        if (!troop->isDead())
        {
            anyTroopAlive = true;
            break;
        }
    }

    if (!canDeployAny && !anyTroopAlive)
    {
        CCLOG("DEFEAT!");
        this->unscheduleUpdate(); // 停止检查

        // 显示失败提示
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto loseSprite = Sprite::create("defeat.png"); // 确保文件名正确
        if (loseSprite)
        {
            loseSprite->setPosition(visibleSize / 2);
            loseSprite->setScale(0.0f); // 初始设为0
            this->addChild(loseSprite, 1000);

            // 添加一个灰色的半透明背景层，让失败文字更清晰（可选）
            /*
            auto darkLayer = LayerColor::create(Color4B(0, 0, 0, 150));
            this->addChild(darkLayer, 999); // 层级比图片低1
            */

            // 添加弹出动画
            loseSprite->runAction(Sequence::create(
                EaseBackOut::create(ScaleTo::create(0.4f,0.6f)), // 使用回弹缓动效果
                nullptr
            ));
        }
        else
        {
            CCLOG("Error: defeat.png not found!");
        }

        // 2秒后返回主界面
        this->scheduleOnce
        ([=](float dt) 
            {
            this->onBattleEnd(); // 同步死亡兵数
            Director::getInstance()->replaceScene(TransitionFade::create(1.0f, MainMode::createScene()));
            }, 2.0f, "ReturnToMain"
        );
        return;
    }
}

void BattleScene::onBattleEnd()
{
    // 获取战斗中死亡的兵种数量
    auto bm = BattleManager::getInstance();
    auto deathCounts = bm->getDeathCounts();

    // 同步死亡数量到GameManager
    auto gm = GameManager::getInstance();
    for (const auto& pair : deathCounts)
    {
        TroopType type = pair.first;
        int deathCount = pair.second;

        if (deathCount > 0)
        {
            gm->consumeTroops(type, deathCount);
        }
    }

    // 清空战斗管理器
    bm->clear();
}

// 初始化兵种数量显示
void BattleScene::initTroopCountLabels()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    float startY = 100; // 起始Y坐标
    float lineHeight = 20; // 行高

    // 兵种信息数组
    struct TroopInfo
    {
        TroopType type;
        std::string name;
        Color3B color;
    };

    std::vector<TroopInfo> troopTypes =
    {
        {TroopType::BARBARIAN, "Barb:", Color3B::GREEN},
        {TroopType::ARCHER, "Arch:", Color3B::MAGENTA},
        {TroopType::GIANT, "Giant:", Color3B::ORANGE},
        {TroopType::BOMBERMAN, "Bomb:", Color3B::GRAY},
        {TroopType::DRAGON, "Dragon:", Color3B::RED }
    };

    // 为每种兵种创建数量标签
    for (size_t i = 0; i < troopTypes.size(); ++i)
    {
        const TroopInfo& info = troopTypes[i];

        auto label = Label::createWithSystemFont(info.name + " 0", "Arial", 16);
        label->setColor(info.color);
        label->setPosition(Vec2(visibleSize.width - 300 + i * 60, startY));
        label->setAnchorPoint(Vec2(0.5, 0.5));
        this->addChild(label, 10);

        m_troopCountLabels[info.type] = label;
    }
}

// 更新兵种数量显示
void BattleScene::updateTroopCountLabels()
{
    auto bm = BattleManager::getInstance();

    for (const auto& pair : m_troopCountLabels)
    {
        TroopType type = pair.first;
        Label* label = pair.second;

        // 从BattleManager获取剩余数量
        int count = bm->getAvailableTroopCount(type);

        // 获取兵种名称
        std::string troopName;
        switch (type)
        {
        case TroopType::BARBARIAN: troopName = "Barb:";  break;
        case TroopType::ARCHER:    troopName = "Arch:";  break;
        case TroopType::GIANT:     troopName = "Giant:"; break;
        case TroopType::BOMBERMAN: troopName = "Bomb:";  break;
        case TroopType::DRAGON:    troopName = "Dragon:"; break;
        }

        label->setString(troopName + std::to_string(count));

        // 如果数量为0，将标签颜色变暗
        if (count == 0)
        {
            label->setColor(Color3B(80, 80, 80)); // 暗灰色
        }
        else
        {
            // 恢复原始颜色
            switch (type)
            {
            case TroopType::BARBARIAN: label->setColor(Color3B::GREEN); break;
            case TroopType::ARCHER: label->setColor(Color3B::MAGENTA); break;
            case TroopType::GIANT: label->setColor(Color3B::ORANGE); break;
            case TroopType::BOMBERMAN: label->setColor(Color3B::GRAY); break;
            case TroopType::DRAGON: label->setColor(Color3B::RED); break;
            }
        }
    }
}

// 【新增】辅助函数：快速创建按钮
void BattleScene::createSelectButton(const std::string& title, Color3B color, TroopType type, int index)
{
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 创建按钮
    auto btn = Button::create("btn3.png");
    btn->setScale(0.1f);
    btn->setTitleText(title);
    btn->setTitleFontSize(180);

    // 排列在右下角
    // index * 60 意味着每个按钮间隔 60 像素
    btn->setPosition(Vec2(visibleSize.width - 300 + index * 60, 50));

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
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 如果点击到了按钮区域（屏幕下方 80 像素内），就不放兵，防止误触
    if (touchLoc.y < 80) return false;

    // 【修改】检查投放区域限制 - 中央区域禁放
    float leftEdge = visibleSize.width * 0.25f;
    float rightEdge = visibleSize.width * 0.75f;
    float bottomEdge = visibleSize.height * 0.25f;
    float topEdge = visibleSize.height * 0.75f;

    // 【关键修改】如果触摸位置在中央禁放区域内，则不允许投放
    if (touchLoc.x >= leftEdge && touchLoc.x <= rightEdge &&
        touchLoc.y >= bottomEdge && touchLoc.y <= topEdge) {

        // 显示提示信息
        m_infoLabel->setString("Cannot deploy in red area!");
        m_infoLabel->setColor(Color3B::RED);

        // 闪烁禁放区域作为提示
        m_deployAreaVisual->runAction(Sequence::create(
            Blink::create(0.4f, 2),
            CallFunc::create([this]() {
                // 确保恢复原始颜色
                m_deployAreaVisual->clear();
                // 重新绘制红色禁放区域
                Size visibleSize = Director::getInstance()->getVisibleSize();
                float leftEdge = visibleSize.width * 0.2f;
                float rightEdge = visibleSize.width * 0.8f;
                float bottomEdge = visibleSize.height * 0.2f;
                float topEdge = visibleSize.height * 0.8f;
                Vec2 vertices[] = {
                    Vec2(leftEdge, bottomEdge),
                    Vec2(rightEdge, bottomEdge),
                    Vec2(rightEdge, topEdge),
                    Vec2(leftEdge, topEdge)
                };
                m_deployAreaVisual->drawPolygon(vertices, 4, Color4F(1.0f, 0.0f, 0.0f, 0.2f), 2, Color4F(1.0f, 0.0f, 0.0f, 0.5f));
                }),
            nullptr
        ));

        return false;
    }

    if (!BattleManager::getInstance()->canDeployTroop(m_selectedType))
    {
        CCLOG("No more troops of this type available!");
        // 可以添加一个提示效果
        m_infoLabel->setString("No more troops available!");
        m_infoLabel->setColor(Color3B::RED);
        return true;
    }

    // 投放兵种，减少可用数量
    auto troop = Troop::create(m_selectedType);
    troop->setPosition(touchLoc);
    this->addChild(troop);

    BattleManager::getInstance()->deployTroop(m_selectedType);

    // 更新兵种数量显示
    updateTroopCountLabels();


    return true;
}

void BattleScene::onExit()
{
    // 确保调度器被停止
    this->unscheduleAllCallbacks();

    // 清理战斗管理器
    BattleManager::getInstance()->clear();

    // 调用父类方法
    Scene::onExit();
}

// 【新增】关卡配置逻辑
void BattleScene::loadLevel(int levelIndex)
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 center = visibleSize / 2;


    if (levelIndex == 1)
    {
        // --- 第一关：新手村 ---
        // 只有 1 个大本营
        auto town = Building::create(BuildingType::TOWN_HALL);
        town->setPosition(center);
        this->addChild(town);
    }
    else if (levelIndex == 2)
    {
        // --- 第二关：初级挑战 ---
        // 大本营 + 1 个加农炮
        auto town = Building::create(BuildingType::TOWN_HALL);
        town->setPosition(center);
        this->addChild(town);

        auto cannon = Building::create(BuildingType::CANNON);
        cannon->setPosition(center + Vec2(150, 0)); // 右边放个炮
        this->addChild(cannon);
    }
    else if (levelIndex == 3)
    {
        // --- 第三关：攻坚战 ---
        // 大本营 + 2 个炮 + 围墙保护
        auto town = Building::create(BuildingType::TOWN_HALL);
        town->setPosition(center);
        this->addChild(town);

        // 左青龙右白虎上朱雀下玄武,四大护法
        auto c1 = Building::create(BuildingType::CANNON);
        c1->setPosition(center + Vec2(150, 50));
        this->addChild(c1);
        auto c2 = Building::create(BuildingType::ARCHER_TOWER);
        c2->setPosition(center + Vec2(150, -50));
        this->addChild(c2);
        auto c3 = Building::create(BuildingType::CANNON);
        c3->setPosition(center + Vec2(-150, 50));
        this->addChild(c3);
        auto c4 = Building::create(BuildingType::ARCHER_TOWER);
        c4->setPosition(center + Vec2(-150, -50));
        this->addChild(c4);

        //放两排墙
        for (int i = 0; i < 5; i++)
        {
            auto wall_front = Building::create(BuildingType::WALL);
            auto wall_back= Building::create(BuildingType::WALL);
            wall_front->setPosition(center + Vec2(i*200-400,-200));
            wall_back->setPosition(center + Vec2(i * 200 - 400, 200));
            this->addChild(wall_front);
            this->addChild(wall_back);
        }
        //两边也放墙
        auto wall_left = Building::create(BuildingType::WALL);
        auto wall_right = Building::create(BuildingType::WALL);
        wall_left->setPosition(center + Vec2(-400, 0));
        wall_right->setPosition(center + Vec2(400, 0));
        this->addChild(wall_left);
        this->addChild(wall_right);

        // 放置一个陷阱
        auto trap = Building::create(BuildingType::TRAP);
        trap->setPosition(center + Vec2(0, -400));
        this->addChild(trap);
    }
}