#include "Barracks.h"
#include "RecruitUI.h"
#include "GameManager.h"
#include "GameUI.h"
#include "MainModeScene.h"
#include "Troop.h"
#include "Building.h"
#include "ui/CocosGUI.h"
USING_NS_CC;
using namespace ui;

// 初始化
bool RecruitUI::init()
{
    if (!Layer::init())
    {
        return false;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 创建主面板
    m_mainPanel = Layout::create();
    m_mainPanel->setBackGroundImage("barracksBoard.png");
    m_mainPanel->setContentSize(Size(visibleSize.width * 0.8, visibleSize.height * 0.6));
    m_mainPanel->setScale(1.8f);
    m_mainPanel->setAnchorPoint(Vec2(0.5, 0.5));
    m_mainPanel->setPosition(visibleSize / 2); // 直接放在屏幕正中间
    this->addChild(m_mainPanel);

    // 添加Cost信息显示区域
    float infoY = m_mainPanel->getContentSize().height - 80;

    // 当前Cost使用显示
    m_currentCostLabel = Label::createWithSystemFont("Current Cost: --/--", "Arial", 28);
    m_currentCostLabel->setPosition(Vec2(m_mainPanel->getContentSize().width / 2, infoY - 40));
    m_currentCostLabel->setColor(Color3B::GREEN);
    m_mainPanel->addChild(m_currentCostLabel);

    // 初始化UI内容
    initUI();

    // 初始隐藏
    this->hide();

    return true;
}

// 初始化UI元素
void RecruitUI::initUI()
{
    Size panelSize = m_mainPanel->getContentSize();

    // 创建标题
    auto titleLabel = Label::createWithSystemFont("Recruit Troops", "Arial", 36);
    // 放在顶部靠下的位置
    titleLabel->setPosition(Vec2(panelSize.width / 2, panelSize.height - 80));
    titleLabel->setColor(Color3B::BLACK); // 建议设为黑色，防止背景太亮看不清
    m_mainPanel->addChild(titleLabel);

    // 定义可招募的兵种数据
    std::vector<RecruitItem> items =
    {
        // 名字, 类型, 招募花费(圣水), 兵种cost(现在通过 getStaticTroopCost 获取), 锁住状态(现在通过 getStaticTroopMinLevel 获取), 图片
        {"Barbarian", TroopType::BARBARIAN, 50,  Troop::getStaticTroopCost(TroopType::BARBARIAN), false, "barbarian_icon.png"},
        {"Archer",    TroopType::ARCHER,    100, Troop::getStaticTroopCost(TroopType::ARCHER),    false, "archer_icon.png"},
        {"Giant",     TroopType::GIANT,     250, Troop::getStaticTroopCost(TroopType::GIANT),     false, "giant_icon.png"},
        {"Bomberman", TroopType::BOMBERMAN, 100, Troop::getStaticTroopCost(TroopType::BOMBERMAN), false, "bomberman_icon.png"},
        {"Dragon",    TroopType::DRAGON,    500, Troop::getStaticTroopCost(TroopType::DRAGON),    false, "dragon_icon.png"}
    };

    // 计算网格布局并调用 createRecruitItemButton
    // 现在的按钮比较大，不能像列表一样排，建议一行排 3 个
    int columns = 4;

    // 起始位置参数 (根据你的面板大小微调)
    float startX = panelSize.width * 0.35f; // 从左边 32% 处开始
    float startY = panelSize.height * 0.58f; // 从高度 55% 处开始 (第一排)
    float gapX = 160.0f; // 左右间距
    float gapY = 220.0f; // 上下间距

    for (int i = 0; i < items.size(); ++i)
    {
        // 计算行列索引
        int row = i / columns; // 第几行 (0, 0, 0, 1, 1...)
        int col = i % columns; // 第几列 (0, 1, 2, 0, 1...)

        // 计算坐标
        float x = startX + col * gapX;
        float y = startY - row * gapY; // y 轴是向下递减的

        // 直接调用封装好的函数
        // 所有的创建逻辑、点击逻辑都在这个函数里完成了
        createRecruitItemButton(items[i], Vec2(x, y), m_mainPanel);
    }

    // 创建关闭按钮 (建议放在右上角，不要挡住兵种按钮)
    auto closeBtn = Button::create("no.png");
    closeBtn->setScale(0.1f);
    closeBtn->setPosition(Vec2(panelSize.width - 350, panelSize.height - 200));
    closeBtn->addClickEventListener([this](Ref*)
        {
            this->hide();
        });
    m_mainPanel->addChild(closeBtn);
}

// 更新Cost显示
void RecruitUI::updateCostDisplay()
{
    auto mainScene = dynamic_cast<MainMode*>(Director::getInstance()->getRunningScene());
    if (!mainScene)
    {
        return;
    }

    auto building = mainScene->getBarracksBuilding();
    auto barracks = dynamic_cast<Barracks*>(building); // 强转
    if (!barracks)
    {
        if (m_costLimitLabel)
        {
            m_costLimitLabel->setString("Cost Limit: No Barracks");
        }
        if (m_currentCostLabel)
        {
            m_currentCostLabel->setString("Current Cost: --/--");
        }
        return;
    }

    int currentCost = barracks->getCurrentCostUsed();
    int maxCost = barracks->getMaxCostLimit();
    int level = barracks->getBarrackLevel();

    if (m_currentCostLabel)
    {
        m_currentCostLabel->setString("Current Cost: " + std::to_string(currentCost) + "/" + std::to_string(maxCost));

        // 根据cost使用情况改变颜色
        if (currentCost >= maxCost)
        {
            m_currentCostLabel->setColor(Color3B::RED);
        }
        else if (currentCost >= maxCost * 0.8)
        {
            m_currentCostLabel->setColor(Color3B::ORANGE);
        }
        else
        {
            m_currentCostLabel->setColor(Color3B::GREEN);
        }
    }
}

// 检查是否可以招募兵种
bool RecruitUI::canRecruitTroop(TroopType type)
{
    auto mainScene = dynamic_cast<MainMode*>(Director::getInstance()->getRunningScene());
    if (!mainScene)
    {
        return false;
    }

    auto building = mainScene->getBarracksBuilding();
    auto barracks = dynamic_cast<Barracks*>(building); // 强转
    if (!barracks)
    {
        return false;
    }

    // 检查军营等级
    int requiredLevel = Troop::getStaticTroopMinLevel(type);
    int currentLevel = barracks->getBarrackLevel(); // 强转
    if (currentLevel < requiredLevel)
    {
        return false;
    }

    // 检查cost
    int troopCost = Troop::getStaticTroopCost(type);
    int currentCost = barracks->getCurrentCostUsed(); // 强转
    int maxCost = barracks->getMaxCostLimit();        // 强转
    if (currentCost + troopCost > maxCost)
    {
        return false;
    }

    return true;
}

// 更新招募按钮状态
void RecruitUI::updateRecruitButtonState(ui::Button* btn, TroopType type)
{
    if (!btn)
    {
        return;
    }

    auto mainScene = dynamic_cast<MainMode*>(Director::getInstance()->getRunningScene());
    if (!mainScene)
    {
        btn->setEnabled(false);
        btn->setBright(false);
        return;
    }

    auto building = mainScene->getBarracksBuilding();
    auto barracks = dynamic_cast<Barracks*>(building);
    if (!barracks)
    {
        btn->setEnabled(false);
        btn->setBright(false);
        return;
    }

    // 检查解锁状态
    int requiredLevel = Troop::getStaticTroopMinLevel(type);
    int currentLevel = barracks->getBarrackLevel();
    bool isUnlocked = currentLevel >= requiredLevel;

    // 检查cost限制
    bool hasEnoughCost = canRecruitTroop(type);

    // 设置按钮状态
    btn->setEnabled(isUnlocked && hasEnoughCost);
    btn->setBright(isUnlocked && hasEnoughCost);

    // 更新按钮颜色表示状态
    if (!isUnlocked)
    {
        btn->setColor(Color3B::GRAY); // 未解锁：灰色
    }
    else if (!hasEnoughCost)
    {
        btn->setColor(Color3B::YELLOW); // cost不足：黄色（实际并不会显示黄色，而是介于白灰之间）
    }
    else
    {
        btn->setColor(Color3B::WHITE); // 可招募：白色
    }
}

// 显示UI
void RecruitUI::show()
{
    this->setVisible(true);
    // 显示时更新cost显示和按钮状态
    updateCostDisplay();

    // 更新所有按钮状态
    if (m_mainPanel)
    {
        for (auto child : m_mainPanel->getChildren())
        {
            auto btn = dynamic_cast<ui::Button*>(child);
            if (btn && btn->getTag() >= 1000)
            {
                // 假设招募按钮的tag >= 1000
                TroopType type = static_cast<TroopType>(btn->getTag() - 1000);
                updateRecruitButtonState(btn, type);
            }
        }
    }
}

// 隐藏UI
void RecruitUI::hide()
{
    this->setVisible(false);
}

// 实现退出逻辑
void RecruitUI::onExit()
{
    // 确保在节点退出时UI是隐藏的，防止在销毁过程中被点击
    this->hide();

    // 调用父类的 onExit，执行标准的清理流程
    Layer::onExit();
}

// 创建招募项按钮
void RecruitUI::createRecruitItemButton(const RecruitItem& item, Vec2 pos, Node* parentNode)
{
    // 创建底座
    auto btn = ui::Button::create("ItemFrame.png");
    btn->setScale(0.2f);
    btn->setPosition(pos);

    // 设置按钮tag用于识别兵种类型
    btn->setTag(1000 + static_cast<int>(item.type));

    // 添加图标 (直接使用 item.iconPath，无需辅助函数！)
    auto icon = Sprite::create(item.iconPath);
    if (icon)
    {
        icon->setPosition(btn->getContentSize() / 2);
        icon->setScale(0.35f); // 假设你的兵种图比较小，稍微放大点
        if (item.type == TroopType::DRAGON)
        {
            icon->setScale(1.5f);
        }
        btn->addChild(icon);
    }

    // 添加兵种cost显示
    auto costLabel = Label::createWithSystemFont("Cost: " + std::to_string(item.troopCost), "Arial", 16);
    costLabel->setPosition(Vec2(btn->getContentSize().width / 2, 95));
    costLabel->setColor(Color3B::BLACK);
    costLabel->setScale(4);
    btn->addChild(costLabel);

    // 添加价格标签
    auto priceLabel = Label::createWithSystemFont("Elixir: " + std::to_string(item.cost), "Arial", 14);
    priceLabel->setPosition(Vec2(btn->getContentSize().width / 2, 40));
    priceLabel->setColor(Color3B::BLACK);
    priceLabel->setScale(5);
    btn->addChild(priceLabel);

    // 添加名称标签
    auto nameLabel = Label::createWithSystemFont(item.name, "Arial", 18);
    nameLabel->setPosition(Vec2(btn->getContentSize().width / 2, 160));
    nameLabel->setColor(Color3B::BLACK);
    nameLabel->setScale(5);
    btn->addChild(nameLabel);

    // 绑定点击事件 - 添加完整检测
    btn->addClickEventListener([=](Ref*)
        {
            // 检查是否可以招募
            if (!canRecruitTroop(item.type))
            {
                // 显示详细原因
                showCannotRecruitDialog(item);
                return;
            }

            // 可以招募，执行招募逻辑
            auto gm = GameManager::getInstance();

            // 检查资源
            int currentResource = item.isGoldCost ? gm->getGold() : gm->getElixir();
            if (currentResource >= item.cost)
            {
                // 扣钱
                if (item.isGoldCost)
                {
                    gm->addGold(-item.cost);
                }
                else
                {
                    gm->addElixir(-item.cost);
                }

                // 加兵
                gm->addTroops(item.type, 1);

                // 更新军营cost使用
                auto mainScene = dynamic_cast<MainMode*>(Director::getInstance()->getRunningScene());
                if (mainScene)
                {
                    auto building = mainScene->getBarracksBuilding();
                    auto barracks = dynamic_cast<Barracks*>(building); // 强转
                    if (barracks)
                    {
                        barracks->updateCurrentCostUsed();
                    }
                }

                // 通知刷新
                Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_TROOPS");

                // 更新UI
                updateCostDisplay();
                updateRecruitButtonState(btn, item.type);
            }
            else
            {
                showNotEnoughResourcesDialog(item);
            }
        }
    );

    // 初始化按钮状态
    updateRecruitButtonState(btn, item.type);

    // 加到父节点
    if (parentNode)
    {
        parentNode->addChild(btn);
    }
}

// 显示无法招募对话框
void RecruitUI::showCannotRecruitDialog(const RecruitItem& item)
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto dialogBg = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
    this->addChild(dialogBg, 1000);

    std::string reason;
    auto mainScene = dynamic_cast<MainMode*>(Director::getInstance()->getRunningScene());
    if (mainScene)
    {
        auto building = mainScene->getBarracksBuilding();
        auto barracks = dynamic_cast<Barracks*>(building); // 强转

        if (barracks)
        {
            int requiredLevel = Troop::getStaticTroopMinLevel(item.type);
            int currentLevel = barracks->getBarrackLevel(); // 强转

            if (currentLevel < requiredLevel)
            {
                reason = "Need Barracks Level " + std::to_string(requiredLevel) +
                    " (Current: " + std::to_string(currentLevel) + ")";
            }
            else
            {
                int troopCost = Troop::getStaticTroopCost(item.type);
                int currentCost = barracks->getCurrentCostUsed(); // 强转
                int maxCost = barracks->getMaxCostLimit();        // 强转
                reason = "Not enough Cost Space!\nNeed: " + std::to_string(troopCost) +
                    ", Available: " + std::to_string(maxCost - currentCost);
            }
        }
        else
        {
            reason = "No Barracks Built!";
        }
    }

    auto messageLabel = Label::createWithSystemFont("Cannot Recruit " + item.name + "!\n" + reason,
        "Arial", 32);
    messageLabel->setPosition(visibleSize / 2);
    messageLabel->setTextColor(Color4B::WHITE);
    messageLabel->setAlignment(TextHAlignment::CENTER);
    dialogBg->addChild(messageLabel);

    auto okBtn = Button::create("CloseNormal.png");
    okBtn->setTitleText("OK");
    okBtn->setTitleFontSize(24);
    okBtn->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.4f));
    okBtn->addClickEventListener([dialogBg](Ref*)
        {
            dialogBg->removeFromParent();
        });
    dialogBg->addChild(okBtn);
}

