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

// 构造函数
Building::Building()
    : GameEntity()
    , m_type(BuildingType::TOWN_HALL)
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
    , m_upgradeCheckTimer(0.0f)
{
}

// 析构函数
Building::~Building()
{
    if (m_goldListener)
    {
        _eventDispatcher->removeEventListener(m_goldListener);
        m_goldListener = nullptr;
    }
}

// 工厂方法：根据类型创建具体的建筑子类
Building* Building::create(BuildingType type)
{
    Building* pRet = nullptr;

    switch (type)
    {
    case BuildingType::TOWN_HALL:       pRet = new TownHall();        break;
    case BuildingType::GOLD_MINE:       pRet = new GoldMine();        break;
    case BuildingType::ELIXIR_COLLECTOR:pRet = new ElixirCollector(); break;
    case BuildingType::CANNON:          pRet = new Cannon();          break;
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
        pRet->m_type = type;
        if (pRet->init())
        {
            pRet->autorelease();
            return pRet;
        }
    }
    delete pRet;
    return nullptr;
}

// 初始化逻辑
bool Building::init()
{
    if (!GameEntity::init())
    {
        return false;
    }

    // 调用子类实现的初始化函数
    initBuildingProperties();

    // 初始化血条（通用逻辑）
    if (m_hpBgSprite)
    {
        Size bgSize = m_hpBgSprite->getContentSize();
        if (bgSize.width > 0 && bgSize.height > 0)
        {
            // 适当缩小血条
            float sx = (m_hpBarWidth / 10.0f) * 0.5f;
            float sy = (m_hpBarHeight / 10.0f) * 0.5f;
            m_hpBgSprite->setScale(sx, sy);
        }
    }

    if (m_hpBarTimer && m_hpBarTimer->getSprite())
    {
        Size fillSize = m_hpBarTimer->getSprite()->getContentSize();
        if (fillSize.width > 0 && fillSize.height > 0)
        {
            // 适当缩小血条
            float sx = (m_hpBarWidth / 10.0f) * 0.5f;
            float sy = (m_hpBarHeight / 10.0f) * 0.5f;
            m_hpBarTimer->setScale(sx, sy);
        }
    }

    // 记录基础缩放
    m_baseScale = this->getScale();

    // 初始化音效
    CocosDenshion::SimpleAudioEngine::getInstance()->setEffectsVolume(0.6f);

    // 激活建筑
    activateBuilding();

    // 注册到管理器
    BattleManager::getInstance()->addBuilding(this);

    this->scheduleUpdate();
    return true;
}

// 激活建筑
void Building::activateBuilding()
{
    if (!m_isActive)
    {
        m_isActive = true;
        this->scheduleUpdate();
    }
}

// 死亡逻辑
void Building::onDeath()
{
    // 陷阱不参与存储上限变更，也不可被摧毁
    if (m_type == BuildingType::TRAP)
    {
        return;
    }

    // 延迟移除，防止在遍历 Building 列表时删除自身导致迭代器失效崩溃
    auto action = CallFunc::create
    ([this]()
        {
            BattleManager::getInstance()->removeBuilding(this);
            GameEntity::onDeath();
        }
    );
    this->runAction(action);
}

// 更新逻辑：处理加速计时器与升级状态检查
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

    // 处理施工加速计时器
    if (m_constructionBoostTimer > 0.0f)
    {
        m_constructionBoostTimer -= dt;
        if (m_constructionBoostTimer <= 0.0f)
        {
            m_constructionBoostTimer = 0.0f;
            m_constructionSpeedMultiplier = 1.0f;
        }
    }

    // 安全检查：防止升级动画卡死
    // 如果本地显示正在升级，但 GameManager 中没有对应的任务，则强制结束升级
    if (m_isUpgrading)
    {
        m_upgradeCheckTimer += dt;
        if (m_upgradeCheckTimer >= 1.0f)
        {
            m_upgradeCheckTimer = 0.0f;
            if (!GameManager::getInstance()->hasPendingUpgrade(m_type, this->getPosition(), 1.0f))
            {
                this->onUpgradeFinished();
            }
        }
    }
}

