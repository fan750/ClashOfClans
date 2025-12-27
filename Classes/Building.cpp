// Building.cpp
#include "Building.h"
#include "BattleManager.h"
#include "GameManager.h"
#include "SimpleAudioEngine.h"
#include "TownHall.h"      // 需要包含所有具体子类头文件以供工厂创建
#include "GoldMine.h"
#include "ElixirCollector.h"
#include "Cannon.h"
#include "ArcherTower.h"
#include "Wall.h"
#include "GoldStorage.h"
#include "ElixirStorage.h"
#include "Barracks.h"
#include "Trap.h"

USING_NS_CC;

Building::Building()
    : GameEntity()
    , m_type(BuildingType::TOWN_HALL) // 默认值，创建时会被覆盖
    , m_level(1)
    , m_timer(0.0f)
    , m_upgradeBtn(nullptr)
    , m_goldListener(nullptr)
    , m_constructionSprite(nullptr)
    , m_baseScale(1.0f)
    , m_productionRate(0)
    , m_maxStorage(0)
    , m_currentStored(0)
    , m_productionAccumulator(0)
    , m_rateMultiplier(1.0f)
    , m_rateBoostTimer(0)
    , m_attackRange(0)
    , m_damage(0)
    , m_attackInterval(0)
    , m_upgradeCheckTimer(0.0f) // 初始化升级检查计时器
{
}

Building::~Building()
{
    if (m_goldListener)
    {
        _eventDispatcher->removeEventListener(m_goldListener);
        m_goldListener = nullptr;
    }
}

Building* Building::create(BuildingType type)
{
    Building* pRet = nullptr;

    // 工厂模式：根据枚举类型实例化对应的子类
    switch (type)
    {
    case BuildingType::TOWN_HALL:       pRet = new TownHall();        break;
    case BuildingType::GOLD_MINE:       pRet = new GoldMine();        break;
    case BuildingType::ELIXIR_COLLECTOR: pRet = new ElixirCollector(); break;
    case BuildingType::CANNON:          pRet = new Cannon();           break;
    case BuildingType::ARCHER_TOWER:    pRet = new ArcherTower();     break;
    case BuildingType::WALL:            pRet = new Wall();            break;
    case BuildingType::GOLD_STORAGE:    pRet = new GoldStorage();     break;
    case BuildingType::ELIXIR_STORAGE:  pRet = new ElixirStorage();   break;
    case BuildingType::BARRACKS:        pRet = new Barracks();        break;
    case BuildingType::TRAP:            pRet = new Trap();            break;
    default: break;
    }

    if (pRet)
    {
        pRet->m_type = type; // 设置类型（虽然构造函数可能已设置，但显式设置更安全）
        if (pRet->init())
        {
            pRet->autorelease();
            return pRet;
        }
    }
    delete pRet;
    return nullptr;
}

bool Building::init()
{
    if (!GameEntity::init())
    {
        return false;
    }

    // 1. 调用子类实现的初始化函数
    // 这里会执行具体子类（如 GoldMine）的 initBuildingProperties
    initBuildingProperties();

    // 2. 初始化血条（通用逻辑）
    // 注意：宽高和偏移现在由子类的 initBuildingProperties 设置
    // 【修改】移到 initBuildingProperties 之后，并缩小尺寸
    if (m_hpBgSprite)
    {
        Size bgSize = m_hpBgSprite->getContentSize();
        if (bgSize.width > 0 && bgSize.height > 0) {
            // 适当缩小血条，乘以 0.5
            float sx = (m_hpBarWidth / 10.0f) * 0.5f;
            float sy = (m_hpBarHeight / 10.0f) * 0.5f;
            m_hpBgSprite->setScale(sx, sy);
        }
    }

    if (m_hpBarTimer && m_hpBarTimer->getSprite())
    {
        Size fillSize = m_hpBarTimer->getSprite()->getContentSize();
        if (fillSize.width > 0 && fillSize.height > 0) {
            // 适当缩小血条，乘以 0.5
            float sx = (m_hpBarWidth / 10.0f) * 0.5f;
            float sy = (m_hpBarHeight / 10.0f) * 0.5f;
            m_hpBarTimer->setScale(sx, sy);
        }
    }

    // 3. 记录基础缩放
    m_baseScale = this->getScale();

    // 4. 初始化音效
    CocosDenshion::SimpleAudioEngine::getInstance()->setEffectsVolume(0.6f);

    // 5. 激活建筑
    activateBuilding();

    // 6. 注册到管理器
    BattleManager::getInstance()->addBuilding(this);

    this->scheduleUpdate();
    return true;
}

