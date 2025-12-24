#include"UpgradeTroopUI.h"
#include"GameManager.h"
#include"BattleManager.h"
USING_NS_CC;
using namespace ui;

bool UpgradeUi::init()
{
    if (!Layer::init())
    {
        return false;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Layout* m_mainPanel = Layout::create();
    m_mainPanel->setBackGroundImage("barracksBoard.png");
    m_mainPanel->setContentSize(Size(visibleSize.width * 0.8, visibleSize.height * 0.6));
    m_mainPanel->setScale(1.8f);
    m_mainPanel->setAnchorPoint(Vec2(0.5, 0.5));
    m_mainPanel->setPosition(visibleSize / 2);
    this->addChild(m_mainPanel);

    Size panelSize = m_mainPanel->getContentSize();

    auto titleLabel = Label::createWithSystemFont("Upgrade Troops", "Arial", 36);
    titleLabel->setPosition(Vec2(panelSize.width / 2, panelSize.height - 80));
    titleLabel->setColor(Color3B::BLACK);
    m_mainPanel->addChild(titleLabel);

    std::vector<UpgradeTroops> items = {
        {"Barbarian", calculateUpgradeCost(TroopType::BARBARIAN), TroopType::BARBARIAN, "barbarian_icon.png"},
        {"Archer", calculateUpgradeCost(TroopType::ARCHER), TroopType::ARCHER, "archer_icon.png"},
        {"Giant", calculateUpgradeCost(TroopType::GIANT), TroopType::GIANT, "giant_icon.png"},
        {"Bomberman", calculateUpgradeCost(TroopType::BOMBERMAN), TroopType::BOMBERMAN, "bomberman_icon.png"},
        {"Dragon", calculateUpgradeCost(TroopType::DRAGON), TroopType::DRAGON, "dragon_icon.png"}
    };
    int columns = 4;

    float startX = panelSize.width * 0.32f;
    float startY = panelSize.height * 0.55f;
    float gapX = 160.0f;
    float gapY = 220.0f;

    for (int i = 0; i < items.size(); ++i)
    {
        int row = i / columns;
        int col = i % columns;
        float x = startX + col * gapX;
        float y = startY - row * gapY;
        createUpgradeButton(items[i], Vec2(x, y), m_mainPanel);
    }

    refreshUpgradeCosts();

    auto closeBtn = Button::create("no.png");
    closeBtn->setScale(0.1f);
    closeBtn->setPosition(Vec2(panelSize.width - 350, panelSize.height - 200));
    closeBtn->addClickEventListener([this](Ref*) {
        this->hide();
    });
    m_mainPanel->addChild(closeBtn);

    return true;
}

void UpgradeUi::hide()
{
    this->setVisible(false);
}

void UpgradeUi::show()
{
    refreshUpgradeCosts();
    this->setVisible(true);
    this->setLocalZOrder(1000);
}

void UpgradeUi::createUpgradeButton(const UpgradeTroops& troops, Vec2 pos, Node* parentnode) {
    auto btn = ui::Button::create("ItemFrame.png");
    btn->setScale(0.2f);
    btn->setPosition(pos);

    btn->setTag(1000 + static_cast<int>(troops.type));

    auto icon = Sprite::create(troops.iconpath);
    if (icon) {
        icon->setPosition(btn->getContentSize() / 2);
        icon->setScale(0.35f);
        if (troops.type == TroopType::DRAGON) {
            icon->setScale(1.5f);
        }
        btn->addChild(icon);
    }
    auto priceLabel = Label::createWithSystemFont("Elixir: " + std::to_string(troops.upgrade_cost), "Arial", 14);
    priceLabel->setPosition(Vec2(btn->getContentSize().width / 2, 40));
    priceLabel->setColor(Color3B::BLACK);
    priceLabel->setScale(5);
    btn->addChild(priceLabel);
    m_priceLabels[troops.type] = priceLabel;

    auto nameLabel = Label::createWithSystemFont(troops.name, "Arial", 18);
    nameLabel->setPosition(Vec2(btn->getContentSize().width / 2, 160));
    nameLabel->setColor(Color3B::BLACK);
    nameLabel->setScale(5);
    btn->addChild(nameLabel);

    btn->addClickEventListener([=](Ref*) {
        auto gm = GameManager::getInstance();
        int m_resource = gm->getElixir();
        int cost = calculateUpgradeCost(troops.type);
        if (m_resource < cost) {
            CCLOG("Not enough elixir to upgrade %s", troops.name.c_str());
            return;
        }
        int newLevel = 0;
        switch (troops.type) {
        case TroopType::BARBARIAN: {
            int level = gm->getBarLevel();
            newLevel = level + 1;
            gm->setBarLevel(newLevel);
            break;
        }
        case TroopType::ARCHER: {
            int level = gm->getArcLevel();
            newLevel = level + 1;
            gm->setArcLevel(newLevel);
            break;
        }
        case TroopType::GIANT: {
            int level = gm->getGiantLevel();
            newLevel = level + 1;
            gm->setGiantLevel(newLevel);
            break;
        }
        case TroopType::BOMBERMAN: {
            int level = gm->getBomLevel();
            newLevel = level + 1;
            gm->setBomLevel(newLevel);
            break;
        }
        case TroopType::DRAGON: {
            int level = gm->getDragonLevel();
            newLevel = level + 1;
            gm->setDragonLevel(newLevel);
            break;
        }
        default:
            break;
        }
        if (newLevel > 0) {
            BattleManager::getInstance()->rescaleTroopsForType(troops.type, newLevel);
        }
        gm->addElixir(-cost);
        onTroopUpgraded();
    });

    if (parentnode)
    {
        parentnode->addChild(btn);
    }
}

int UpgradeUi::calculateUpgradeCost(TroopType type) const {
    auto gm = GameManager::getInstance();
    switch (type) {
    case TroopType::BARBARIAN:
        return gm->getBarLevel() * 50;
    case TroopType::ARCHER:
        return gm->getArcLevel() * 100;
    case TroopType::GIANT:
        return gm->getGiantLevel() * 250;
    case TroopType::BOMBERMAN:
        return gm->getBomLevel() * 100;
    case TroopType::DRAGON:
        return gm->getDragonLevel() * 500;
    default:
        return 0;
    }
}

void UpgradeUi::refreshUpgradeCosts() {
    for (auto& pair : m_priceLabels) {
        if (!pair.second) continue;
        int cost = calculateUpgradeCost(pair.first);
        pair.second->setString("Elixir: " + std::to_string(cost));
    }
}

void UpgradeUi::onTroopUpgraded() {
    refreshUpgradeCosts();
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_ELIXIR");
}