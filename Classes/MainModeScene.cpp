//MainModeScene.cpp
#include "MainModeScene.h"
#include "SimpleAudioEngine.h"
#include "GameEntity.h"
#include "Building.h"
#include "Barracks.h"
#include "Troop.h"
#include "GameManager.h"
#include "GameUI.h"
#include "BattleScene.h"
#include "ui/CocosGUI.h"
#include "LevelMapScene.h"
#include"MenuBuilder.h";
#include <chrono>
USING_NS_CC;
using namespace ui;

Scene* MainMode::createScene()
{
    return MainMode::create();
}

static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in MainModeScene.cpp\n");
}

// 获取军营建筑
Building* MainMode::getBarracksBuilding() const {
    for (auto node : m_gameLayer->getChildren()) {
        auto building = dynamic_cast<Building*>(node);
        if (building && building->getBuildingType() == BuildingType::BARRACKS) {
            return building;
        }
    }
    return nullptr;
}

bool MainMode::init() {

    if (!Scene::init()) return false;
    // 只在首次启动时初始化
    if (!GameManager::getInstance()->isInitialized()) {
        GameManager::getInstance()->initAccount(2000, 2000);
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 先初始化数据 (最先做！)
    GameManager::getInstance()->initAccount(2000, 2000);
    m_pendingBuilding = nullptr;
    m_selectedBuilding = nullptr;
    m_isConfirming = false;
    m_confirmLayer = nullptr;

    // 2. 创建游戏容器层 (桌布)
    m_gameLayer = Node::create();
    // m_gameLayer->setPosition(Vec2::ZERO);
    this->addChild(m_gameLayer); // Layer 加到 Scene 上

    // 3. 加载背景图
    auto background = Sprite::create("background1.png");
    if (background) {
        background->setAnchorPoint(Vec2(0, 0));
        background->setPosition(0, 0);
        float scale = (visibleSize.width * 3.0f) / background->getContentSize().width;
        background->setScale(scale);

        m_mapSize = background->getContentSize() * scale;
        m_gameLayer->addChild(background, -99); // 加到 m_gameLayer
        // 公式：Layer位置 = 屏幕中心 - 地图中心
        Vec2 screenCenter = visibleSize / 2;
        Vec2 mapCenter = Vec2(m_mapSize.width / 2, m_mapSize.height / 2);

        // 设置 m_gameLayer 的位置
        m_gameLayer->setPosition(screenCenter - mapCenter);
    }
    else {
        CCLOG("Error: Background image not found!");
        m_mapSize = visibleSize * 2;
    }

    // 4. 恢复/创建 建筑
    const auto& savedBuildings = GameManager::getInstance()->getHomeBuildings();
    if (savedBuildings.empty()) {
        // --- 第一次玩：新建大本营 ---
        auto myTown = Building::create(BuildingType::TOWN_HALL);
        // 放在地图中心，而不是屏幕中心
        Vec2 centerMapPos = Vec2(m_mapSize.width / 2, m_mapSize.height / 2);
        myTown->setPosition(centerMapPos);

        // 加到 m_gameLayer，这样它才会跟着地图动！
        m_gameLayer->addChild(myTown);

        // 立即存档，记录等级信息
        GameManager::getInstance()->addHomeBuilding(BuildingType::TOWN_HALL, centerMapPos, myTown->getLevel());
    }
    else {
        // --- 有存档，恢复建筑 ---
        
        // 重新计算上限：基础值 1000 + 每个存储建筑提供的加成
        int calculatedMaxGold = 1000;
        int calculatedMaxElixir = 1000;

        for (const auto& data : savedBuildings) {
            auto b = Building::create(data.type);
            b->setPosition(data.position);
            b->setOpacity(255);
            // 恢复等级
            b->setLevel(data.level);
            
            // 根据建筑类型和等级累加存储上限
            // 基础等级1提供500，每升一级再加500
            if (data.type == BuildingType::GOLD_STORAGE) {
                calculatedMaxGold += 500 + (data.level - 1) * 500;
            }
            else if (data.type == BuildingType::ELIXIR_STORAGE) {
                calculatedMaxElixir += 500 + (data.level - 1) * 500;
            }

            if (data.type == BuildingType::GOLD_MINE || data.type == BuildingType::ELIXIR_COLLECTOR) {
                b->playWorkAnimation();
            }
            // 恢复军营等级
            if (data.type == BuildingType::BARRACKS) {
                // 将基类指针转换为 Barracks 指针
                auto barracks = dynamic_cast<Barracks*>(b);
                if (barracks) {
                    // 调用 Barracks 特有的方法恢复军营功能等级
                    barracks->setBarrackLevel(data.barrackLevel);

                }
            }
            b->activateBuilding(); 
            m_gameLayer->addChild(b); 

            // 检查该建筑是否正在进行中的升级任务
            if (GameManager::getInstance()->hasPendingUpgrade(data.type, data.position))
            {
                b->showConstructionAnimation();
            }
        }
        
        // 应用计算出的上限
        GameManager::getInstance()->modifyMaxGold(calculatedMaxGold);
        GameManager::getInstance()->modifyMaxElixir(calculatedMaxElixir);
    }

    // 2. 加载 UI 层 (之前的代码)
    auto uiLayer = GameUI::create();
    this->addChild(uiLayer, 100);
    Sprite* music_sound = Sprite::create("sound.png");
    Sprite* music_off = Sprite::create("sound_off.png");
    music_sound->setScale(0.3f);
    music_off->setScale(0.3f);
    auto menu_music = MenuBuilder::createToggleMenu("sound.png", "sound_off.png",
        [&](Ref* ref) {
            auto item = dynamic_cast<MenuItemToggle*>(ref);
            if (item) {
                if (item->getSelectedIndex() == 0) {
                    auto engine = CocosDenshion::SimpleAudioEngine::getInstance();
                    engine->playBackgroundMusic("backgroundmusic.mp3", true);
                }
                else {
                    auto engine = CocosDenshion::SimpleAudioEngine::getInstance();
                    engine->stopBackgroundMusic();
                }
            }
        },
        Vec2(visibleSize.width * 0.05f, visibleSize.height * 0.9f),
        0.3f);
    uiLayer->addChild(menu_music);//添加音乐菜单
    // 3. 商店按钮 (放在左下角)
    auto shopBtn = Button::create("shop_icon.png");
    shopBtn->setScale(0.1f);
    shopBtn->setTitleText("SHOP");
    shopBtn->setTitleFontSize(24);
    shopBtn->setPosition(Vec2(visibleSize.width - 350, 125));
    shopBtn->addClickEventListener([=](Ref*) {
        this->toggleShop(); // 点击开关商店
        });
    this->addChild(shopBtn, 10);


    // 4. 初始化商店面板和加速面板(默认隐藏)
    initShopUI();
    initTime();

    // 5. 进攻按钮
    auto attackBtn = Button::create("attack_icon.png");
    attackBtn->setScale(0.15f);
    attackBtn->setPosition(Vec2(visibleSize.width * 0.92f, visibleSize.height * 0.1f));
    attackBtn->addClickEventListener
    ([=](Ref*)
        {
            // 检查兵力
            auto gm = GameManager::getInstance();
            int totalTroops = 
                gm->getTroopCount(TroopType::BARBARIAN) +
                gm->getTroopCount(TroopType::ARCHER) +
                gm->getTroopCount(TroopType::GIANT) +
                gm->getTroopCount(TroopType::BOMBERMAN) +
                gm->getTroopCount(TroopType::DRAGON);

            if (totalTroops == 0)
            {
                // 没有兵力，显示提示并返回
                CCLOG("Cannot enter battle without any troops!");

                // 创建一个简单的提示框
                auto visibleSize = Director::getInstance()->getVisibleSize();
                auto alertBg = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
                this->addChild(alertBg, 1000);

                auto alertPanel = Sprite::create("barracksBoard.png"); // 复用一下背景图
                if (alertPanel) {
                    alertPanel->setPosition(visibleSize / 2);
                    alertPanel->setScale(1.0f);
                    alertBg->addChild(alertPanel);
                }

                auto alertLabel = Label::createWithSystemFont("You have no troops!\nGo to the Barracks to recruit some!", "Arial", 46);
                alertLabel->setPosition(visibleSize / 2);
                alertLabel->setTextColor(Color4B::BLACK);
                alertLabel->setAlignment(TextHAlignment::CENTER);
                alertBg->addChild(alertLabel);

                auto okBtn = Button::create("yes.png");
                okBtn->setScale(0.1f);
                okBtn->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.4f));
                okBtn->addClickEventListener
                ([alertBg](Ref*) 
                    {
                    alertBg->removeFromParent();
                    }
                );
                alertBg->addChild(okBtn);

                return; // 阻止跳转
            }

            // 有兵力，正常跳转到关卡选择
            auto scene = LevelMapScene::createScene();
            Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        }
    );
    this->addChild(attackBtn);

    // 初始化军营UI，并添加到场景中，但默认隐藏
    m_barracksUI = BarracksUI::create();
    this->addChild(m_barracksUI, 300); // 确保层级最高
    //6.加速按钮
    TimeBtn = Button::create("time_icon.png");
    TimeBtn->setScale(0.7f);
    TimeBtn->setTitleText("Accelerate");
    TimeBtn->setTitleFontSize(36);
    TimeBtn->setPosition(Vec2(visibleSize.width - 750, 125));
    TimeBtn->addClickEventListener([=](Ref*) {
        if (!TimeBtn->isEnabled()) return;
        this->toggleTime();
        });
    this->addChild(TimeBtn, 10);
    updateTimeButtonCooldown();
    // 7. 触摸监听 (保持不变)
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = CC_CALLBACK_2(MainMode::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(MainMode::onTouchMoved, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

void MainMode::initShopUI() {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 创建商店背景板 
    m_shopLayer = Layout::create();

    m_shopLayer->setBackGroundImage("ShopBackground.png");

    // 设置面板的大小 
    m_shopLayer->setContentSize(Size(visibleSize.width * 0.8, visibleSize.height * 0.6));
    // 创建好 layout 后，直接设置整体缩放
    m_shopLayer->setScale(0.8f); // 缩小到 80%

    // 居中显示
    m_shopLayer->setAnchorPoint(Vec2(0.5, 0.5));
    m_shopLayer->setPosition(visibleSize / 2); // 直接放在屏幕正中间

    m_shopLayer->setVisible(false); // 默认看不见
    this->addChild(m_shopLayer, 200); // 层级很高，盖住一切

    // 2. 定义商品列表 (增加对应的图标路径)
    std::vector<ShopItem> items = {
        {"Gold Mine", BuildingType::GOLD_MINE, 100, false, "gold_anim_0.png"},
        {"Cannon", BuildingType::CANNON, 200, true, "Cannon.png"},
        {"Wall", BuildingType::WALL, 50, true, "Wall.png"},
        {"Archer Twr", BuildingType::ARCHER_TOWER, 300, true, "ArcherTower.png"},
        {"Elixir Pump", BuildingType::ELIXIR_COLLECTOR, 100, true, "elixir_anim_2.png"},
        {"Elixir Tank", BuildingType::ELIXIR_STORAGE, 300, true, "ElixirStorage.png"},
        {"Gold Storage", BuildingType::GOLD_STORAGE, 300, false, "GoldStorage.png"},
        {"Barracks", BuildingType::BARRACKS, 500, true, "Barracks.png"}
    };

    // 面板高度 400
    float row1_Y = 480; // 第一排的高度 (靠上)
    float row2_Y = 40; // 第二排的高度 (靠下)

    // 起始 X 坐标和间距
    float start_X = 380;
    float gap_X = 100;

    // --- 第一排 (Row 1) ---
    // 0: 金矿
    createShopItemButton(items[0], Vec2(start_X + 0 * gap_X, row1_Y), 0.3f);
    // 1: 加农炮
    createShopItemButton(items[1], Vec2(start_X + 3 * gap_X, row1_Y), 0.6f);
    // 2: 围墙
    createShopItemButton(items[2], Vec2(start_X + 6 * gap_X, row1_Y), 1.3f);
    // 3: 箭塔
    createShopItemButton(items[3], Vec2(start_X + 9 * gap_X, row1_Y), 0.6f);

    // --- 第二排 (Row 2) ---
    // 4: 收集器
    createShopItemButton(items[4], Vec2(start_X + 3 * gap_X, row2_Y), 0.3f);
    // 5: 圣水瓶
    createShopItemButton(items[5], Vec2(start_X + 6 * gap_X, row2_Y), 0.3f);
    // 6: 储金罐
    createShopItemButton(items[6], Vec2(start_X + 9 * gap_X, row2_Y), 0.3f);
    // 7: 军营
    createShopItemButton(items[7], Vec2(start_X + 0 * gap_X, row2_Y), 0.6f);
}

void MainMode::toggleShop() {
    // 切换显示状态
    m_shopLayer->setVisible(!m_shopLayer->isVisible());
}

void MainMode::initTime() {
    Size visibleSize = Director::getInstance()->getVisibleSize();
    m_timeLayer = Layout::create();
    m_timeLayer->setBackGroundImage("accelerate_background.png");
    m_timeLayer->setContentSize(Size(visibleSize.width * 0.8, visibleSize.height * 0.6));
    // 创建好 layout 后，直接设置整体缩放
    m_timeLayer->setScale(0.8f); // 缩小到 80%

    // 居中显示
    m_timeLayer->setAnchorPoint(Vec2(0.5, 0.5));
    m_timeLayer->setPosition(visibleSize / 2); // 直接放在屏幕正中间

    m_timeLayer->setVisible(false); // 默认看不见

    auto time_btn = Button::create("accelerate.png");
    time_btn->setScale(0.2f);
    time_btn->setPosition(Vec2(visibleSize.width * 0.4f, visibleSize.height * 0.1f));
    m_accelerateBtn = time_btn;
    updateTimeButtonCooldown();
    time_btn->addClickEventListener([=](Ref*) {
        m_timeLayer->setVisible(false);
        auto gm = GameManager::getInstance();
        auto now = std::chrono::steady_clock::now();
        gm->setTimeAccelerateCooldownEnd(now + std::chrono::seconds(60));
        updateTimeButtonCooldown();

        for (auto node : m_gameLayer->getChildren()) {
            if (auto building = dynamic_cast<Building*>(node)) {
                auto type = building->getBuildingType();
                if (type == BuildingType::GOLD_MINE || type == BuildingType::ELIXIR_COLLECTOR) {
                    building->applyProductionBoost(2.0f, 30.0f);
                }
                // 如果是正在升级的建筑，应用施工加速
                building->applyConstructionBoost(2.0f, 20.0f);
            }
        }
        });
    m_timeLayer->addChild(time_btn);
    this->addChild(m_timeLayer, 201); // 层级很高，盖住一切
}
void MainMode::toggleTime() {
    m_timeLayer->setVisible(!m_timeLayer->isVisible());
}
void MainMode::updateTimeButtonCooldown()
{
    auto gm = GameManager::getInstance();
    float remaining = gm->getTimeAccelerateCooldownRemaining();
    this->unschedule("AccelerateCooldown");
    bool onCooldown = (remaining > 0.0f);

    if (TimeBtn)
    {
        TimeBtn->setEnabled(!onCooldown);
        TimeBtn->setBright(!onCooldown);
    }
    if (m_accelerateBtn)
    {
        m_accelerateBtn->setEnabled(!onCooldown);
        m_accelerateBtn->setBright(!onCooldown);
    }

    if (onCooldown)
    {
        this->scheduleOnce([this](float) {
            GameManager::getInstance()->setTimeAccelerateCooldownEnd(std::chrono::steady_clock::now());
            updateTimeButtonCooldown();
            }, remaining, "AccelerateCooldown");
    }
}
void MainMode::tryBuyBuilding(const ShopItem& item) {
    // 检查军营数量限制
    if (item.type == BuildingType::BARRACKS) {
        // 遍历现有建筑，检查是否已有军营
        bool hasBarracks = false;
        if (m_gameLayer) {
            for (auto node : m_gameLayer->getChildren()) {
                auto building = dynamic_cast<Building*>(node);
                if (building && building->getBuildingType() == BuildingType::BARRACKS) {
                    hasBarracks = true;
                    break;
                }
            }
        }
        
        if (hasBarracks) {
            CCLOG("Already have a Barracks!");
            // 弹出提示
            auto visibleSize = Director::getInstance()->getVisibleSize();
            auto alertBg = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
            this->addChild(alertBg, 1000);

            auto alertPanel = Sprite::create("barracksBoard.png");
            if (alertPanel) {
                alertPanel->setPosition(visibleSize / 2);
                alertPanel->setScale(1.0f);
                alertBg->addChild(alertPanel);
            }

            auto alertLabel = Label::createWithSystemFont("You can only build ONE Barracks!", "Arial", 40);
            alertLabel->setPosition(visibleSize / 2);
            alertLabel->setTextColor(Color4B::BLACK);
            alertLabel->setAlignment(TextHAlignment::CENTER);
            alertBg->addChild(alertLabel);

            auto okBtn = Button::create("yes.png");
            okBtn->setScale(0.1f);
            okBtn->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.4f));
            okBtn->addClickEventListener([alertBg](Ref*) {
                alertBg->removeFromParent();
            });
            alertBg->addChild(okBtn);
            
            toggleShop(); // 关闭商店
            return;
        }
    }

    // 1. 检查钱够不够
    int currentRes = item.isGold ? GameManager::getInstance()->getGold() : GameManager::getInstance()->getElixir();

    if (currentRes < item.price) {
        CCLOG("Not enough resources!");
        return;
    }
    toggleShop();
    // 3. 创建虚影
    if (m_pendingBuilding) {
        m_pendingBuilding->removeFromParent();
        m_pendingBuilding = nullptr; // 确保置空
    }

    m_pendingBuilding = Building::create(item.type);

    // 检查创建是否成功
    if (!m_pendingBuilding) {
        CCLOG("ERROR: Failed to create building of type %d", (int)item.type);
        // 可以显示一个错误提示给用户
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto errorLabel = Label::createWithSystemFont("Failed to create building!", "Arial", 36);
        errorLabel->setColor(Color3B::RED);
        errorLabel->setPosition(visibleSize / 2);
        this->addChild(errorLabel, 1000);

        // 3秒后自动移除错误提示
        this->scheduleOnce([=](float) {
            errorLabel->removeFromParent();
            }, 3.0f, "error_msg");

        return;
    }

    m_pendingBuilding->setOpacity(128);

    // 4. 加到地图层
    if (m_gameLayer) {  // 也检查 gameLayer 是否存在
        m_gameLayer->addChild(m_pendingBuilding);
    }
    else {
        CCLOG("ERROR: m_gameLayer is null!");
        delete m_pendingBuilding;
        m_pendingBuilding = nullptr;
        return;
    }

    // 5. 设置初始位置 (屏幕中心 -> 转换到地图坐标)
    Vec2 centerScreen = Director::getInstance()->getVisibleSize() / 2;
    Vec2 worldPos = m_gameLayer->convertToNodeSpace(centerScreen);
    m_pendingBuilding->setPosition(worldPos);

    // 6. 记录价格
    m_pendingCost = item.price;
    m_pendingIsGold = item.isGold;
}