// 受伤逻辑：播放音效
void Building::takeDamage(int damage)
{
    GameEntity::takeDamage(damage);

    if (m_type == BuildingType::TRAP)
    {
        return;
    }

    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("attack.wav");
}

// 升级完成回调：处理视觉、数值与特效
void Building::onUpgradeFinished()
{
    // 停止施工状态
    m_isUpgrading = false;
    if (m_constructionSprite)
    {
        m_constructionSprite->removeFromParent();
        m_constructionSprite = nullptr;
    }

    // 基础数值提升
    m_level++;

    // 视觉更新 (先更新贴图，确保 baseScale 基于新贴图计算)
    updateBuildingTexture();
    if (this->getContentSize().width > 0)
    {
        m_baseScale = 150.0f / this->getContentSize().width;
    }

    // 计算新的最大血量和缩放
    setLevel(m_level);

    // 存档
    GameManager::getInstance()->updateHomeBuildingLevel(m_type, this->getPosition(), m_level);

    // "Q弹" 特效
    float finalScale = this->getScale();
    this->setScale(finalScale * 0.8f);

    auto popAction = Sequence::create(
        EaseBackOut::create(ScaleTo::create(0.4f, finalScale)),
        CallFunc::create
        ([this]()
            {
                CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("upgrade_complete.wav");
            }
        ),
        nullptr
    );
    this->runAction(popAction);
}

// 更新建筑外观贴图
void Building::updateBuildingTexture()
{
    std::string filename;
    switch (m_type)
    {
    case BuildingType::TOWN_HALL:       filename = "TownHall.png";        break;
    case BuildingType::CANNON:          filename = "Cannon.png";          break;
    case BuildingType::GOLD_MINE:       filename = "gold_anim_0.png";     break;
    case BuildingType::GOLD_STORAGE:    filename = "GoldStorage.png";     break;
    case BuildingType::ARCHER_TOWER:    filename = "ArcherTower.png";     break;
    case BuildingType::WALL:            filename = "Wall.png";            break;
    case BuildingType::BARRACKS:        filename = "Barracks.png";        break;
    case BuildingType::ELIXIR_COLLECTOR:filename = "elixir_anim_2.png";   break;
    case BuildingType::ELIXIR_STORAGE:  filename = "ElixirStorage.png";   break;
    case BuildingType::TRAP:            filename = "Trap.png";            break;
    default: break;
    }

    this->setTexture(filename);

    // 重新调整缩放以保持一致大小
    if (this->getContentSize().width > 0)
    {
        this->setScale(150.0f / this->getContentSize().width);
    }
}

// 设置建筑等级并重新计算属性
void Building::setLevel(int level)
{
    if (level <= 0)
    {
        return;
    }
    m_level = level;

    // 计算基础血量（保留原 switch 逻辑，因为基类不知道自己是金矿还是大本营）
    int baseHp = 500;
    switch (m_type)
    {
    case BuildingType::TOWN_HALL:        baseHp = 2000; break;
    case BuildingType::CANNON:           baseHp = 800;  break;
    case BuildingType::GOLD_MINE:        baseHp = 600;  break;
    case BuildingType::ARCHER_TOWER:     baseHp = 700;  break;
    case BuildingType::WALL:             baseHp = 1000; break;
    case BuildingType::BARRACKS:         baseHp = 800;  break;
    case BuildingType::ELIXIR_COLLECTOR: baseHp = 600;  break;
    case BuildingType::ELIXIR_STORAGE:   baseHp = 1500; break;
    case BuildingType::GOLD_STORAGE:     baseHp = 600;  break;
    case BuildingType::TRAP:             baseHp = 200;  break;
    default: baseHp = 500; break;
    }

    // 每级增加 200 HP，回满
    m_maxHp = baseHp + (m_level - 1) * 200;
    m_currentHp = m_maxHp;

    // 视觉变大
    float newScale = m_baseScale * std::pow(1.1f, m_level - 1);
    this->setScale(newScale);
}

