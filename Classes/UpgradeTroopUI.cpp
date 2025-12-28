#include "UpgradeTroopUI.h"
#include "GameManager.h"
#include "BattleManager.h"
USING_NS_CC;
using namespace ui;

// 初始化
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

    // 创建标题
    auto titleLabel = Label::createWithSystemFont("Upgrade Troops", "Arial", 36);
    titleLabel->setPosition(Vec2(panelSize.width / 2, panelSize.height - 80));
    titleLabel->setColor(Color3B::BLACK);
    m_mainPanel->addChild(titleLabel);

    // 定义兵种升级数据
    std::vector<UpgradeTroops> items = 
    {
        {"Barbarian", calculateUpgradeCost(TroopType::BARBARIAN), TroopType::BARBARIAN, "barbarian_icon.png"},
        {"Archer", calculateUpgradeCost(TroopType::ARCHER), TroopType::ARCHER, "archer_icon.png"},
        {"Giant", calculateUpgradeCost(TroopType::GIANT), TroopType::GIANT, "giant_icon.png"},
        {"Bomberman", calculateUpgradeCost(TroopType::BOMBERMAN), TroopType::BOMBERMAN, "bomberman_icon.png"},
        {"Dragon", calculateUpgradeCost(TroopType::DRAGON), TroopType::DRAGON, "dragon_icon.png"}
    };

    // 计算网格布局
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

    // 刷新升级费用
    refreshUpgradeCosts();

    // 创建关闭按钮
    auto closeBtn = Button::create("no.png");
    closeBtn->setScale(0.1f);
    closeBtn->setPosition(Vec2(panelSize.width - 350, panelSize.height - 200));
    closeBtn->addClickEventListener([this](Ref*)
        {
            this->hide();
        });
    m_mainPanel->addChild(closeBtn);

    return true;
}

// 隐藏UI
void UpgradeUi::hide()
{
    this->setVisible(false);
}

// 显示UI
void UpgradeUi::show()
{
    refreshUpgradeCosts();
    this->setVisible(true);
    this->setLocalZOrder(1000);
}