void MainMode::onTouchMoved(Touch* touch, Event* event) {
    // 情况A: 正在拖拽放置建筑
    if (m_pendingBuilding) {
        // 1. 获取屏幕触摸点
        Vec2 screenPos = touch->getLocation();

        // 2. 转换为地图坐标
        Vec2 worldPos = m_gameLayer->convertToNodeSpace(screenPos);

        // 3. 建筑跟随鼠标
        m_pendingBuilding->setPosition(worldPos);

        // 让按钮也跟着走！
        if (m_confirmLayer) {
            m_confirmLayer->setPosition(worldPos);
        }

        return;
    }
    // --- 地图拖拽逻辑 ---
    if (m_gameLayer)
    {
        // 1. 计算手指移动了多少
        Vec2 currentTouchPos = touch->getLocation();
        Vec2 delta = currentTouchPos - m_lastTouchPos;

        // 2. 计算目标位置
        Vec2 newPos = m_gameLayer->getPosition() + delta;

        // 3. 边界限制 (Clamping)
        Size visibleSize = Director::getInstance()->getVisibleSize();

        // X轴限制：在 [-(地图宽-屏幕宽), 0] 之间
        float minX = -(m_mapSize.width - visibleSize.width);
        float maxX = 0;

        // Y轴限制：在 [-(地图高-屏幕高), 0] 之间
        float minY = -(m_mapSize.height - visibleSize.height);
        float maxY = 0;

        // 如果地图比屏幕还小，就锁死在 0 (或者居中)
        if (m_mapSize.width <= visibleSize.width) {
            minX = 0; maxX = 0;
        }
        if (m_mapSize.height <= visibleSize.height) {
            minY = 0; maxY = 0;
        }

        // 执行限制
        newPos.x = std::max(minX, std::min(newPos.x, maxX));
        newPos.y = std::max(minY, std::min(newPos.y, maxY));

        // 4. 应用位置
        m_gameLayer->setPosition(newPos);

        // 5. 更新记录点
        m_lastTouchPos = currentTouchPos;
    }
}