// 显示资源不足对话框
void RecruitUI::showNotEnoughResourcesDialog(const RecruitItem& item)
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto dialogBg = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
    this->addChild(dialogBg, 1000);

    auto gm = GameManager::getInstance();
    int current = item.isGoldCost ? gm->getGold() : gm->getElixir();
    std::string resourceType = item.isGoldCost ? "Gold" : "Elixir";

    auto messageLabel = Label::createWithSystemFont("Not enough " + resourceType + "!\n" +
        "Need: " + std::to_string(item.cost) +
        ", Have: " + std::to_string(current),
        "Arial", 32);
    messageLabel->setPosition(visibleSize / 2);
    messageLabel->setTextColor(Color4B::WHITE);
    messageLabel->setAlignment(TextHAlignment::CENTER);
    dialogBg->addChild(messageLabel);

    auto okBtn = Button::create("CloseNormal.png");
    okBtn->setTitleText("OK");
    okBtn->setTitleFontSize(24);
    okBtn->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.4f));
    okBtn->addClickEventListener([dialogBg](Ref*)
        {
            dialogBg->removeFromParent();
        });
    dialogBg->addChild(okBtn);
}

// 进入场景回调
void RecruitUI::onEnter()
{
    Layer::onEnter();

    // 监听cost更新事件
    auto costListener = EventListenerCustom::create("EVENT_COST_UPDATED", [this](EventCustom* event)
        {
            updateCostDisplay();
            // 更新所有按钮状态
            if (m_mainPanel)
            {
                for (auto child : m_mainPanel->getChildren())
                {
                    auto btn = dynamic_cast<ui::Button*>(child);
                    if (btn && btn->getTag() >= 1000)
                    {
                        TroopType type = static_cast<TroopType>(btn->getTag() - 1000);
                        updateRecruitButtonState(btn, type);
                    }
                }
            }
        });
    _eventDispatcher->addEventListenerWithSceneGraphPriority(costListener, this);

    // 监听兵种数量变化事件
    auto troopListener = EventListenerCustom::create("EVENT_UPDATE_TROOPS", [this](EventCustom* event)
        {
            updateCostDisplay();
            // 更新所有按钮状态
            if (m_mainPanel)
            {
                for (auto child : m_mainPanel->getChildren())
                {
                    auto btn = dynamic_cast<ui::Button*>(child);
                    if (btn && btn->getTag() >= 1000)
                    {
                        TroopType type = static_cast<TroopType>(btn->getTag() - 1000);
                        updateRecruitButtonState(btn, type);
                    }
                }
            }
        });
    _eventDispatcher->addEventListenerWithSceneGraphPriority(troopListener, this);
}