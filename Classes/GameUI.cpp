// GameUI.cpp
#include "GameUI.h"
#include "GameManager.h" // 一定要包含这个，用来获取兵种数量
#include "GameEntity.h"  // 包含 TroopType 定义

USING_NS_CC;
using namespace ui;

bool GameUI::init() {
    if (!Layer::init()) {
        return false;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 初始化资源标签
    initResourceLabels();
    // 初始化各兵种标签
    initTroopLabels();

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
void GameUI::initResourceLabels()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 创建金币标签 (使用系统字体 Arial，字号 36)
    m_goldLabel = Label::createWithSystemFont("Gold: 0", "Arial", 36);
    m_goldLabel->setColor(Color3B::YELLOW);
    // 放在右上角
    m_goldLabel->setPosition(Vec2(visibleSize.width - 150, visibleSize.height - 40));
    // 锚点设为右对齐
    m_goldLabel->setAnchorPoint(Vec2(1, 0.5));
    this->addChild(m_goldLabel);

    // 2. 创建圣水标签
    m_elixirLabel = Label::createWithSystemFont("Elixir: 0", "Arial", 36);
    m_elixirLabel->setColor(Color3B::MAGENTA);
    m_elixirLabel->setPosition(Vec2(visibleSize.width - 150, visibleSize.height - 100));
    m_elixirLabel->setAnchorPoint(Vec2(1, 0.5));
    this->addChild(m_elixirLabel);
    //为金币和圣水添加图片
    auto gold = Sprite::create("coin_icon.png");
    gold->setPosition(Vec2(visibleSize.width * 0.80f-40, visibleSize.height * 0.97f));
    gold->setScale(0.05f);
    this->addChild(gold);
    auto water = Sprite::create("elixir_icon.png");
    water->setPosition(Vec2(visibleSize.width * 0.80f-40, visibleSize.height * 0.92f));
    water->setScale(0.06f);
    this->addChild(water);
}
void GameUI::initTroopLabels()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 创建入口按钮 (保持不变)
    auto armyBtn = Button::create("troop_icon.png");
    armyBtn->setPosition(Vec2(visibleSize.width - 650, 125));
    armyBtn->setScale(0.15f);
    armyBtn->addClickEventListener([=](Ref*) {
        this->showArmyPanel();
        });
    this->addChild(armyBtn);

    // ==========================================
    // 2. 创建弹窗面板 (加宽一点，方便放按钮)
    // ==========================================
    m_armyPanel = Layout::create();
    m_armyPanel->setBackGroundImage("barracksBoard.png");
    m_armyPanel->setBackGroundImageScale9Enabled(true);
    // 【修改】宽度从 400 改为 550，高度适当增加
    m_armyPanel->setContentSize(Size(visibleSize.width * 0.8, visibleSize.height * 0.8));
    m_armyPanel->setScale(1.3f);
    m_armyPanel->setAnchorPoint(Vec2(0.5, 0.5));
    m_armyPanel->setPosition(visibleSize / 2);
    m_armyPanel->setVisible(false);
    this->addChild(m_armyPanel, 100);

    // 标题 & 关闭按钮 (保持不变)
    auto title = Label::createWithSystemFont("Manage Troops", "Arial", 28);
    title->setPosition(Vec2(m_armyPanel->getContentSize().width / 2, m_armyPanel->getContentSize().height - 30));
    title->setColor(Color3B::BLACK);
    m_armyPanel->addChild(title);

    auto closeBtn = Button::create("no.png");
    closeBtn->setScale(0.15f);
    closeBtn->setPosition(Vec2(m_armyPanel->getContentSize().width - 25, m_armyPanel->getContentSize().height - 25));
    closeBtn->addClickEventListener([=](Ref*) { this->hideArmyPanel(); });
    m_armyPanel->addChild(closeBtn);

    // ==========================================
    // 3. 兵种列表 (带价格和购买功能)
    // ==========================================

    // 【修改】结构体增加价格信息，直接把 RecruitUI 里的数据拿过来
    struct TroopInfo {
        TroopType type;
        std::string name;
        std::string iconPath;
        int cost;           // 新增：价格
        bool isGoldCost;    // 新增：是否金币
    };

    std::vector<TroopInfo> troopInfos = {
        {TroopType::BARBARIAN, "Barbarian", "barbarian_icon.png", 50,  false},
        {TroopType::ARCHER,    "Archer",    "archer_icon.png",    100, false},
        {TroopType::GIANT,     "Giant",     "giant_icon.png",     250, false},
        {TroopType::BOMBERMAN, "Bomberman", "bomberman_icon.png", 100, false},
        {TroopType::DRAGON,    "Dragon",    "dragon_icon.png",    300, false}
    };

    float startX = 280;
    float startY = 800; // 起始高度稍微调高
    float gapY = 160;    // 行间距加大

    for (int i = 0; i < troopInfos.size(); ++i) {
        const auto& info = troopInfos[i]; // 捕获 info 供 Lambda 使用
        float y = startY - i * gapY;

        // A. 图标
        auto icon = Sprite::create(info.iconPath);
        if (icon) {
            icon->setScale(0.1f);
            if (info.type == TroopType::DRAGON) {
                icon->setScale(0.6f);
            }
            icon->setPosition(Vec2(startX, y));
            m_armyPanel->addChild(icon);
        }

        // B. 名称
        auto nameLabel = Label::createWithSystemFont(info.name, "Arial", 20);
        nameLabel->setAnchorPoint(Vec2(0, 0.5));
        nameLabel->setPosition(Vec2(startX + 140, y + 10)); // 稍微往上一点
        nameLabel->setColor(Color3B::BLACK);
        nameLabel->setScale(1.2f);
        m_armyPanel->addChild(nameLabel);

        // C. 价格显示 (新增)
        std::string costStr = (info.isGoldCost ? "Gold: " : "Elixir: ") + std::to_string(info.cost);
        auto costLabel = Label::createWithSystemFont(costStr, "Arial", 14);
        costLabel->setAnchorPoint(Vec2(0, 0.5));
        costLabel->setScale(1.5f);
        costLabel->setPosition(Vec2(startX + 140, y - 15)); // 放在名字下面
        // 金币用深黄，圣水用深紫
        costLabel->setColor(info.isGoldCost ? Color3B(200, 150, 0) : Color3B(150, 0, 150));
        m_armyPanel->addChild(costLabel);

        // D. 当前数量 (向右移)
        auto countLabel = Label::createWithSystemFont("x 0", "Arial", 22);
        countLabel->setAnchorPoint(Vec2(0, 0.5));
        countLabel->setPosition(Vec2(startX + 380, y));
        countLabel->setColor(Color3B::BLUE);
        countLabel->setScale(1.5f);
        m_armyPanel->addChild(countLabel);

        // 存入 Map 方便刷新
        m_troopLabels[info.type] = countLabel;

        // E. 【核心】训练按钮 (新增)
        auto trainBtn = Button::create("yes.png"); // 或者用 "upgrade_btn.png"
        trainBtn->setScale(0.1f); // 按钮不用太大
        trainBtn->setTitleText("Train");
        trainBtn->setTitleFontSize(24);
        trainBtn->setPosition(Vec2(startX + 750, y)); // 放在最右边

        // 绑定购买逻辑 (复用之前的逻辑)
        trainBtn->addClickEventListener([=](Ref*) {
            auto gm = GameManager::getInstance();
            int currentRes = info.isGoldCost ? gm->getGold() : gm->getElixir();

            // 1. 检查钱够不够
            if (currentRes >= info.cost) {
                // 2. 扣钱
                if (info.isGoldCost) gm->addGold(-info.cost);
                else gm->addElixir(-info.cost);

                // 3. 加兵
                gm->addTroops(info.type, 1);

                // 4. 【关键】立刻刷新界面上的数量
                // 这样你点一下，数量就会从 x5 变成 x6，反馈感很强
                this->updateArmyLabels();

                CCLOG("Trained 1 %s", info.name.c_str());
            }
            else {
                CCLOG("Not enough resources!");
                // 这里可以做一个简单的缩放动画提示钱不够
                trainBtn->runAction(Sequence::create(
                    ScaleTo::create(0.1f, 0.12f),
                    ScaleTo::create(0.1f, 0.1f),
                    nullptr
                ));
            }
            });
        m_armyPanel->addChild(trainBtn);
    }
}

