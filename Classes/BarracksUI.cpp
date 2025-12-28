#include "Barracks.h"
#include "BarracksUI.h"
#include "RecruitUI.h"
#include "GameManager.h"
#include "MainModeScene.h"
#include "Building.h"
#include "UpgradeTroopUI.h"
USING_NS_CC;
using namespace ui;

// 初始化
bool BarracksUI::init()
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
    m_mainPanel->setScale(1.3f);
    m_mainPanel->setAnchorPoint(Vec2(0.5, 0.5));
    m_mainPanel->setPosition(visibleSize / 2); // 直接放在屏幕正中间
    this->addChild(m_mainPanel);

    // 添加军营信息显示区域
    float infoY = m_mainPanel->getContentSize().height - 100;

    // 军营等级显示
    m_barrackLevelLabel = Label::createWithSystemFont("Barracks Level: 1", "Arial", 36);
    m_barrackLevelLabel->setPosition(Vec2(m_mainPanel->getContentSize().width / 2, infoY));
    m_barrackLevelLabel->setColor(Color3B::BLACK);
    m_mainPanel->addChild(m_barrackLevelLabel);

    // Cost上限显示
    m_costLimitLabel = Label::createWithSystemFont("Cost Limit: 10", "Arial", 32);
    m_costLimitLabel->setPosition(Vec2(m_mainPanel->getContentSize().width / 2, infoY - 50));
    m_costLimitLabel->setColor(Color3B::BLUE);
    m_mainPanel->addChild(m_costLimitLabel);

    // 当前Cost使用显示
    m_currentCostLabel = Label::createWithSystemFont("Current Cost: 0/10", "Arial", 32);
    m_currentCostLabel->setPosition(Vec2(m_mainPanel->getContentSize().width / 2, infoY - 100));
    m_currentCostLabel->setColor(Color3B::GREEN);
    m_mainPanel->addChild(m_currentCostLabel);

    // 初始化按钮
    initButtons();

    // 创建并添加招募UI，但不持有其指针
    auto recruitUI = RecruitUI::create();
    if (recruitUI)
    {
        // 给招募UI设置一个唯一的名称，方便后续查找
        recruitUI->setName("RecruitUI_Panel");
        this->addChild(recruitUI);
    }

    // 初始时隐藏整个UI
    this->setVisible(false);

    return true;
}