// 手指按下时
bool MainMode::onTouchBegan(Touch* touch, Event* event) {
    // 记录一下初始位置，供地图拖拽计算用
    m_lastTouchPos = touch->getLocation();

    // 如果正在放置建筑
    if (m_pendingBuilding) {
        // 1. 同样要进行坐标转换
        Vec2 screenPos = touch->getLocation();
        Vec2 worldPos = m_gameLayer->convertToNodeSpace(screenPos);

        // 2. 设置最终确认的位置
        m_pendingBuilding->setPosition(worldPos);

        // 3. 弹出确认框 (使用之前的逻辑)
        m_isConfirming = true;
        showConfirmationUI(worldPos); // 确认框也显示在建筑旁边
        if (m_confirmLayer) {
            m_confirmLayer->setPosition(worldPos);
        }

        // 如果还没显示按钮，就显示出来
        if (!m_isConfirming) {
            m_isConfirming = true;
            showConfirmationUI(worldPos);
        }

        return true;
    }

    // 如果商店是开着的，点击外部可以关闭商店
    if (m_shopLayer->isVisible()) {
        Vec2 nodePos = m_shopLayer->convertToNodeSpace(touch->getLocation());
        Rect box = Rect(0, 0, m_shopLayer->getContentSize().width, m_shopLayer->getContentSize().height);

        if (!box.containsPoint(nodePos)) {
            toggleShop(); 
        }
    }
    //如果加速窗口开着,点击外部可以关闭
    if (m_timeLayer->isVisible()) {
        Vec2 nodePos = m_timeLayer->convertToNodeSpace(touch->getLocation());
        Rect box = Rect(0, 0, m_timeLayer->getContentSize().width, m_timeLayer->getContentSize().height);

        if (!box.containsPoint(nodePos)) {
            toggleTime();
        }
    }

    // 1. 获取屏幕坐标
    Vec2 screenPos = touch->getLocation();
    // 2. 转换为地图坐标！
    // 所有的建筑都在 m_gameLayer 上，所以必须用在这个坐标系下的点去判断
    Vec2 worldPos = m_gameLayer->convertToNodeSpace(screenPos);

    bool hitBuilding = false;

    // 检查是否点了军营 (使用 worldPos)
    if (!m_pendingBuilding && !m_isConfirming && !m_shopLayer->isVisible()) {
        // 修改 getBarracksAtPosition，让它接收 worldPos (见下一步)
        auto clickedBarracks = getBarracksAtPosition(worldPos);
        if (clickedBarracks) {
            CCLOG("Barracks clicked!");
            m_barracksUI->show();
            return true;
        }
    }

    for (auto node : m_gameLayer->getChildren()) {
        auto building = dynamic_cast<Building*>(node);
        if (building) {
            Rect boundingBox = building->getBoundingBox();

            // 使用转换后的 worldPos 进行判断
            if (boundingBox.containsPoint(worldPos)) {

                // 3. 选中建筑
                if (m_selectedBuilding != building) {
                    // 如果正在施工，则不选中（不显示升级按钮等）
                    if (building->isUpgrading()) {
                        CCLOG("Building is upgrading, cannot select.");
                        hitBuilding = true;
                        return true;
                    }

                    selectBuilding(building);
                }

                // 收集资源
                int amount = building->collectResource();
                if (amount > 0) {
                    playCollectAnimation(amount, building->getPosition(), building->getBuildingType());
                }

                hitBuilding = true;
                return true;
            }
        }
    }

    // 如果没有点中任何建筑，取消选中
    if (!hitBuilding) {
        if (m_selectedBuilding) {
            m_selectedBuilding->hideUpgradeButton();
            m_selectedBuilding = nullptr;
        }
    }

    return true;
}