void Building::activateBuilding()
{
    if (!m_isActive)
    {
        m_isActive = true;
        this->scheduleUpdate();
    }
}

void Building::onDeath()
{
    // 陷阱不参与存储上限变更，也不可被摧毁
    if (m_type == BuildingType::TRAP)
    {
        return;
    }
    
    // 【关键修复】延迟移除，防止在遍历 Building 列表时删除自身导致迭代器失效崩溃
    // 将移除操作放入 CallFunc 中，使其在 ActionManager 更新时执行，而不是在当前逻辑循环中立即执行
    auto action = CallFunc::create([this]() {
        BattleManager::getInstance()->removeBuilding(this);
        GameEntity::onDeath();
    });
    this->runAction(action);
}

// 【关键修改】基类 updateLogic 仅保留通用加速逻辑，其余移除
void Building::updateLogic(float dt)
{
    m_timer += dt;

    // 处理加速计时器（通用）
    if (m_rateBoostTimer > 0.0f)
    {
        m_rateBoostTimer -= dt;
        if (m_rateBoostTimer <= 0.0f)
        {
            m_rateBoostTimer = 0.0f;
            m_rateMultiplier = 1.0f;
        }
    }

    // 【新增】处理施工加速计时器
    if (m_constructionBoostTimer > 0.0f)
    {
        m_constructionBoostTimer -= dt;
        if (m_constructionBoostTimer <= 0.0f)
        {
            m_constructionBoostTimer = 0.0f;
            m_constructionSpeedMultiplier = 1.0f;
        }
    }

    // 【新增】安全检查：防止升级动画卡死
    // 如果本地显示正在升级，但 GameManager 中没有对应的任务（说明任务已完成或丢失），则强制结束升级
    if (m_isUpgrading)
    {
        m_upgradeCheckTimer += dt;
        if (m_upgradeCheckTimer >= 1.0f)
        {
            m_upgradeCheckTimer = 0.0f;
            // 使用较小的容差，因为位置应该没变
            if (!GameManager::getInstance()->hasPendingUpgrade(m_type, this->getPosition(), 1.0f))
            {
                CCLOG("Building: Upgrade task missing in GameManager, forcing finish.");
                this->onUpgradeFinished();
            }
        }
    }

    // 生产、攻击、陷阱触发等逻辑已完全移至子类重写函数中
    // 此处不再有任何 switch-case
}

void Building::takeDamage(int damage)
{
    GameEntity::takeDamage(damage);

    if (m_type == BuildingType::TRAP) return;

    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("attack.wav");
}

// 【通用升级逻辑】处理视觉、回血、HP计算，但不处理特殊效果（如加上限）
void Building::onUpgradeFinished()
{
    // 1. 停止施工状态
    m_isUpgrading = false;
    if (m_constructionSprite) {
        m_constructionSprite->removeFromParent();
        m_constructionSprite = nullptr;
    }

    // 2. 基础数值提升
    m_level++;

    // 3. 视觉更新 (先更新贴图，确保 baseScale 基于新贴图计算)
    updateBuildingTexture();
    if (this->getContentSize().width > 0) {
        m_baseScale = 150.0f / this->getContentSize().width;
    }

    // 4. 计算新的最大血量和缩放 (需要在子类或 setLevel 中有统一的公式)
    // 这里我们简单复用 setLevel 的逻辑，或者直接调用 setLevel(m_level)
    setLevel(m_level);

    // 存档
    GameManager::getInstance()->updateHomeBuildingLevel(m_type, this->getPosition(), m_level);

    // 5. "Q弹" 特效
    float finalScale = this->getScale();
    // 修改：从 80% 大小开始弹起，确保点击区域足够大
    this->setScale(finalScale * 0.8f);

    auto popAction = Sequence::create(
        EaseBackOut::create(ScaleTo::create(0.4f, finalScale)),
        CallFunc::create([this]() {
            CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("upgrade_complete.wav");
            }),
        nullptr
    );
    this->runAction(popAction);

    CCLOG("Upgrade finished! Level: %d", m_level);
}

