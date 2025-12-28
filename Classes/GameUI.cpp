#include "Barracks.h"
#include "GameUI.h"
#include "GameManager.h"
#include "GameEntity.h"  // 包含 TroopType 定义
#include "MainModeScene.h"
#include "Troop.h"
#include "Building.h"
USING_NS_CC;
using namespace ui;

// 初始化
bool GameUI::init()
{
    if (!Layer::init())
    {
        return false;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 初始化资源标签
    initResourceLabels();
    // 初始化各兵种标签
    initTroopLabels();

    // 监听金币更新事件
    auto goldListener = EventListenerCustom::create
    ("EVENT_UPDATE_GOLD", [this](EventCustom* event)
        {
            this->updateLabels();
        }
    );
    _eventDispatcher->addEventListenerWithSceneGraphPriority(goldListener, this);

    // 监听圣水更新事件
    auto elixirListener = EventListenerCustom::create("EVENT_UPDATE_ELIXIR", [this](EventCustom* event)
        {
            this->updateLabels();
        });
    _eventDispatcher->addEventListenerWithSceneGraphPriority(elixirListener, this);

    // 监听兵种数量更新事件
    auto troopListener = EventListenerCustom::create
    ("EVENT_UPDATE_TROOPS", [this](EventCustom* event)
        {
            this->updateLabels();
        }
    );
    _eventDispatcher->addEventListenerWithSceneGraphPriority(troopListener, this);

    // 初始化显示一次
    updateLabels();

    return true;
}

// 初始化资源标签
void GameUI::initResourceLabels()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 创建金币标签 (使用系统字体 Arial，字号 36)
    m_goldLabel = Label::createWithSystemFont("Gold: 0", "Arial", 36);
    m_goldLabel->setColor(Color3B::YELLOW);
    // 放在右上角
    m_goldLabel->setPosition(Vec2(visibleSize.width - 150, visibleSize.height - 40));
    // 锚点设为右对齐
    m_goldLabel->setAnchorPoint(Vec2(1, 0.5));
    this->addChild(m_goldLabel);

    // 创建圣水标签
    m_elixirLabel = Label::createWithSystemFont("Elixir: 0", "Arial", 36);
    m_elixirLabel->setColor(Color3B::MAGENTA);
    m_elixirLabel->setPosition(Vec2(visibleSize.width - 150, visibleSize.height - 100));
    m_elixirLabel->setAnchorPoint(Vec2(1, 0.5));
    this->addChild(m_elixirLabel);

    // 为金币和圣水添加图标
    auto gold = Sprite::create("coin_icon.png");
    gold->setPosition(Vec2(visibleSize.width * 0.80f - 40, visibleSize.height * 0.97f));
    gold->setScale(0.05f);
    this->addChild(gold);

    auto water = Sprite::create("elixir_icon.png");
    water->setPosition(Vec2(visibleSize.width * 0.80f - 40, visibleSize.height * 0.92f));
    water->setScale(0.06f);
    this->addChild(water);
}