void MainMode::selectBuilding(Building* building) {
    if (!building) return;

    // 如果已经选中了别的，先隐藏它的升级按钮
    if (m_selectedBuilding && m_selectedBuilding != building) {
        m_selectedBuilding->hideUpgradeButton();
    }

    m_selectedBuilding = building;
    // 当选中建筑时，根据当前金币自动显示或隐藏升级按钮
    m_selectedBuilding->updateUpgradeButtonVisibility();
}

// ...existing code...

void MainMode::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
}

// 显示 √ 和 × 按钮
void MainMode::showConfirmationUI(Vec2 pos) {
    if (m_confirmLayer) {
        m_confirmLayer->removeFromParent();
        m_confirmLayer = nullptr;
    }
    m_confirmLayer = Node::create();

    // 1. 把容器层加到地图上
    m_gameLayer->addChild(m_confirmLayer, 200);

    // 2. 容器层直接定位于建筑的位置
    m_confirmLayer->setPosition(pos);

    // --- 1. 确定按钮 (绿色) ---
    auto btnYes = Button::create("yes.png");
    btnYes->setScale(0.1f);

    // 位置改为相对坐标 (相对于中心点向右偏移 60)
    btnYes->setPosition(Vec2(80, 0));

    btnYes->addClickEventListener([=](Ref*) { this->onConfirmPlacement(); });
    m_confirmLayer->addChild(btnYes);

    // --- 2. 取消按钮 (红色) ---
    auto btnNo = Button::create("no.png");
    btnNo->setScale(0.07f);
    // 放在 pos 的左边
    btnNo->setPosition(Vec2(-80, 0));

    btnNo->addClickEventListener([=](Ref*) { this->onCancelPlacement(); });
    m_confirmLayer->addChild(btnNo);
}