// 显示升级确认面板
void Building::showUpgradeButton()
{
    // 获取当前运行的场景（全局，不属于地图层）
    auto scene = Director::getInstance()->getRunningScene();
    if (!scene) return;

    // 检查场景中是否已有面板（防止重复弹出）
    if (scene->getChildByName("UpgradePanel")) return;

    // 创建面板背景
    auto panel = cocos2d::ui::Layout::create();
    panel->setName("UpgradePanel");
    panel->setBackGroundImage("barracksBoard.png");
    panel->setContentSize(Size(400, 300));
    panel->setAnchorPoint(Vec2(0.5, 0.5)); // 中心对齐

    // 获取屏幕大小
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 位置设置在屏幕正中央
    panel->setPosition(visibleSize / 2);
    panel->setScale(1.5f);

    // 将面板添加到 Scene，Z轴设为非常大的数字（比如 99999）
    // 使之变为全局 UI，永远在最上层
    scene->addChild(panel, 99999);

    // 显示升级信息 (保持不变)
    int cost = getUpgradeCost();
    int nextLevel = m_level + 1;

    auto infoLabel = Label::createWithSystemFont(
        "Upgrade to Lv." + std::to_string(nextLevel) + "\nCost: " + std::to_string(cost),
        "Arial", 40);
    infoLabel->setPosition(Vec2(200, 200));
    infoLabel->setColor(Color3B::BLACK);
    infoLabel->setAlignment(TextHAlignment::CENTER);
    panel->addChild(infoLabel);

    // 确认按钮
    auto btnYes = cocos2d::ui::Button::create("yes.png");
    btnYes->setScale(0.2f);
    btnYes->setPosition(Vec2(100, 80));
    btnYes->addClickEventListener
    ([this, cost, panel](Ref*)
        {
            int max_level = GameManager::getInstance()->getTown_Hall_Level();
            // 大本营等级限制检查
            if (m_type != BuildingType::TOWN_HALL && m_level >= max_level)
            {
                auto visibleSize = Director::getInstance()->getVisibleSize();
                auto scene = Director::getInstance()->getRunningScene();
                if (scene)
                {
                    // 显示等级限制提示弹窗
                    auto alertBg = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
                    scene->addChild(alertBg, 10000);

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

                    auto okBtn = cocos2d::ui::Button::create("yes.png");
                    okBtn->setScale(0.1f);
                    okBtn->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.4f));
                    okBtn->addClickEventListener
                    ([alertBg](Ref*)
                        {
                            alertBg->removeFromParent();
                        }
                    );
                    alertBg->addChild(okBtn);
                }
                panel->removeFromParent();
                return;
            }

            // 金币检查与扣款
            if (GameManager::getInstance()->getGold() >= cost)
            {
                GameManager::getInstance()->addGold(-cost);
                this->startUpgradeProcess();

                // 直接移除面板
                panel->removeFromParent();
            }
        }
    );
    panel->addChild(btnYes);

    // 取消按钮
    auto btnNo = cocos2d::ui::Button::create("no.png");
    btnNo->setScale(0.15f);
    btnNo->setPosition(Vec2(300, 80));
    btnNo->addClickEventListener([panel](Ref*) 
        {
        // 直接移除面板
        panel->removeFromParent();
        });
    panel->addChild(btnNo);
}