// 初始化兵种标签
void GameUI::initTroopLabels()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 创建入口按钮
    auto armyBtn = Button::create("troop_icon.png");
    armyBtn->setPosition(Vec2(visibleSize.width - 550, 125));
    armyBtn->setScale(0.15f);
    armyBtn->addClickEventListener([=](Ref*)
        {
            this->showArmyPanel();
        });
    this->addChild(armyBtn);

    // 创建弹窗面板
    m_armyPanel = Layout::create();
    m_armyPanel->setBackGroundImage("barracksBoard.png");
    m_armyPanel->setBackGroundImageScale9Enabled(true);
    // 设置面板大小与缩放
    m_armyPanel->setContentSize(Size(visibleSize.width * 0.8, visibleSize.height * 0.8));
    m_armyPanel->setScale(1.3f);
    m_armyPanel->setAnchorPoint(Vec2(0.5, 0.5));
    m_armyPanel->setPosition(visibleSize / 2);
    m_armyPanel->setVisible(false);
    this->addChild(m_armyPanel, 100);

    // 标题 & 关闭按钮
    auto title = Label::createWithSystemFont("Manage Troops", "Arial", 28);
    title->setPosition(Vec2(m_armyPanel->getContentSize().width / 2, m_armyPanel->getContentSize().height - 30));
    title->setColor(Color3B::BLACK);
    m_armyPanel->addChild(title);

    auto closeBtn = Button::create("no.png");
    closeBtn->setScale(0.15f);
    closeBtn->setPosition(Vec2(m_armyPanel->getContentSize().width - 25, m_armyPanel->getContentSize().height - 25));
    closeBtn->addClickEventListener([=](Ref*)
        {
            this->hideArmyPanel();
        });
    m_armyPanel->addChild(closeBtn);

    // 定义兵种信息结构体
    struct TroopInfo
    {
        TroopType type;
        std::string name;
        std::string iconPath;
        int cost;           // 兵种cost值
        int minLevel;       // 最低军营等级
    };

    std::vector<TroopInfo> troopInfos =
    {
        {TroopType::BARBARIAN, "Barbarian", "barbarian_icon.png", 1, 1},
        {TroopType::ARCHER,    "Archer",    "archer_icon.png",    1, 1},
        {TroopType::GIANT,     "Giant",     "giant_icon.png",     3, 2},
        {TroopType::BOMBERMAN, "Bomberman", "bomberman_icon.png", 2, 2},
        {TroopType::DRAGON,    "Dragon",    "dragon_icon.png",    5, 3}
    };

    float startX = 280;
    float startY = 800; // 起始高度稍微调高
    float gapY = 160;    // 行间距加大

    // 遍历创建兵种列表项
    for (int i = 0; i < troopInfos.size(); ++i)
    {
        const auto& info = troopInfos[i]; // 捕获 info 供 Lambda 使用
        float y = startY - i * gapY;

        // 图标
        auto icon = Sprite::create(info.iconPath);
        if (icon)
        {
            icon->setScale(0.1f);
            if (info.type == TroopType::DRAGON)
            {
                icon->setScale(0.6f);
            }
            icon->setPosition(Vec2(startX, y));
            m_armyPanel->addChild(icon);
        }

        // 名称
        auto nameLabel = Label::createWithSystemFont(info.name, "Arial", 20);
        nameLabel->setAnchorPoint(Vec2(0, 0.5));
        nameLabel->setPosition(Vec2(startX + 140, y + 10)); // 稍微往上一点
        nameLabel->setColor(Color3B::BLACK);
        nameLabel->setScale(1.2f);
        m_armyPanel->addChild(nameLabel);

        // 兵种Cost显示
        auto costLabel = Label::createWithSystemFont("Cost: " + std::to_string(info.cost), "Arial", 16);
        costLabel->setAnchorPoint(Vec2(0, 0.5));
        costLabel->setScale(1.5f);
        costLabel->setPosition(Vec2(startX + 140, y - 15));
        costLabel->setColor(Color3B::BLACK);
        m_armyPanel->addChild(costLabel);
        m_costLabels[info.type] = costLabel;

        // 当前数量显示
        auto countLabel = Label::createWithSystemFont("x 0", "Arial", 22);
        countLabel->setAnchorPoint(Vec2(0, 0.5));
        countLabel->setPosition(Vec2(startX + 380, y));
        countLabel->setColor(Color3B::BLACK);
        countLabel->setScale(1.5f);
        m_armyPanel->addChild(countLabel);
        m_troopLabels[info.type] = countLabel;

        // 解锁状态显示
        auto unlockLabel = Label::createWithSystemFont("To Unlock Needs Barrack Level:" + std::to_string(info.minLevel), "Arial", 16);
        unlockLabel->setAnchorPoint(Vec2(0, 0.5));
        unlockLabel->setScale(1.2f);
        unlockLabel->setPosition(Vec2(startX + 520, y));
        unlockLabel->setColor(Color3B::BLACK);
        m_armyPanel->addChild(unlockLabel);
    }
    // 添加总体Cost使用情况显示
    auto totalCostLabel = Label::createWithSystemFont("Total Cost: --/--", "Arial", 24);
    totalCostLabel->setTag(1002); // 用于后续更新
    totalCostLabel->setPosition(Vec2(m_armyPanel->getContentSize().width / 2, 100));
    totalCostLabel->setColor(Color3B::GREEN);
    m_armyPanel->addChild(totalCostLabel);
}

// 显示弹窗并刷新数据
void GameUI::showArmyPanel()
{
    if (m_armyPanel)
    {
        updateArmyLabels(); // 打开前先刷新一下数据
        updateCostDisplay(); // 更新cost显示
        m_armyPanel->setVisible(true);

        // 简单的弹窗动画：从小变大
        m_armyPanel->setScale(0.1f);
        m_armyPanel->runAction(ScaleTo::create(0.2f, 1.0f));
    }
}

