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

    
    // 2. 主面板
    m_mainPanel = Layout::create();
    m_mainPanel->setBackGroundImage("barracksBoard.png");
    m_mainPanel->setContentSize(Size(visibleSize.width * 0.8, visibleSize.height * 0.6));
    m_mainPanel->setScale(1.8f);
    m_mainPanel->setAnchorPoint(Vec2(0.5, 0.5));
    m_mainPanel->setPosition(visibleSize / 2); // 直接放在屏幕正中间
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

    // 1. 标题 (保持不变)
    auto titleLabel = Label::createWithSystemFont("Recruit Troops", "Arial", 36);
    // 放在顶部靠下的位置
    titleLabel->setPosition(Vec2(panelSize.width / 2, panelSize.height - 80));
    titleLabel->setColor(Color3B::BLACK); // 建议设为黑色，防止背景太亮看不清
    m_mainPanel->addChild(titleLabel);

    // 2. 定义可招募的兵种数据
    std::vector<RecruitItem> items = {
        // 名字,       类型,                 价格, 锁住,  图片路径
        {"Barbarian", TroopType::BARBARIAN, 50,  false, "barbarian_icon.png"},
        {"Archer",    TroopType::ARCHER,    100, false, "archer_icon.png"},
        {"Giant",     TroopType::GIANT,     250, false, "giant_icon.png"},
        {"Bomberman", TroopType::BOMBERMAN, 100, false, "bomberman_icon.png"},
        {"Dragon",    TroopType::DRAGON,    300, false, "barbarian_icon.png"}
    };

    // 3. 【核心修改】计算网格布局并调用 createRecruitItemButton
    // 现在的按钮比较大，不能像列表一样排，建议一行排 3 个
    int columns = 4;

    // 起始位置参数 (根据你的面板大小微调)
    float startX = panelSize.width * 0.32f; // 从左边 32% 处开始
    float startY = panelSize.height * 0.63f; // 从高度 63% 处开始 (第一排)
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

        // 【关键】直接调用封装好的函数！
        // 所有的创建逻辑、点击逻辑都在这个函数里完成了
        createRecruitItemButton(items[i], Vec2(x, y), m_mainPanel);
    }

    // 4. 关闭按钮 (建议放在右上角，不要挡住兵种按钮)
    auto closeBtn = Button::create("no.png");
    closeBtn->setScale(0.1f);
    closeBtn->setPosition(Vec2(panelSize.width - 350, panelSize.height - 200));
    closeBtn->addClickEventListener([this](Ref*) {
        this->hide();
        });
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

void RecruitUI::createRecruitItemButton(const RecruitItem& item, Vec2 pos, Node* parentNode) 
{
    // 1. 创建底座
    auto btn = ui::Button::create("ItemFrame.png");
    btn->setScale(0.2f);
    btn->setPosition(pos);

    // 2. 添加图标 (直接使用 item.iconPath，无需辅助函数！)
    auto icon = Sprite::create(item.iconPath);
    if (icon) {
        icon->setPosition(btn->getContentSize() / 2);
        icon->setScale(0.35f); // 假设你的兵种图比较小，稍微放大点
        btn->addChild(icon);
    }

    // 3. 添加价格标签
    auto priceLabel = Label::createWithSystemFont("Elixir: " + std::to_string(item.cost), "Arial", 14);
    priceLabel->setPosition(Vec2(btn->getContentSize().width / 2, 80));
    priceLabel->setColor(Color3B::BLACK);
    priceLabel->setScale(5);
    btn->addChild(priceLabel);

    // 4. 添加名称标签
    auto nameLabel = Label::createWithSystemFont(item.name, "Arial", 18);
    nameLabel->setPosition(Vec2(btn->getContentSize().width / 2, 160));
    nameLabel->setColor(Color3B::BLACK);
    nameLabel->setScale(5);
    btn->addChild(nameLabel);

    // 5. 绑定点击事件
    btn->addClickEventListener([=](Ref*) {
        // 【修改】补全业务逻辑
        auto gm = GameManager::getInstance();

        // 1. 检查资源 (注意：结构体里你用的是 cost 和 isGoldCost)
        int currentResource = item.isGoldCost ? gm->getGold() : gm->getElixir();

        if (currentResource >= item.cost)
        {
            CCLOG("Recruiting %s", item.name.c_str());

            // 2. 扣钱
            if (item.isGoldCost) {
                gm->addGold(-item.cost);
            }
            else {
                gm->addElixir(-item.cost);
            }

            // 3. 加兵
            gm->addTroops(item.type, 1);

            // 4. 通知刷新
            Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_UPDATE_TROOPS");
        }
        else
        {
            CCLOG("Not enough resources!");
            // (可选) 可以在这里加个飘字提示
        }
        });
    // 6. 加到父节点
    if (parentNode) {
        parentNode->addChild(btn);
    }
}