// 更新军营信息显示
void BarracksUI::updateBarrackInfo()
{
    auto mainScene = dynamic_cast<MainMode*>(Director::getInstance()->getRunningScene());
    if (!mainScene)
    {
        return;
    }

    // 将 Building* 转换为 Barracks*
    auto building = mainScene->getBarracksBuilding();
    auto barracks = dynamic_cast<Barracks*>(building);
    if (!barracks)
    {
        m_barrackLevelLabel->setString("No Barracks Built");
        m_costLimitLabel->setString("");
        m_currentCostLabel->setString("");
        return;
    }

    int level = barracks->getBarrackLevel();
    int currentCost = barracks->getCurrentCostUsed();
    int maxCost = barracks->getMaxCostLimit();

    // 更新显示文本
    m_barrackLevelLabel->setString("Barracks Level: " + std::to_string(level));
    m_costLimitLabel->setString("Cost Limit: " + std::to_string(maxCost));
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

// 显示升级失败对话框
void BarracksUI::showUpgradeFailureDialog(Building* building)
{
    // 传入的参数是 Building*，需要转换为 Barracks* 才能获取军营等级
    auto barracks = dynamic_cast<Barracks*>(building);
    if (!barracks)
    {
        return;
    }

    int level = barracks->getBarrackLevel();
    std::string reason;

    if (level >= 3)
    {
        reason = "Barracks is already at maximum level!";
    }
    else
    {
        // 检查资源不足情况
        int nextLevel = level + 1;
        int upgradeCost = 0;
        int currentGold = GameManager::getInstance()->getGold();
        reason = "Need Gold to Upgrade (Current: " + std::to_string(currentGold) + ")";
    }

    // 创建提示对话框
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto dialogBg = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
    this->addChild(dialogBg, 1000);

    auto dialogPanel = Sprite::create("ShopBackground.png");
    if (dialogPanel)
    {
        dialogPanel->setPosition(visibleSize / 2);
        dialogPanel->setScale(0.5f);
        dialogBg->addChild(dialogPanel);
    }

    auto messageLabel = Label::createWithSystemFont("Cannot Upgrade!\n" + reason, "Arial", 36);
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

// 显示无军营对话框
void BarracksUI::showNoBarracksDialog()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto dialogBg = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
    this->addChild(dialogBg, 1000);

    auto messageLabel = Label::createWithSystemFont("No Barracks Built!\nPlease build a barracks first.", "Arial", 36);
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

// 初始化按钮
void BarracksUI::initButtons()
{
    Size panelSize = m_mainPanel->getContentSize();

    // 招募士兵按钮
    auto recruitBtn = Button::create("btn1.png");
    recruitBtn->setScale(0.3f);
    recruitBtn->setTitleText("Recruit Troops");
    recruitBtn->setTitleFontSize(100);
    recruitBtn->setTitleColor(Color3B::BLACK);
    recruitBtn->setPosition(Vec2(panelSize.width / 2, panelSize.height * 0.7));

    // 通过名称查找子节点来显示UI，避免持有指针
    recruitBtn->addClickEventListener
    (
        [this](Ref*)
        {
            auto recruitUI = this->getChildByName<RecruitUI*>("RecruitUI_Panel");
            if (recruitUI)
            {
                recruitUI->show();
            }
        }
    );
    m_mainPanel->addChild(recruitBtn);

    // 升级军营按钮
    auto upgradeBarracksBtn = Button::create("btn1.png");
    upgradeBarracksBtn->setScale(0.3f);
    upgradeBarracksBtn->setTitleText("Upgrade Barracks");
    upgradeBarracksBtn->setTitleFontSize(100);
    upgradeBarracksBtn->setTitleColor(Color3B::BLACK);
    upgradeBarracksBtn->setPosition(Vec2(panelSize.width / 2, panelSize.height * 0.5));

    upgradeBarracksBtn->addClickEventListener([this](Ref*)
        {
            // 获取当前场景
            auto mainScene = dynamic_cast<MainMode*>(Director::getInstance()->getRunningScene());
            if (mainScene)
            {
                // 获取军营建筑对象
                auto building = mainScene->getBarracksBuilding();
                auto barracks = dynamic_cast<Barracks*>(building);
                if (barracks)
                {
                    // 检查大本营等级限制
                    int max_level = GameManager::getInstance()->getTown_Hall_Level();
                    if (barracks->getBarrackLevel() >= max_level)
                    {
                        // 显示提示
                        auto visibleSize = Director::getInstance()->getVisibleSize();
                        auto scene = Director::getInstance()->getRunningScene();
                        if (scene)
                        {
                            auto alertBg = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
                            scene->addChild(alertBg, 10000);

                            // 吞噬触摸，防止点击穿透
                            auto listener = EventListenerTouchOneByOne::create();
                            listener->setSwallowTouches(true);
                            listener->onTouchBegan = [](Touch*, Event*) { return true; };
                            scene->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, alertBg);

                            auto alertPanel = Sprite::create("barracksBoard.png");
                            if (alertPanel)
                            {
                                alertPanel->setPosition(visibleSize / 2);
                                alertPanel->setScale(1.0f);
                                alertBg->addChild(alertPanel);
                            }

                            std::string msg = "Cannot Upgrade!\nRequire Town Hall Level " + std::to_string(max_level + 1);
                            auto alertLabel = Label::createWithSystemFont(msg, "Arial", 40);
                            alertLabel->setPosition(visibleSize / 2);
                            alertLabel->setTextColor(Color4B::BLACK);
                            alertLabel->setAlignment(TextHAlignment::CENTER);
                            alertBg->addChild(alertLabel);

                            auto okBtn = Button::create("yes.png");
                            okBtn->setScale(0.1f);
                            okBtn->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.4f));
                            okBtn->addClickEventListener([alertBg](Ref*)
                                {
                                    alertBg->removeFromParent();
                                });
                            alertBg->addChild(okBtn);
                        }
                        return;
                    }

                    // 执行升级或显示失败对话框
                    if (barracks->canUpgradeBarrack())
                    {
                        barracks->upgradeBarrack();
                        this->updateBarrackInfo();
                    }
                    else
                    {
                        this->showUpgradeFailureDialog(barracks);
                    }
                }
                else
                {
                    showNoBarracksDialog();
                }
            }
        });
    m_mainPanel->addChild(upgradeBarracksBtn);

    // 升级兵种按钮
    auto upgradeTroopsBtn = Button::create("btn1.png");
    upgradeTroopsBtn->setScale(0.3f);
    upgradeTroopsBtn->setTitleText("Upgrade Troops");
    upgradeTroopsBtn->setTitleFontSize(100);
    upgradeTroopsBtn->setTitleColor(Color3B::BLACK);
    upgradeTroopsBtn->setPosition(Vec2(panelSize.width / 2, panelSize.height * 0.3));

    upgradeTroopsBtn->addClickEventListener
    (
        [this](Ref*)
        {
            auto upgradeTroopUI = this->getChildByName<UpgradeUi*>("UpgradeTroopUI_Panel");
            if (!upgradeTroopUI)
            {
                auto layer = UpgradeUi::create();
                upgradeTroopUI = static_cast<UpgradeUi*>(layer);
                if (upgradeTroopUI)
                {
                    upgradeTroopUI->setName("UpgradeTroopUI_Panel");
                    this->addChild(upgradeTroopUI);
                }
            }
            if (upgradeTroopUI)
            {
                upgradeTroopUI->show();
            }
        }
    );
    m_mainPanel->addChild(upgradeTroopsBtn);

    // 关闭按钮
    auto closeBtn = Button::create("no.png");
    closeBtn->setScale(0.1f);
    closeBtn->setPosition(Vec2(panelSize.width - 350, panelSize.height - 200));

    closeBtn->addClickEventListener
    (
        [this](Ref*)
        {
            this->hide();
        }
    );
    m_mainPanel->addChild(closeBtn);
}