// 玩家点了 YES
void MainMode::onConfirmPlacement() {
    if (!m_pendingBuilding) return;

    // 1. 恢复实体不透明度
    m_pendingBuilding->setOpacity(255);
    m_pendingBuilding->playWorkAnimation();

    // 2. 扣钱
    if (m_pendingIsGold) {
        GameManager::getInstance()->addGold(-m_pendingCost);
    }
    else {
        GameManager::getInstance()->addElixir(-m_pendingCost);
    }

    // 3. 存档 (保存到 GameManager) - 记录等级信息
    GameManager::getInstance()->addHomeBuilding(
        m_pendingBuilding->getBuildingType(),
        m_pendingBuilding->getPosition(),
        m_pendingBuilding->getLevel()
    );

    // 4. 如果是军营，手动触发 Cost 更新事件
    if (m_pendingBuilding->getBuildingType() == BuildingType::BARRACKS) {
        auto barracks = dynamic_cast<Barracks*>(m_pendingBuilding);
        if (barracks) {
            // 确保在放置确认后，军营被正确初始化并更新 Cost
            barracks->updateCurrentCostUsed();
        }
    }

    // 5.如果是储存相关建筑,增加上限
    if (m_pendingBuilding->getBuildingType() == BuildingType::GOLD_STORAGE) {
        int current_max = GameManager::getInstance()->getMaxGold();
        GameManager::getInstance()->modifyMaxGold(current_max + 500);
    }
    if (m_pendingBuilding->getBuildingType() == BuildingType::ELIXIR_STORAGE) {
        int current_max = GameManager::getInstance()->getMaxElixir();
        GameManager::getInstance()->modifyMaxElixir(current_max + 500);
    }

    // 6. 清理现场
    m_pendingBuilding = nullptr; // 指针置空，表示放置结束

    if (m_confirmLayer) {
        m_confirmLayer->removeFromParent();
        m_confirmLayer = nullptr;
    }

    // 退出确认状态
    m_isConfirming = false;

    CCLOG("Placed Successfully!");
}