// 【通用逻辑】根据等级调整外观和血量
void Building::updateBuildingTexture()
{
    std::string filename;
    // ... (保留原有的根据类型判断文件名的逻辑) ...
    // 简化起见，这里保持原 switch 逻辑，因为文件名映射是通用的
    switch (m_type)
    {
    case BuildingType::TOWN_HALL:       filename = "TownHall.png";        break;
    case BuildingType::CANNON:          filename = "Cannon.png";          break;
    case BuildingType::GOLD_MINE:       filename = "gold_anim_0.png";    break;
    case BuildingType::GOLD_STORAGE:    filename = "GoldStorage.png";    break;
    case BuildingType::ARCHER_TOWER:    filename = "ArcherTower.png";     break;
    case BuildingType::WALL:            filename = "Wall.png";            break;
    case BuildingType::BARRACKS:        filename = "Barracks.png";        break;
    case BuildingType::ELIXIR_COLLECTOR:filename = "elixir_anim_2.png";   break;
    case BuildingType::ELIXIR_STORAGE:  filename = "ElixirStorage.png";   break;
    case BuildingType::TRAP:            filename = "Trap.png";           break;
    default: break;
    }

    this->setTexture(filename);

    // 重新调整缩放以保持一致大小
    if (this->getContentSize().width > 0) {
        this->setScale(150.0f / this->getContentSize().width);
    }
}

void Building::setLevel(int level)
{
    if (level <= 0) return;
    m_level = level;

    // 计算基础血量（保留原 switch 逻辑，因为基类不知道自己是金矿还是大本营）
    // 这是一个为了兼容旧的 setLevel 接口的妥协
    int baseHp = 500;
    switch (m_type)
    {
    case BuildingType::TOWN_HALL: baseHp = 2000; break;
    case BuildingType::CANNON: baseHp = 800; break;
    case BuildingType::GOLD_MINE: baseHp = 600; break;
    case BuildingType::ARCHER_TOWER: baseHp = 700; break;
    case BuildingType::WALL: baseHp = 1000; break;
    case BuildingType::BARRACKS: baseHp = 800; break;
    case BuildingType::ELIXIR_COLLECTOR: baseHp = 600; break;
    case BuildingType::ELIXIR_STORAGE: baseHp = 1500; break;
    case BuildingType::GOLD_STORAGE: baseHp = 600; break;
    case BuildingType::TRAP: baseHp = 200; break;
    default: baseHp = 500; break;
    }

    // 每级增加 200 HP，回满
    m_maxHp = baseHp + (m_level - 1) * 200;
    m_currentHp = m_maxHp;

    // 存储类上限逻辑移除，由具体子类的 onUpgradeFinished 处理

    // 视觉变大
    float newScale = m_baseScale * std::pow(1.1f, m_level - 1);
    this->setScale(newScale);
}