// 隐藏升级确认面板
void Building::hideUpgradeButton()
{
    auto panel = this->getChildByName("UpgradePanel");
    if (panel)
    {
        panel->removeFromParent();
    }

    // 同时也清理旧的 m_upgradeBtn (如果存在)
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

// 根据金币状态更新升级按钮显示
void Building::updateUpgradeButtonVisibility()
{
    if (!this->getChildByName("UpgradePanel"))
    {
        showUpgradeButton();
    }
}

// 获取升级费用（通用公式）
int Building::getUpgradeCost() const
{
    return 200 * m_level;
}

// 播放工作动画
void Building::playWorkAnimation()
{
    std::string framePrefix = "";
    float targetScale = 1.0f;
    int frameCount = 4;

    if (m_type == BuildingType::ELIXIR_COLLECTOR)
    {
        framePrefix = "elixir_anim_";
        targetScale = 0.1f;
    }
    else if (m_type == BuildingType::GOLD_MINE)
    {
        framePrefix = "gold_anim_";
        targetScale = 0.1f;
    }
    else return;

    if (this->getActionByTag(0x999))
    {
        return;
    }

    Vector<SpriteFrame*> frames;
    for (int i = 0; i < frameCount; ++i)
    {
        std::string name = StringUtils::format("%s%d.png", framePrefix.c_str(), i);

        // 增加空指针检查
        auto tempSprite = Sprite::create(name);
        if (tempSprite)
        {
            auto frame = tempSprite->getSpriteFrame();
            if (frame)
            {
                frames.pushBack(frame);
            }
        }
    }

    if (!frames.empty())
    {
        auto animation = Animation::createWithSpriteFrames(frames, 0.2f);
        auto animate = Animate::create(animation);
        auto repeat = RepeatForever::create(animate);
        repeat->setTag(0x999);
        this->runAction(repeat);
        this->setScale(targetScale);
    }
}

// 应用生产加速
void Building::applyProductionBoost(float multiplier, float durationSec)
{
    if (durationSec <= 0.0f)
    {
        return;
    }

    m_rateMultiplier = multiplier;
    m_rateBoostTimer = durationSec;
}

// 开始升级流程：播放动画并注册任务
void Building::startUpgradeProcess()
{
    if (m_isUpgrading)
    {
        return;
    }

    m_isUpgrading = true;

    // 播放施工动画
    Vector<SpriteFrame*> frames;
    for (int i = 0; i < 3; ++i)
    {
        std::string name = StringUtils::format("construct_%d.png", i);
        auto frame = Sprite::create(name)->getSpriteFrame();
        if (frame)
        {
            frames.pushBack(frame);
        }
    }

    if (!frames.empty())
    {
        auto animation = Animation::createWithSpriteFrames(frames, 0.2f);
        auto animate = Animate::create(animation);
        auto repeat = RepeatForever::create(animate);

        if (m_constructionSprite)
        {
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

    // 注册到 GameManager 进行全局计时
    float upgradeDuration = 10.0f;
    GameManager::getInstance()->scheduleBuildingUpgrade(this, upgradeDuration);

    // 如果当前已经处于施工加速状态，立即应用加速
    if (m_constructionBoostTimer > 0.0f && m_constructionSpeedMultiplier > 1.0f)
    {
        GameManager::getInstance()->applyBuildingUpgradeBoost(m_type, this->getPosition(), m_constructionSpeedMultiplier, m_constructionBoostTimer);
    }
}

// 应用施工加速
void Building::applyConstructionBoost(float multiplier, float durationSec)
{
    if (durationSec <= 0.0f)
    {
        return;
    }
    if (multiplier <= 1.0f)
    {
        multiplier = 1.0f;
    }

    m_constructionSpeedMultiplier = multiplier;
    m_constructionBoostTimer = durationSec;

    // 如果正在施工，通知 GameManager 更新剩余时间
    if (m_isUpgrading)
    {
        GameManager::getInstance()->applyBuildingUpgradeBoost(m_type, this->getPosition(), multiplier, durationSec);
    }
}

// 显示施工动画（用于读取存档时立即恢复动画）
void Building::showConstructionAnimation()
{
    if (m_isUpgrading)
    {
        return;
    }

    m_isUpgrading = true;

    Vector<SpriteFrame*> frames;
    for (int i = 0; i < 3; ++i)
    {
        std::string name = StringUtils::format("construct_%d.png", i);
        auto frame = Sprite::create(name)->getSpriteFrame();
        if (frame)
        {
            frames.pushBack(frame);
        }
    }

    if (frames.empty())
    {
        return;
    }

    if (m_constructionSprite)
    {
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