// 玩家点了 NO
void MainMode::onCancelPlacement() {
    // 1. 移除那个虚影建筑
    if (m_pendingBuilding) {
        m_pendingBuilding->removeFromParent();
        m_pendingBuilding = nullptr;
    }

    // 2. 移除确认按钮
    if (m_confirmLayer) {
        m_confirmLayer->removeFromParent();
        m_confirmLayer = nullptr;
    }

    // 3. 退出确认状态
    m_isConfirming = false;

    CCLOG("Placement Cancelled!");
}

// 传入参数已经是转换好的 worldPos
Building* MainMode::getBarracksAtPosition(Vec2 worldPos) {
    // 遍历 m_gameLayer 的子节点，而不是 this
    for (auto child : m_gameLayer->getChildren())
    {
        auto building = dynamic_cast<Building*>(child);
        if (building && building->getBuildingType() == BuildingType::BARRACKS) {
            // 直接判断
            if (building->getBoundingBox().containsPoint(worldPos))
            {
                return building;
            }
        }
    }
    return nullptr;
}

void MainMode::playCollectAnimation(int amount, Vec2 startPos, BuildingType type) {
    // 1. 决定飞什么图标 (金币还是圣水)
    std::string iconName = (type == BuildingType::GOLD_MINE) ? "coin_icon.png" : "elixir_icon.png";
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 targetPos = Vec2(visibleSize.width - 100, visibleSize.height - 30);
    Vec2 screenStartPos = m_gameLayer->convertToWorldSpace(startPos);

    auto icon = Sprite::create(iconName);
    icon->setScale(0.03);

    icon->setPosition(screenStartPos);
    this->addChild(icon, 200); // 层级要在最上面

    // 3. 飘字 (显示 +100)
    auto label = Label::createWithSystemFont("+" + std::to_string(amount), "Arial", 24);
    label->setColor(Color3B::WHITE);
    label->enableOutline(Color4B::BLACK, 2);
    label->setPosition(startPos + Vec2(0, 40)); // 在建筑头顶
    this->addChild(label, 201);

    // 4. 动作序列
    // 图标动作：先放大弹出 -> 飞向右上角 -> 缩小消失
    auto jump = JumpBy::create(0.3f, Vec2(0, 0), 30, 1); // 原地跳一下
    auto move = MoveTo::create(0.5f, targetPos); // 飞过去
    auto scale = ScaleTo::create(0.1f, 0.0f); // 变没
    auto remove = RemoveSelf::create();

    icon->runAction(Sequence::create(jump, move, scale, remove, nullptr));

    // 文字动作：向上飘 -> 淡出
    auto labelMove = MoveBy::create(0.8f, Vec2(0, 50));
    auto fade = FadeOut::create(0.8f);
    auto labelSeq = Sequence::create(Spawn::create(labelMove, fade, nullptr), RemoveSelf::create(), nullptr);
    label->runAction(labelSeq);

}