void Building::showUpgradeButton()
{
    using namespace ui;
    if (m_upgradeBtn) return;

    int cost = getUpgradeCost();
    m_upgradeBtn = Button::create("upgrade.png");
    if (!m_upgradeBtn) return;

    m_upgradeBtn->setTitleText(std::string("$") + std::to_string(cost));
    m_upgradeBtn->setTitleFontName("Arial");
    m_upgradeBtn->setTitleFontSize(50);
    m_upgradeBtn->setTitleColor(Color3B::BLACK);
    m_upgradeBtn->setScale(1.5f);

    Size contentSize = this->getContentSize();
    Vec2 anchor = this->getAnchorPoint();
    float localCenterX = contentSize.width * anchor.x;
    float localTopY = contentSize.height * (1.0f - anchor.y);
    float yOffset = localTopY + 600.0f;
    m_upgradeBtn->setAnchorPoint(Vec2(0.5, 0.5));
    m_upgradeBtn->setPosition(Vec2(localCenterX, yOffset));

    bool affordable = GameManager::getInstance()->getGold() >= cost;
    m_upgradeBtn->setEnabled(affordable);
    m_upgradeBtn->setBright(affordable);

    m_upgradeBtn->addClickEventListener([this](Ref*) {
        // 升级逻辑：点击按钮开始 startUpgradeProcess
        // 注意：startUpgradeProcess 逻辑可以保留在这里，作为基类通用流程
        // 具体的升级效果（加上限）由 onUpgradeFinished 链式调用处理
        int max_level = GameManager::getInstance()->getTown_Hall_Level();
        if (m_type != BuildingType::TOWN_HALL && m_level >= max_level) {
            // 显示提示
            auto visibleSize = Director::getInstance()->getVisibleSize();
            auto scene = Director::getInstance()->getRunningScene();
            if (scene) {
                auto alertBg = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
                scene->addChild(alertBg, 10000);

                // 吞噬触摸，防止点击穿透
                auto listener = EventListenerTouchOneByOne::create();
                listener->setSwallowTouches(true);
                listener->onTouchBegan = [](Touch*, Event*) { return true; };
                scene->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, alertBg);

                auto alertPanel = Sprite::create("barracksBoard.png");
                if (alertPanel) {
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
                okBtn->addClickEventListener([alertBg](Ref*) {
                    alertBg->removeFromParent();
                });
                alertBg->addChild(okBtn);
            }
            return;
        }
        int cost = this->getUpgradeCost();
        if (GameManager::getInstance()->getGold() >= cost) {
            GameManager::getInstance()->addGold(-cost);
            this->startUpgradeProcess(); // 开始施工动画
            this->hideUpgradeButton();
        }
        });

    this->addChild(m_upgradeBtn, 300);

    if (!m_goldListener)
    {
        m_goldListener = EventListenerCustom::create("EVENT_UPDATE_GOLD", [this](EventCustom* event)
            {
                this->updateUpgradeButtonVisibility();
            });
        _eventDispatcher->addEventListenerWithSceneGraphPriority(m_goldListener, this);
    }
}

void Building::hideUpgradeButton()
{
    if (m_upgradeBtn)
    {
        m_upgradeBtn->removeFromParent();
        m_upgradeBtn = nullptr;
    }
    if (m_goldListener)
    {
        _eventDispatcher->removeEventListener(m_goldListener);
        m_goldListener = nullptr;
    }
}

void Building::updateUpgradeButtonVisibility()
{
    int cost = getUpgradeCost();
    bool affordable = GameManager::getInstance()->getGold() >= cost;

    if (affordable)
    {
        if (!m_upgradeBtn) showUpgradeButton();
        else {
            m_upgradeBtn->setTitleText(std::string("UPGRADE\n$") + std::to_string(cost));
            m_upgradeBtn->setEnabled(true);
            m_upgradeBtn->setBright(true);
        }
    }
    else
    {
        if (m_upgradeBtn) hideUpgradeButton();
    }
}

// 获取升级费用（通用公式）
int Building::getUpgradeCost() const
{
    return 200 * m_level;
}

void Building::playWorkAnimation()
{
    // 原有逻辑保留
    std::string framePrefix = "";
    float targetScale = 1.0f;
    int frameCount = 4;

    if (m_type == BuildingType::ELIXIR_COLLECTOR) {
        framePrefix = "elixir_anim_";
        targetScale = 0.1f;
    }
    else if (m_type == BuildingType::GOLD_MINE) {
        framePrefix = "gold_anim_";
        targetScale = 0.1f;
    }
    else {
        return;
    }

    if (this->getActionByTag(0x999)) return;

    Vector<SpriteFrame*> frames;
    for (int i = 0; i < frameCount; ++i) {
        std::string name = StringUtils::format("%s%d.png", framePrefix.c_str(), i);

        // 【关键修改】增加空指针检查
        auto tempSprite = Sprite::create(name);
        if (tempSprite) {
            auto frame = tempSprite->getSpriteFrame();
            if (frame) frames.pushBack(frame);
        }
        // 如果 Sprite::create 返回空，这里直接跳过该帧，避免崩溃
    }

    if (!frames.empty()) {
        auto animation = Animation::createWithSpriteFrames(frames, 0.2f);
        auto animate = Animate::create(animation);
        auto repeat = RepeatForever::create(animate);
        repeat->setTag(0x999);
        this->runAction(repeat);
        this->setScale(targetScale);
    }
}