// 创建升级兵种按钮
void UpgradeUi::createUpgradeButton(const UpgradeTroops& troops, Vec2 pos, Node* parentnode)
{
    // 创建底座按钮
    auto btn = ui::Button::create("ItemFrame.png");
    btn->setScale(0.2f);
    btn->setPosition(pos);

    // 设置按钮Tag用于识别兵种类型
    btn->setTag(1000 + static_cast<int>(troops.type));

    // 添加图标
    auto icon = Sprite::create(troops.iconpath);
    if (icon)
    {
        icon->setPosition(btn->getContentSize() / 2);
        icon->setScale(0.35f);
        if (troops.type == TroopType::DRAGON)
        {
            icon->setScale(1.5f);
        }
        btn->addChild(icon);
    }

    // 添加价格标签
    auto priceLabel = Label::createWithSystemFont("Elixir: " + std::to_string(troops.upgrade_cost), "Arial", 14);
    priceLabel->setPosition(Vec2(btn->getContentSize().width / 2, 40));
    priceLabel->setColor(Color3B::BLACK);
    priceLabel->setScale(5);
    btn->addChild(priceLabel);
    m_priceLabels[troops.type] = priceLabel;

    // 获取并显示当前等级
    int currentLevel = 1;
    auto gm = GameManager::getInstance();
    switch (troops.type)
    {
    case TroopType::BARBARIAN:  currentLevel = gm->getBarLevel();    break;
    case TroopType::ARCHER:     currentLevel = gm->getArcLevel();    break;
    case TroopType::GIANT:      currentLevel = gm->getGiantLevel();  break;
    case TroopType::BOMBERMAN:  currentLevel = gm->getBomLevel();    break;
    case TroopType::DRAGON:     currentLevel = gm->getDragonLevel(); break;
    }

    auto levelLabel = Label::createWithSystemFont("Lv." + std::to_string(currentLevel), "Arial", 18);
    levelLabel->setPosition(Vec2(btn->getContentSize().width / 2, 200)); // 放在名字上方
    levelLabel->setColor(Color3B::BLUE);
    levelLabel->setScale(5);
    btn->addChild(levelLabel);

    // 保存等级标签引用以便刷新
    m_levelLabels[troops.type] = levelLabel;

    // 添加名称标签
    auto nameLabel = Label::createWithSystemFont(troops.name, "Arial", 18);
    nameLabel->setPosition(Vec2(btn->getContentSize().width / 2, 160));
    nameLabel->setColor(Color3B::BLACK);
    nameLabel->setScale(5);
    btn->addChild(nameLabel);

    // 绑定点击事件
    btn->addClickEventListener([=](Ref*)
        {
            auto gm = GameManager::getInstance();
            int m_resource = gm->getElixir();
            int cost = calculateUpgradeCost(troops.type);

            // 检查资源
            if (m_resource < cost)
            {
                return;
            }

            int newLevel = 0;
            // 根据兵种类型增加对应等级
            switch (troops.type)
            {
            case TroopType::BARBARIAN:
            {
                int level = gm->getBarLevel();
                newLevel = level + 1;
                gm->setBarLevel(newLevel);
                break;
            }
            case TroopType::ARCHER:
            {
                int level = gm->getArcLevel();
                newLevel = level + 1;
                gm->setArcLevel(newLevel);
                break;
            }
            case TroopType::GIANT:
            {
                int level = gm->getGiantLevel();
                newLevel = level + 1;
                gm->setGiantLevel(newLevel);
                break;
            }
            case TroopType::BOMBERMAN:
            {
                int level = gm->getBomLevel();
                newLevel = level + 1;
                gm->setBomLevel(newLevel);
                break;
            }
            case TroopType::DRAGON:
            {
                int level = gm->getDragonLevel();
                newLevel = level + 1;
                gm->setDragonLevel(newLevel);
                break;
            }
            default:
                break;
            }

            // 更新战斗中的兵种属性
            if (newLevel > 0)
            {
                BattleManager::getInstance()->rescaleTroopsForType(troops.type, newLevel);
            }

            // 扣除资源
            gm->addElixir(-cost);
            // 刷新界面
            onTroopUpgraded();
        });

    // 将按钮添加到父节点
    if (parentnode)
    {
        parentnode->addChild(btn);
    }
}

// 计算升级费用
int UpgradeUi::calculateUpgradeCost(TroopType type) const
{
    auto gm = GameManager::getInstance();
    switch (type)
    {
    case TroopType::BARBARIAN:    return gm->getBarLevel() * 50;
    case TroopType::ARCHER:       return gm->getArcLevel() * 100;
    case TroopType::GIANT:        return gm->getGiantLevel() * 250;
    case TroopType::BOMBERMAN:    return gm->getBomLevel() * 100;
    case TroopType::DRAGON:       return gm->getDragonLevel() * 500;
    default:return 0;
    }
}

// 刷新升级费用显示
void UpgradeUi::refreshUpgradeCosts()
{
    auto gm = GameManager::getInstance();

    // 遍历更新价格标签
    for (auto& pair : m_priceLabels)
    {
        if (!pair.second)
        {
            continue;
        }
        int cost = calculateUpgradeCost(pair.first);
        pair.second->setString("Elixir: " + std::to_string(cost));
    }

    // 遍历更新等级显示
    for (auto& pair : m_levelLabels)
    {
        if (!pair.second)
        {
            continue;
        }
        int currentLevel = 1;
        switch (pair.first)
        {
        case TroopType::BARBARIAN:  currentLevel = gm->getBarLevel();    break;
        case TroopType::ARCHER:     currentLevel = gm->getArcLevel();    break;
        case TroopType::GIANT:      currentLevel = gm->getGiantLevel();  break;
        case TroopType::BOMBERMAN:  currentLevel = gm->getBomLevel();    break;
        case TroopType::DRAGON:     currentLevel = gm->getDragonLevel(); break;
        }
        pair.second->setString("Lv." + std::to_string(currentLevel));
    }
}

// 兵种升级回调
void UpgradeUi::onTroopUpgraded()
{
    // 刷新界面上的费用和等级显示
    refreshUpgradeCosts();
    // 发送圣水更新事件，通知主界面UI刷新
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_ELIXIR");
}