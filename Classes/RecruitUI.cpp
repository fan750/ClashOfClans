// RecruitUI.cpp
#include "RecruitUI.h"
#include "GameManager.h"
#include "GameUI.h" // 为了将来可能的UI联动
#include "ui/CocosGUI.h"
USING_NS_CC;
using namespace ui;

bool RecruitUI::init()
{
    if (!Layer::init()) { return false; }

    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 全屏半透明背景
    this->setColor(Color3B::BLACK);
    this->setOpacity(150);
    this->setContentSize(visibleSize);

    // 2. 主面板
    m_mainPanel = Layout::create();
    m_mainPanel->setContentSize(Size(visibleSize.width * 0.8, visibleSize.height * 0.7));
    m_mainPanel->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    m_mainPanel->setBackGroundColor(Color3B::BLACK);
    m_mainPanel->setBackGroundColorOpacity(255);
    m_mainPanel->setPosition(Vec2(visibleSize.width * 0.1, visibleSize.height * 0.15));
    this->addChild(m_mainPanel);

    // 3. 初始化UI内容
    initUI();

    // 4. 初始隐藏
    this->hide();

    return true;
}

void RecruitUI::initUI()
{
    Size panelSize = m_mainPanel->getContentSize();

    // 标题
    auto titleLabel = Label::createWithSystemFont("Recruit Troops", "Arial", 36);
    titleLabel->setPosition(Vec2(panelSize.width / 2, panelSize.height - 50));
    m_mainPanel->addChild(titleLabel);

    // 定义可招募的兵种
    std::vector<RecruitItem> items = 
    {
        {"Barbarian", TroopType::BARBARIAN, 50, false},   // 50圣水
        {"Archer", TroopType::ARCHER, 100, false},        // 100圣水
        {"Giant", TroopType::GIANT, 250, false},          // 250圣水
        {"Bomberman", TroopType::BOMBERMAN, 100, false} ,  // 100圣水
        {"Dragon",TroopType::DRAGON,300,false}
    };

    // 创建兵种项和招募按钮
    for (int i = 0; i < items.size(); ++i)
    {
        const auto& item = items[i];
        float y = panelSize.height - 150 - i * 80;

        // 兵种名称
        auto nameLabel = Label::createWithSystemFont(item.name, "Arial", 24);
        nameLabel->setAnchorPoint(Vec2(0, 0.5));
        nameLabel->setPosition(Vec2(50, y));
        m_mainPanel->addChild(nameLabel);

        // 招募花费
        std::string costStr = std::to_string(item.cost) + (item.isGoldCost ? " Gold" : " Elixir");
        auto costLabel = Label::createWithSystemFont(costStr, "Arial", 20);
        costLabel->setAnchorPoint(Vec2(0, 0.5));
        costLabel->setColor(item.isGoldCost ? Color3B::YELLOW : Color3B::MAGENTA);
        costLabel->setPosition(Vec2(250, y));
        m_mainPanel->addChild(costLabel);

        // 招募按钮
        auto recruitBtn = Button::create("CloseNormal.png");
        recruitBtn->setTitleText("Recruit");
        recruitBtn->setTitleFontSize(18);
        recruitBtn->setPosition(Vec2(panelSize.width - 120, y));

        // 招募逻辑
        recruitBtn->addClickEventListener
        ([=](Ref*) 
            {
            auto gm = GameManager::getInstance();
            int currentResource = item.isGoldCost ? gm->getGold() : gm->getElixir();

            if (currentResource >= item.cost)
            {
                // 资源足够，进行招募
                CCLOG("Recruiting %s", item.name.c_str());

                // 扣除资源
                if (item.isGoldCost) 
                {
                    gm->addGold(-item.cost);
                }
                else 
                {
                    gm->addElixir(-item.cost);
                }

                // 增加兵种数量
                gm->addTroops(item.type, 1);

                // 分发事件，通知UI更新兵种数量
                Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_TROOPS");

            }
            else 
            {
                // 资源不足
                CCLOG("Not enough resources to recruit %s!", item.name.c_str());
                // 这里可以添加一个“资源不足”的提示
            }
            }
        );
        m_mainPanel->addChild(recruitBtn);
    }

    // 关闭按钮
    auto closeBtn = Button::create("CloseSelected.png");
    closeBtn->setTitleText("X");
    closeBtn->setTitleFontSize(24);
    closeBtn->setPosition(Vec2(panelSize.width - 30, panelSize.height - 30));
    closeBtn->addClickEventListener
    ([this](Ref*) 
        {
        this->hide();
        }
    );
    m_mainPanel->addChild(closeBtn);
}

void RecruitUI::show()
{
    this->setVisible(true);
}

void RecruitUI::hide()
{
    this->setVisible(false);
}

//实现 onExit 函数
void RecruitUI::onExit()
{
    // 确保在节点退出时UI是隐藏的，防止在销毁过程中被点击
    this->hide();

    // 调用父类的 onExit，执行标准的清理流程
    Layer::onExit();
}
// RecruitUI.cpp