// 显示UI
void BarracksUI::show()
{
    this->setVisible(true);
    // 显示时立即更新军营信息
    updateBarrackInfo();
}

// 进入场景回调
void BarracksUI::onEnter()
{
    Layer::onEnter();

    // 监听军营升级事件
    auto listener = EventListenerCustom::create("EVENT_BARRACK_UPGRADED", [this](EventCustom* event)
        {
            updateBarrackInfo();
        });
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 监听兵种数量变化事件
    auto troopListener = EventListenerCustom::create("EVENT_UPDATE_TROOPS", [this](EventCustom* event)
        {
            updateBarrackInfo();
        });
    _eventDispatcher->addEventListenerWithSceneGraphPriority(troopListener, this);
}

// 隐藏UI
void BarracksUI::hide()
{
    this->setVisible(false);
}

// 更新升级按钮状态
void BarracksUI::updateUpgradeButton()
{
    auto mainScene = dynamic_cast<MainMode*>(Director::getInstance()->getRunningScene());
    if (mainScene)
    {
        auto building = mainScene->getBarracksBuilding();
        auto barracks = dynamic_cast<Barracks*>(building);
        if (barracks)
        {
            int level = barracks->getBarrackLevel();
            int maxCost = barracks->getMaxCostLimit();
            int currentCost = barracks->getCurrentCostUsed();
        }
    }
}