// 这是一个通用的“造按钮”工厂
void MainMode::createShopItemButton(const ShopItem& item, Vec2 pos, float iconScale) {
    // 1. 创建统一底座 (相框)
    auto btn = Button::create("ItemFrame.png");
    btn->setScale(0.4f);
    btn->setPosition(pos);

    // 2. 添加专属图标
    auto icon = Sprite::create(item.iconPath);
    if (icon) {
        icon->setPosition(btn->getContentSize() / 2);
        // 这里使用参数 iconScale，而不是写死的数字
        icon->setScale(iconScale);
        btn->addChild(icon);
    }

    // 3. 添加价格标签
    std::string currencyStr = item.isGold ? "Gold: " : "Elixir: ";
    auto priceLabel = Label::createWithSystemFont(currencyStr + std::to_string(item.price), "Arial", 14);
    // 放在底座下方外部，或者底座内部下方，这里放在底座内部靠下
    priceLabel->setPosition(Vec2(btn->getContentSize().width / 2, 80));
    // 如果底座颜色深，字体用白色；如果底座浅，用黑色
    priceLabel->setColor(Color3B::BLACK);
    priceLabel->setScale(5);
    auto name = Label::createWithSystemFont(item.name, "Arial", 18);
    name->setPosition(Vec2(btn->getContentSize().width / 2, 160));
    name->setColor(Color3B::BLACK);
    name->setScale(5);

    btn->addChild(priceLabel);
    btn->addChild(name);

    // 4. 绑定点击事件
    btn->addClickEventListener([=](Ref*) {
        this->tryBuyBuilding(item);
        });

    // 5. 加到商店面板
    m_shopLayer->addChild(btn);
}