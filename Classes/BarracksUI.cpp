// BarracksUI.cpp
#include "BarracksUI.h"
#include "RecruitUI.h"
USING_NS_CC;
using namespace ui;

bool BarracksUI::init()
{
    if (!Layer::init())
    {
        return false;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();

    
    // 2. 创建主面板
    m_mainPanel = Layout::create();
    m_mainPanel->setBackGroundImage("barracksBoard.png");
    m_mainPanel->setContentSize(Size(visibleSize.width * 0.8, visibleSize.height * 0.6));
    m_mainPanel->setScale(1.3f);
    m_mainPanel->setAnchorPoint(Vec2(0.5, 0.5));
    m_mainPanel->setPosition(visibleSize / 2); // 直接放在屏幕正中间
    this->addChild(m_mainPanel);

    // 3. 初始化按钮
    initButtons(); // 添加这行，确保按钮被初始化

    // 4. 创建并添加招募UI，但不持有其指针
    auto recruitUI = RecruitUI::create();
    if (recruitUI)
    {
        // 给招募UI设置一个唯一的名称，方便后续查找
        recruitUI->setName("RecruitUI_Panel");
        this->addChild(recruitUI);
    }

    // 5. 初始时隐藏整个UI
    this->setVisible(false);

    return true;
}

void BarracksUI::initButtons()
{
    Size panelSize = m_mainPanel->getContentSize();

    // 招募士兵按钮
    auto recruitBtn = Button::create("btn1.png");
    recruitBtn->setScale(0.3f);
    recruitBtn->setTitleText("Recruit Troops");
    recruitBtn->setTitleFontSize(100);
    recruitBtn->setTitleColor(Color3B::BLACK);
    recruitBtn->setPosition(Vec2(panelSize.width / 2, panelSize.height*0.7 ));
    // 通过名称查找子节点来显示UI，避免持有指针
    recruitBtn->addClickEventListener
    ([this](Ref*)
        {
            CCLOG("Recruit Troops button clicked.");
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
    upgradeBarracksBtn->addClickEventListener
    ([](Ref*)
        {
            CCLOG("Upgrade Barracks button clicked (functionality not implemented).");
        }
    );
    m_mainPanel->addChild(upgradeBarracksBtn);

    // 升级兵种按钮
    auto upgradeTroopsBtn = Button::create("btn1.png");
    upgradeTroopsBtn->setScale(0.3f);
    upgradeTroopsBtn->setTitleText("Upgrade Troops");
    upgradeTroopsBtn->setTitleFontSize(100);
    upgradeTroopsBtn->setTitleColor(Color3B::BLACK);
    upgradeTroopsBtn->setPosition(Vec2(panelSize.width / 2, panelSize.height * 0.3));
    upgradeTroopsBtn->addClickEventListener
    ([](Ref*)
        {
            CCLOG("Upgrade Troops button clicked (functionality not implemented).");
        }
    );
    m_mainPanel->addChild(upgradeTroopsBtn);

    // 关闭按钮
    auto closeBtn = Button::create("no.png"); // 使用一个不同的图片以示区别
    closeBtn->setScale(0.1f);
    closeBtn->setPosition(Vec2(panelSize.width - 350, panelSize.height-200));
    closeBtn->addClickEventListener
    ([this](Ref*)
        {
            this->hide();
        }
    );
    m_mainPanel->addChild(closeBtn);
}

void BarracksUI::show()
{
    this->setVisible(true);
}

void BarracksUI::hide()
{
    this->setVisible(false);
}