// 显示弹窗并刷新数据
void GameUI::showArmyPanel()
{
    if (m_armyPanel) {
        updateArmyLabels(); // 打开前先刷新一下数据
        m_armyPanel->setVisible(true);

        // 简单的弹窗动画：从小变大
        m_armyPanel->setScale(0.1f);
        m_armyPanel->runAction(ScaleTo::create(0.2f, 1.0f));
    }
}

// 隐藏弹窗
void GameUI::hideArmyPanel()
{
    if (m_armyPanel) {
        m_armyPanel->setVisible(false);
    }
}

// 核心：刷新数据逻辑
void GameUI::updateArmyLabels()
{
    auto gm = GameManager::getInstance();

    // 遍历我们在 init 里存好的所有标签
    for (auto& pair : m_troopLabels)
    {
        TroopType type = pair.first;
        Label* label = pair.second;

        // 1. 从 GameManager 获取该兵种的真实拥有数量
        int count = gm->getTroopCount(type);

        // 2. 更新文字
        label->setString("x " + std::to_string(count));
    }
}
void GameUI::updateLabels() {
    // 从 GameManager 取出金币、圣水数量并更新
    int gold = GameManager::getInstance()->getGold();
    int elixir = GameManager::getInstance()->getElixir();
    int maxgold= GameManager::getInstance()->getMaxGold();
    int maxelixir= GameManager::getInstance()->getMaxElixir();
    m_goldLabel->setString("Gold: " + std::to_string(gold)+"/"+std::to_string(maxgold));
    m_elixirLabel->setString("Elixir: " + std::to_string(elixir)+"/"+std::to_string(maxelixir));
    // 更新各兵种数量
    auto gm = GameManager::getInstance();
    for (const auto& pair : m_troopLabels)
    {
        TroopType type = pair.first;
        Label* label = pair.second;
        int count = gm->getTroopCount(type);

        // 获取兵种名称
        std::string troopName;
        Color3B troopColor;
        switch (type)
        {
        case TroopType::BARBARIAN:troopName = "Barbarian:"; troopColor = Color3B::GREEN;  break;
        case TroopType::ARCHER:   troopName = "Archer:";   troopColor = Color3B::MAGENTA; break;
        case TroopType::GIANT:    troopName = "Giant:";    troopColor = Color3B::ORANGE; break;
        case TroopType::BOMBERMAN:troopName = "Bomberman:"; troopColor = Color3B::GRAY;   break;
        case TroopType::DRAGON:   troopName = "Dragon:"; troopColor = Color3B::RED; break;
        }

        // 更新标签文本和颜色
        label->setString(troopName + " " + std::to_string(count));
        label->setColor(troopColor);
    }
}