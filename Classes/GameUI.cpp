//GameUI.cpp
#include "GameUI.h"
#include "GameManager.h" // 需要读取数据
USING_NS_CC;

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
    m_goldLabel->setPosition(Vec2(visibleSize.width - 80, visibleSize.height - 40));
    // 锚点设为右对齐
    m_goldLabel->setAnchorPoint(Vec2(1, 0.5));
    this->addChild(m_goldLabel);

    // 2. 创建圣水标签
    m_elixirLabel = Label::createWithSystemFont("Elixir: 0", "Arial", 36);
    m_elixirLabel->setColor(Color3B::MAGENTA);
    m_elixirLabel->setPosition(Vec2(visibleSize.width - 80, visibleSize.height - 100));
    m_elixirLabel->setAnchorPoint(Vec2(1, 0.5));
    this->addChild(m_elixirLabel);
    //为金币和圣水添加图片
    auto gold = Sprite::create("coin_icon.png");
    gold->setPosition(Vec2(visibleSize.width * 0.86f, visibleSize.height * 0.97f));
    gold->setScale(0.05f);
    this->addChild(gold);
    auto water = Sprite::create("elixir_icon.png");
    water->setPosition(Vec2(visibleSize.width * 0.86f, visibleSize.height * 0.92f));
    water->setScale(0.06f);
    this->addChild(water);
}
void GameUI::initTroopLabels() // 初始化各兵种标签
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    float startY = visibleSize.height - 90; // 起始Y坐标
    float lineHeight = 25; // 行高

    // 兵种信息数组
    struct TroopInfo
    {
        TroopType type;
        std::string name;
        Color3B color;
    };

    std::vector<TroopInfo> troopTypes =
    {
        {TroopType::BARBARIAN, "Barbarian:", Color3B::GREEN},
        {TroopType::ARCHER, "Archer:", Color3B::MAGENTA},
        {TroopType::GIANT, "Giant:", Color3B::ORANGE},
        {TroopType::BOMBERMAN, "Bomberman:", Color3B::GRAY},
        {TroopType::DRAGON, "Dragon:", Color3B::RED}
    };

    // 为每种兵种创建标签
    for (size_t i = 0; i < troopTypes.size(); ++i)
    {
        const TroopInfo& info = troopTypes[i];

        // 创建标签，初始显示"名称: 0"
        auto label = Label::createWithSystemFont(info.name + " 0", "Arial", 20);
        label->setColor(info.color);
        label->setPosition(Vec2(visibleSize.width - 100, startY - i * lineHeight));
        label->setAnchorPoint(Vec2(1, 0.5));
        this->addChild(label);

        // 保存到map中
        m_troopLabels[info.type] = label;
    }
}
void GameUI::updateLabels() {
    // 从 GameManager 取出金币、圣水数量并更新
    int gold = GameManager::getInstance()->getGold();
    int elixir = GameManager::getInstance()->getElixir();
    m_goldLabel->setString("Gold: " + std::to_string(gold));
    m_elixirLabel->setString("Elixir: " + std::to_string(elixir));
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