void Building::applyProductionBoost(float multiplier, float durationSec)
{
    // 【修改】移除 m_productionRate <= 0.0f 的检查
    // 因为有些建筑（如大本营）可能没有生产速率，但仍然需要记录加速状态（虽然可能没用）
    // 或者 m_productionRate 可能在子类初始化时才设置，这里检查可能过早
    // 最重要的是，如果 durationSec 有效，我们就应该应用加速
    if (durationSec <= 0.0f) return;
    
    m_rateMultiplier = multiplier;
    m_rateBoostTimer = durationSec;
    CCLOG("Applied production boost: x%f for %f sec", multiplier, durationSec);
}

// 【保留通用升级流程】施工动画
void Building::startUpgradeProcess()
{
    if (m_isUpgrading) return;

    m_isUpgrading = true;

    // 1. 播放施工动画
    Vector<SpriteFrame*> frames;
    for (int i = 0; i < 3; ++i) {
        std::string name = StringUtils::format("construct_%d.png", i);
        auto frame = Sprite::create(name)->getSpriteFrame();
        if (frame) frames.pushBack(frame);
    }

    if (!frames.empty()) {
        auto animation = Animation::createWithSpriteFrames(frames, 0.2f);
        auto animate = Animate::create(animation);
        auto repeat = RepeatForever::create(animate);

        if (m_constructionSprite) {
            m_constructionSprite->removeFromParent();
            m_constructionSprite = nullptr;
        }

        m_constructionSprite = Sprite::createWithSpriteFrame(frames.front());
        Size size = this->getContentSize();
        m_constructionSprite->setPosition(Vec2(size.width / 2.5, size.height / 2));
        this->addChild(m_constructionSprite, 10);
        m_constructionSprite->runAction(repeat);
        m_constructionSprite->setScale(0.2f);
    }

    // 2. 注册到 GameManager 进行全局计时
    float upgradeDuration = 10.0f;
    GameManager::getInstance()->scheduleBuildingUpgrade(this, upgradeDuration);

    // 【新增】如果当前已经处于施工加速状态，立即应用加速
    if (m_constructionBoostTimer > 0.0f && m_constructionSpeedMultiplier > 1.0f)
    {
        GameManager::getInstance()->applyBuildingUpgradeBoost(m_type, this->getPosition(), m_constructionSpeedMultiplier, m_constructionBoostTimer);
    }

    // 3. 本地不再使用 Sequence/DelayTime 计时，而是等待 GameManager 回调
    // 移除之前的 runAction(seq)
}
void Building::applyConstructionBoost(float multiplier, float durationSec)
{
    if (durationSec <= 0.0f) return;
    if (multiplier <= 1.0f) multiplier = 1.0f;

    m_constructionSpeedMultiplier = multiplier;
    m_constructionBoostTimer = durationSec;
    
    CCLOG("Applied construction boost: x%f for %f sec", multiplier, durationSec);

    // 如果正在施工，通知 GameManager 更新剩余时间
    if (m_isUpgrading)
    {
        GameManager::getInstance()->applyBuildingUpgradeBoost(m_type, this->getPosition(), multiplier, durationSec);
    }
}

void Building::showConstructionAnimation()
{
    if (m_isUpgrading) return;

    m_isUpgrading = true;

    Vector<SpriteFrame*> frames;
    for (int i = 0; i < 3; ++i) {
        std::string name = StringUtils::format("construct_%d.png", i);
        auto frame = Sprite::create(name)->getSpriteFrame();
        if (frame) {
            frames.pushBack(frame);
        }
    }

    if (frames.empty()) return;

    if (m_constructionSprite) {
        m_constructionSprite->removeFromParent();
        m_constructionSprite = nullptr;
    }

    m_constructionSprite = Sprite::createWithSpriteFrame(frames.front());
    Size size = this->getContentSize();
    m_constructionSprite->setPosition(Vec2(size.width / 2.5f, size.height / 2));
    this->addChild(m_constructionSprite, 10);

    auto animation = Animation::createWithSpriteFrames(frames, 0.2f);
    auto animate = Animate::create(animation);
    auto repeat = RepeatForever::create(animate);
    m_constructionSprite->runAction(repeat);
    m_constructionSprite->setScale(0.2f);
}