// 隐藏弹窗
void GameUI::hideArmyPanel()
{
    if (m_armyPanel)
    {
        m_armyPanel->setVisible(false);
    }
}

// 核心：刷新数据逻辑
void GameUI::updateArmyLabels()
{
    auto gm = GameManager::getInstance();
    auto mainScene = dynamic_cast<MainMode*>(Director::getInstance()->getRunningScene());

    // 获取军营等级用于解锁状态检查
    int barrackLevel = 0;
    if (mainScene)
    {
        auto building = mainScene->getBarracksBuilding();
        auto barracks = dynamic_cast<Barracks*>(building);
        if (barracks)
        {
            barrackLevel = barracks->getBarrackLevel();
        }
    }

    // 遍历所有兵种标签
    for (auto& pair : m_troopLabels)
    {
        TroopType type = pair.first;
        Label* countLabel = pair.second;

        // 更新数量
        int count = gm->getTroopCount(type);
        countLabel->setString("x " + std::to_string(count));

        // 更新解锁状态颜色
        int minLevel = Troop::getStaticTroopMinLevel(type);
        if (barrackLevel >= minLevel)
        {
            countLabel->setColor(Color3B::BLUE); // 已解锁：蓝色
        }
        else
        {
            countLabel->setColor(Color3B::GRAY);  // 未解锁：灰色
        }
    }

    // 更新cost标签的解锁状态
    for (auto& pair : m_costLabels)
    {
        TroopType type = pair.first;
        Label* costLabel = pair.second;

        int minLevel = Troop::getStaticTroopMinLevel(type);
        if (barrackLevel >= minLevel)
        {
            costLabel->setColor(Color3B::BLUE); // 已解锁：蓝色
        }
        else
        {
            costLabel->setColor(Color3B::GRAY);  // 未解锁：灰色
        }
    }
}

// 刷新显示的文字
void GameUI::updateLabels()
{
    // 从 GameManager 取出金币、圣水数量并更新
    int gold = GameManager::getInstance()->getGold();
    int elixir = GameManager::getInstance()->getElixir();
    int maxgold = GameManager::getInstance()->getMaxGold();
    int maxelixir = GameManager::getInstance()->getMaxElixir();
    m_goldLabel->setString("Gold: " + std::to_string(gold) + "/" + std::to_string(maxgold));
    m_elixirLabel->setString("Elixir: " + std::to_string(elixir) + "/" + std::to_string(maxelixir));

    // 更新各兵种数量
    updateArmyLabels();

    // 更新cost显示
    updateCostDisplay();
}

// 更新cost显示的方法
void GameUI::updateCostDisplay()
{
    // 更新总体cost显示
    auto totalCostLabel = m_armyPanel->getChildByTag<Label*>(1002);
    if (totalCostLabel)
    {
        auto mainScene = dynamic_cast<MainMode*>(Director::getInstance()->getRunningScene());
        if (mainScene)
        {
            auto building = mainScene->getBarracksBuilding();
            auto barracks = dynamic_cast<Barracks*>(building);
            if (barracks)
            {
                int currentCost = barracks->getCurrentCostUsed();
                int maxCost = barracks->getMaxCostLimit();
                int level = barracks->getBarrackLevel();

                totalCostLabel->setString("Total Cost: " + std::to_string(currentCost) + "/" +
                    std::to_string(maxCost) + " (Lv." + std::to_string(level) + ")");

                // 根据使用情况改变颜色
                if (currentCost >= maxCost)
                {
                    totalCostLabel->setColor(Color3B::RED);
                }
                else if (currentCost >= maxCost * 0.8)
                {
                    totalCostLabel->setColor(Color3B::ORANGE);
                }
                else
                {
                    totalCostLabel->setColor(Color3B::GREEN);
                }
            }
            else
            {
                totalCostLabel->setString("Total Cost: No Barracks");
                totalCostLabel->setColor(Color3B::GRAY);
            }
        }
    }
}

// 监听cost更新事件
void GameUI::onEnter()
{
    Layer::onEnter();

    // 监听军营升级事件
    auto barrackListener = EventListenerCustom::create("EVENT_BARRACK_UPGRADED", [this](EventCustom* event)
        {
            updateArmyLabels();
            updateCostDisplay();
        });
    _eventDispatcher->addEventListenerWithSceneGraphPriority(barrackListener, this);
}