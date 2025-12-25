//Building.cpp
#include "Building.h"
#include "BattleManager.h"
#include "Troop.h"
#include "GameManager.h"
#include "SimpleAudioEngine.h" // 新增：音效
USING_NS_CC;

// 【新增】实现军营升级配置表
const std::map<int, Building::BarrackUpgradeConfig> Building::BARRACK_UPGRADE_CONFIGS = {
    {0, {0, 0}},                              // 未建造：cost上限0
    {1, {0, 10}},                             // 一级：免费，cost上限10
    {2, {500, 25}},                           // 二级：500金币，cost上限25
    {3, {1000, 50}}                           // 三级：1000金币，cost上限50
};

Building::Building()
    : m_type(BuildingType::TOWN_HALL)
    , m_level(1)
    , m_timer(0.0f)
    , m_upgradeBtn(nullptr)
    , m_goldListener(nullptr)
    , m_constructionSprite(nullptr)
    , m_baseScale(1.0f)
    , m_barrackLevel(0)
    , m_currentCostUsed(0)
    , m_maxCostLimit(0)
{
}

Building::~Building()
{
    // 确保移除监听
    if (m_goldListener)
    {
        _eventDispatcher->removeEventListener(m_goldListener);
        m_goldListener = nullptr;
    }
}

Building* Building::create(BuildingType type)
{
    Building* pRet = new(std::nothrow) Building();
    if (pRet)
    {
        pRet->m_type = type; // 先设置类型
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

    initBuildingProperties();                        // 初始化属性
    if (m_hpBgSprite)
    {
        Size bgSize = m_hpBgSprite->getContentSize();
        if (bgSize.width > 0 && bgSize.height > 0) {
            float sx = m_hpBarWidth/10;
            float sy = m_hpBarHeight/10;
            m_hpBgSprite->setScale(sx, sy);
        }
    }

    if (m_hpBarTimer && m_hpBarTimer->getSprite())
    {
        Size fillSize = m_hpBarTimer->getSprite()->getContentSize();
        if (fillSize.width > 0 && fillSize.height > 0) {
            float sx = m_hpBarWidth / 10;
            float sy = m_hpBarHeight / 10;
            m_hpBarTimer->setScale(sx, sy);
        }
    }

    // 记录基础缩放（initBuildingProperties 已经对 scale 做了初始设定）
    m_baseScale = this->getScale();

    // 【新增】初始化音效音量（0.0~1.0），可根据需要调整或改为从配置读取
    CocosDenshion::SimpleAudioEngine::getInstance()->setEffectsVolume(0.6f);

    activateBuilding();                              // 激活建筑
    BattleManager::getInstance()->addBuilding(this); // 传递信息：建筑增加
    this->scheduleUpdate();                          // 开启每帧更新 (用于生产资源或攻击)
    return true;
}

void Building::activateBuilding() // 激活建筑
{
    if (!m_isActive)
    {
        m_isActive = true;
        this->scheduleUpdate(); // 只有在激活后才开启每帧更新
    }
}

void Building::onDeath()
{
    // 陷阱不参与存储上限变更，也不可被摧毁
    if (m_type == BuildingType::TRAP)
    {
        return; // 忽略死亡逻辑
    }
    BattleManager::getInstance()->removeBuilding(this);

    GameEntity::onDeath();
}

void Building::initBuildingProperties()
{
    // 1. 定义一个变量存文件名
   // 默认属性
    std::string filename = "CloseNormal.png";
    int hp = 500;
    m_productionRate = 0;
    m_maxStorage = 0;     // 这是“内部暂存”上限 (收集器用)
    m_currentStored = 0;


    switch (m_type)
    {
    case BuildingType::TOWN_HALL:        filename = "TownHall.png";        hp = 2000; break;
    case BuildingType::CANNON:           filename = "Cannon.png";          hp = 800;  break;
    case BuildingType::GOLD_MINE:        filename = "gold_anim_0.png";        hp = 600;  break;
    case BuildingType::GOLD_STORAGE:        filename = "GoldStorage.png";        hp = 600;  break;
    case BuildingType::ARCHER_TOWER:     filename = "ArcherTower.png";     hp = 700;  break;
    case BuildingType::WALL:             filename = "Wall.png";            hp = 1000; break;
    case BuildingType::BARRACKS:
        filename = "Barracks.png";
        hp = 800;
        // 【新增】初始化军营属性
        m_barrackLevel = 1;          // 初始为一级军营
        m_maxCostLimit = BARRACK_UPGRADE_CONFIGS.at(1).maxCostLimit;  // cost上限为10
        m_currentCostUsed = 0;       // 初始使用cost为0
        break;   
    case BuildingType::ELIXIR_COLLECTOR: 
        filename = "elixir_anim_2.png"; 
        hp = 600;
        m_productionRate = 10.0f;
        m_maxStorage = 100.0f;
        break;

    case BuildingType::ELIXIR_STORAGE:  
        filename = "ElixirStorage.png"; 
        hp = 1500;
        m_productionRate = 0;
        GameManager::getInstance()->modifyMaxElixir(1000);
        break;
    case BuildingType::TRAP:             filename = "Trap.png";            hp = 200;  break; // 陷阱：默认隐形
    default:break;
    }

    // 3. 加载图片纹理
    this->setTexture(filename);

    // 4. 设置其他属性
    this->setProperties(hp, CampType::PLAYER);


    // 使用统一固定的血条尺寸，避免不同建筑因图片大小或缩放导致血条不一致
    float DEFAULT_HP_BAR_WIDTH = 5.0f;  // 可根据 UI 需求调整为合适像素
    float DEFAULT_HP_BAR_HEIGHT = 4.0f;  // 宽高比可自定义

    m_hpBarWidth = DEFAULT_HP_BAR_WIDTH;
    m_hpBarHeight = DEFAULT_HP_BAR_HEIGHT;


    this->setHpBarOffsetY(20.0f);

    if (m_type == BuildingType::WALL)
    {
        m_hpBarWidth =2.0f;
        m_hpBarHeight =2.0f;
        // 墙很矮，血条不要飘太高
        this->setHpBarOffsetY(-300.0f);
        this->setHpBarOffsetX(-220.0f);
    }
    else if (m_type == BuildingType::ARCHER_TOWER)
    {
        // 箭塔很高，必须把血条顶上去
        this->setHpBarOffsetY(280.0f);
    }
    else if (m_type == BuildingType::CANNON)
    {
        this->setHpBarOffsetY(240.0f);
    }
    else if (m_type == BuildingType::TOWN_HALL)
    {
        this->setHpBarOffsetX(-40.0f);
        this->setHpBarOffsetY(180.0f);
    }
    else if (m_type == BuildingType::GOLD_MINE)
    {
        this->setHpBarOffsetX(200.0f);
        this->setHpBarOffsetY(880.0f);
    }
    else if (m_type == BuildingType::GOLD_STORAGE)
    {
        this->setHpBarOffsetX(300.0f);
        this->setHpBarOffsetY(880.0f);
    }
    else if (m_type == BuildingType::ELIXIR_COLLECTOR)
    {
        this->setHpBarOffsetX(300.0f);
        this->setHpBarOffsetY(1080.0f);
    }
    else if (m_type == BuildingType::ELIXIR_STORAGE)
    {
        this->setHpBarOffsetX(300.0f);
        this->setHpBarOffsetY(780.0f);
    }
    else if (m_type == BuildingType::BARRACKS)
    {
        this->setHpBarOffsetX(40.0f);
        this->setHpBarOffsetY(280.0f);
    }
    // TRAP 默认隐形，且无血条、不可选中
    else if (m_type == BuildingType::TRAP)
    {
        this->setOpacity(0);            // 隐身
        this->removeHpBar();            // 删除血条机制
        m_maxHp = 0;                    // 无生命值
        m_currentHp = 0;                // 无生命值
        this->setCascadeOpacityEnabled(true);
    }

    // 图片可能很大(比如 500x500)，我们需要把它缩放到合适的大小(比如 64x64)
    // 假设你想让所有建筑大约占 60x60 像素：
    float targetSize = 150.0f;
    Size contentSize = this->getContentSize(); // 获取图片原始大小
    if (contentSize.width > 0)
    {
        this->setScale(targetSize / contentSize.width);
        if (m_type == BuildingType::TRAP) {
            this->setScale(0.1f);
        }
    }
    // 默认非资源建筑属性
    m_productionRate = 0;
    m_maxStorage = 0;
    m_currentStored = 0;
    m_productionAccumulator = 0;

    // 配置金矿属性 (数值参考 CoC 1级金矿)
    if (m_type == BuildingType::GOLD_MINE)
    {
        // 假设：1级金矿每小时200，容量500
        // 为了演示效果，我们调快一点：每秒产 10，容量 100
        m_productionRate = 10.0f;
        m_maxStorage = 100.0f;
    }
    else if (m_type == BuildingType::ELIXIR_COLLECTOR)
    {
        m_productionRate = 10.0f;
        m_maxStorage = 100.0f;
    }
}

// 将原来的 upgrade() 改名为 onUpgradeFinished()
void Building::onUpgradeFinished()
{
    // 1. 停止施工状态
    m_isUpgrading = false;
    if (m_constructionSprite) {
        // 从父节点移除，这会自动停止它正在播放的动画并释放内存
        m_constructionSprite->removeFromParent();
        // 指针置空，防止野指针
        m_constructionSprite = nullptr;
    }

    // 2. ---------- 原 upgrade() 的逻辑开始 ----------

    if (m_type == BuildingType::TOWN_HALL) {
        int maxlevel = GameManager::getInstance()->getTown_Hall_Level();
        GameManager::getInstance()->setTown_Hall_Level(++maxlevel);
    }

    m_level++; // 等级提升

    // 数值提升逻辑
    // 为了更严谨，我们可以用 setLevel 里的公式重新算一遍 MaxHP
    // m_maxHp += 200; 
    int baseHp = 500;
    // ... (复制 setLevel 里的 switch case 获取 baseHp) ...
    m_maxHp = baseHp + (m_level - 1) * 200;
    m_currentHp = m_maxHp; // 回满血

    // 资源上限提升逻辑
    if (m_type == BuildingType::GOLD_STORAGE) {
        int current_max = GameManager::getInstance()->getMaxGold();
        GameManager::getInstance()->modifyMaxGold(current_max + 500);
    }
    if (m_type == BuildingType::ELIXIR_STORAGE) {
        int current_max = GameManager::getInstance()->getMaxElixir();
        GameManager::getInstance()->modifyMaxElixir(current_max + 500);
    }

    // 存档
    GameManager::getInstance()->updateHomeBuildingLevel(m_type, this->getPosition(), m_level);

    // ---------- 原 upgrade() 的逻辑结束 ----------

    // 3. 【视觉核心】变身！
    // 切换回建筑原本的图片 (或者下一级的图片)
    updateBuildingTexture();

    // 4. 播放 "Q弹" 特效表示完成
    // 先记录一下目标缩放（基于 m_baseScale * 1.1^(level-1)）
    float targetScale = m_baseScale * std::pow(1.1f, m_level - 1);
    this->setScale(0.1f); // 瞬间变小

    auto popAction = Sequence::create(
        EaseBackOut::create(ScaleTo::create(0.5f, targetScale)),
        CallFunc::create([this]() {
            CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("upgrade_complete.wav");
            }),
        nullptr
    );
    this->runAction(popAction);

    CCLOG("Upgrade finished! Level: %d", m_level);
}

void Building::updateLogic(float dt)
{
    m_timer += dt;

    // 【新增】更新加速计时器
    if (m_constructionBoostTimer > 0.0f)
    {
        m_constructionBoostTimer -= dt;
        if (m_constructionBoostTimer <= 0.0f)
        {
            m_constructionBoostTimer = 0.0f;
            m_constructionSpeedMultiplier = 1.0f;
        }
    }

    // 定期检查升级状态（防止跨场景回调丢失）
    if (m_isUpgrading)
    {
        m_upgradeCheckTimer += dt;
        if (m_upgradeCheckTimer >= 1.0f)
        {
            m_upgradeCheckTimer = 0.0f;
            // 如果 GameManager 里已经没有这个升级任务了，说明升级完成了
            if (!GameManager::getInstance()->hasPendingUpgrade(m_type, this->getPosition()))
            {
                this->onUpgradeFinished();
            }
        }
    }

    // 陷阱逻辑：敌军单位在范围内时显现，并定期造成范围伤害
    if (m_type == BuildingType::TRAP)
    {
        const float trapRange = 50.0f;     // 设置触发范围
        const int trapDps = 30;            // 每次触发伤害
        bool enemyInRange = false;

        this->setCascadeOpacityEnabled(true);
        this->setTag(-1);

        // 仅对地面单位触发显形（不会因空中单位如 Dragon 显形/攻击）
        for (auto troop : BattleManager::getInstance()->getTroops())
        {
            if (!troop || troop->isDead()) continue;
            if (troop->getMovementType() == TroopMovementType::AIR) continue; // 跳过空军（Dragon）
            float dist = this->getPosition().distance(troop->getPosition());
            if (dist <= trapRange)
            {
                enemyInRange = true;
                break;
            }
        }

        this->setOpacity(enemyInRange ? 255 : 0);

        if (enemyInRange && m_timer >= 0.5f)
        {
            m_timer = 0.0f;

            // 【修复】避免遍历时移除导致迭代器失效：改为调用 BattleManager::dealAreaDamage（内部先收集再伤害）
            BattleManager::getInstance()->dealAreaDamage(this->getPosition(), trapRange, trapDps);

            auto seq = Sequence::create(
                TintTo::create(0.1f, Color3B::RED),
                TintTo::create(0.1f, this->getColor()),
                nullptr
            );
            this->runAction(seq);
        }
        return;
    }

    // 机制：自动生产 & 存满即停

    // 只有资源类建筑才生产
    //核心机制：存满即停
    if (m_productionRate > 0)
    {
        if (m_rateBoostTimer > 0.0f)
        {
            m_rateBoostTimer -= dt;
            if (m_rateBoostTimer <= 0.0f)
            {
                m_rateBoostTimer = 0.0f;
                m_rateMultiplier = 1.0f;
            }
        }
        if (m_currentStored >= m_maxStorage)
        {
            m_currentStored = m_maxStorage;
            return; // 满了就不跑下面的累加代码了 -> 停止生产
        }
        // ... 累加代码 ...


        // 2. 累加生产
        // 我们不直接用 m_currentStored += rate * dt，因为 int 丢失精度
        // 我们用一个累加器，攒够 1 块钱再加进去
        float effectiveRate = m_productionRate * ((m_rateBoostTimer > 0.0f) ? m_rateMultiplier : 1.0f);
        m_productionAccumulator += effectiveRate * dt;

        if (m_productionAccumulator >= 1.0f)
        {
            int amountToAdd = (int)m_productionAccumulator;
            m_currentStored += amountToAdd;
            m_productionAccumulator -= amountToAdd; // 扣除已加部分

            // 再次检查上限
            if (m_currentStored >= m_maxStorage)
            {
                m_currentStored = m_maxStorage;
            }

        }
    }
    // 2. 防御塔逻辑
    else if (m_type == BuildingType::CANNON || m_type == BuildingType::ARCHER_TOWER)
    {
        if (m_timer >= 1.0f) // 攻击间隔1秒
        {
            // 设定射程
            float range = 150.0f;

            // 询问 BattleManager：我射程内有没有敌人？
            Troop* target = BattleManager::getInstance()->findClosestTroopForBuilding(this);
            if (target)
            {
                m_timer = 0; // 重置冷却

                // 造成伤害 (假设加农炮伤害 80)
                CCLOG("Cannon Fired at Troop!");
                target->takeDamage(80); // 造成伤害

                // 视觉反馈 简单的闪烁效果 表示开火
                auto seq = Sequence::create
                (
                    TintTo::create(0.1f, Color3B::ORANGE),
                    TintTo::create(0.1f, this->getColor()),
                    nullptr
                );
                this->runAction(seq);
            }
        }
    }
}

// 机制：玩家收集 & 转入金库
int Building::collectResource()
{
    // 1. 如果没存货，收集个寂寞
    if (m_currentStored <= 0) return 0;

    // 2. 取出所有存货 (取整)
    int amountToCollect = (int)m_currentStored;

    // 3. 转入国库 (GameManager)
    if (m_type == BuildingType::GOLD_MINE)
    {
        GameManager::getInstance()->addGold(amountToCollect);
    }
    else if (m_type == BuildingType::ELIXIR_COLLECTOR)
    {
        GameManager::getInstance()->addElixir(amountToCollect);
    }

    // 4. 清空内部存储，恢复生产
    m_currentStored = 0.0f;
    m_productionAccumulator = 0.0f;

    CCLOG("Collected %d resources!", amountToCollect);
    return amountToCollect;
}

bool Building::isFull() const
{
    return m_currentStored >= m_maxStorage;
}

int Building::getUpgradeCost() const
{
    // 简单的升级价格公式：基础 200 * 等级
    return 200 * m_level;
}

void Building::showUpgradeButton()
{
    using namespace ui;
    if (m_upgradeBtn) return;

    int cost = getUpgradeCost();

    m_upgradeBtn = Button::create("upgrade.png");
    if (!m_upgradeBtn) return;
    m_upgradeBtn->setTitleText(std::string("$") + std::to_string(this->getUpgradeCost()));
    m_upgradeBtn->setTitleFontName("Arial");
    m_upgradeBtn->setTitleFontSize(50);
    m_upgradeBtn->setTitleColor(cocos2d::Color3B::BLACK);
    m_upgradeBtn->setScale(1.5f);

    // 放在建筑顶部之上（相对于建筑中心）
    Size contentSize = this->getContentSize();
    // 计算在父节点局部坐标系中的水平中心点（考虑 anchor）
    Vec2 anchor = this->getAnchorPoint();
    float localCenterX = contentSize.width * anchor.x;
    // 计算顶部本地 Y（contentSize 高度的顶部位置），然后向上偏移
    float localTopY = contentSize.height * (1.0f - anchor.y);
    float yOffset = localTopY + 600.0f;
    m_upgradeBtn->setAnchorPoint(Vec2(0.5f, 0.5f));
    m_upgradeBtn->setPosition(Vec2(localCenterX, yOffset));

    // 初始可用性
    bool affordable = GameManager::getInstance()->getGold() >= cost;
    m_upgradeBtn->setEnabled(affordable);
    m_upgradeBtn->setBright(affordable);

    // 点击监听
    m_upgradeBtn->addClickEventListener
    ([this](Ref*)
        {
            int max_level = GameManager::getInstance()->getTown_Hall_Level();
            if (m_type!=BuildingType::TOWN_HALL&&m_level == max_level) {
                    auto warning = Label::createWithSystemFont("the level of buildings can't exceed the level of town_hall", "Arial", 24);
                    warning->setTextColor(Color4B::YELLOW);
                    warning->setOpacity(0);
                    warning->setAnchorPoint(Vec2(0.5f, 0.5f));
                    Size visibleSize = Director::getInstance()->getVisibleSize();
                    warning->setPosition(Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.5f));
                    warning->enableOutline(Color4B::BLACK, 2);
                    if (auto scene = Director::getInstance()->getRunningScene())
                    {
                        scene->addChild(warning, 1000);
                    }
 
                     warning->runAction(Sequence::create
                     (
                         FadeIn::create(0.1f),
                         DelayTime::create(1.0f),
                         FadeOut::create(0.2f),
                         RemoveSelf::create(),
                         nullptr
                     ));
                     return;
                 }

            int cost = this->getUpgradeCost();
            if (GameManager::getInstance()->getGold() >= cost)
            {
                GameManager::getInstance()->addGold(-cost);
                this->startUpgradeProcess();
                this->hideUpgradeButton();
            }
            else
            {
                CCLOG("Not enough gold to upgrade");
            }
        });

    // 添加到建筑节点，这样会随着建筑移动
    this->addChild(m_upgradeBtn, 300);

    // 注册金币更新监听用于自动显示/隐藏（绑定到建筑，随建筑自动移除）
    if (!m_goldListener)
    {
        m_goldListener = EventListenerCustom::create
        ("EVENT_UPDATE_GOLD", [this](EventCustom* event)
            {
                this->updateUpgradeButtonVisibility();
            }
        );
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
    // Ensure we have a listener so future gold changes re-evaluate visibility
    if (!m_goldListener)
    {
        m_goldListener = EventListenerCustom::create
        ("EVENT_UPDATE_GOLD", [this](EventCustom* event)
            {
                this->updateUpgradeButtonVisibility();
            }
        );
        _eventDispatcher->addEventListenerWithSceneGraphPriority(m_goldListener, this);
    }

    int cost = getUpgradeCost();
    bool affordable = GameManager::getInstance()->getGold() >= cost;

    if (affordable)
    {
        // show if not already shown
        if (!m_upgradeBtn)
        {
            showUpgradeButton();
        }
        else
        {
            // update existing button text/state
            m_upgradeBtn->setTitleText(std::string("UPGRADE\n$") + std::to_string(cost));
            m_upgradeBtn->setEnabled(true);
            m_upgradeBtn->setBright(true);
        }
    }
    else
    {
        // hide if exists
        if (m_upgradeBtn)
        {
            hideUpgradeButton();
        }
    }
}

// 新增：实现 setLevel
void Building::setLevel(int level)
{
    if (level <= 0) return;
    m_level = level;

    // 根据等级调整血量和显示（简单策略）
    // 每级增加 200 点最大生命值，并恢复当前生命值为满
    // 为了避免重复叠加，在 initBuildingProperties 中已经设置了基础 m_maxHp
    // 我们将 m_maxHp 设置为基础 + (level-1)*200

    int baseHp = 500; // 默认基础血量，如果类型有特定值，需要先取决于类型
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
    case BuildingType::TRAP: baseHp = 200; break;
    default: baseHp = 500; break;
    }
    m_maxHp = baseHp + (m_level - 1) * 200;
    m_currentHp = m_maxHp;
    if (m_type == BuildingType::GOLD_STORAGE) {
        int original = GameManager::getInstance()->getMaxGold();
        GameManager::getInstance()->modifyMaxGold(original+500*level);
    }
    if (m_type == BuildingType::ELIXIR_STORAGE) {
        int original = GameManager::getInstance()->getMaxElixir();
        GameManager::getInstance()->modifyMaxElixir(original+500*level);
    }
    // 根据等级修改显示大小（每级放大 10%）
    float newScale = m_baseScale * std::pow(1.1f, m_level - 1);
    this->setScale(newScale);
}

void Building::takeDamage(int damage)
{
    // 先走基础伤害逻辑
    GameEntity::takeDamage(damage);

    // 陷阱不播放被攻击音效（不可选中、无血）
    if (m_type == BuildingType::TRAP) return;

    // 【新增】播放前可按需动态设置音量（示例：降低音量）
    // CocosDenshion::SimpleAudioEngine::getInstance()->setEffectsVolume(0.5f);

    // 播放被攻击音效（确保资源存在于搜索路径）
    auto engine = CocosDenshion::SimpleAudioEngine::getInstance();
    engine->playEffect("attack.wav");
}

void Building::playWorkAnimation()
{
    // 1. 根据建筑类型决定“图片前缀”和“缩放比例”
    std::string framePrefix = "";
    float targetScale = 1.0f; 
    int frameCount = 4; // 假设每种动画都是4帧，如果不一样可以单独设

    if (m_type == BuildingType::ELIXIR_COLLECTOR) 
    {
        framePrefix = "elixir_anim_"; // 圣水瓶图片前缀
        targetScale = 0.1f;           // 圣水瓶缩放
    }
    else if (m_type == BuildingType::GOLD_MINE) 
    {
        framePrefix = "gold_anim_";   // 【新增】金矿图片前缀
        targetScale = 0.1f;           // 【新增】金矿缩放 (你可以根据实际图片大小调整)
    }
    else 
    {
        return; // 其他建筑（如大本营）没有动画，直接返回
    }

    // 2. 防止重复播放
    if (this->getActionByTag(0x999)) return;

    // 3. 加载动画帧
    Vector<SpriteFrame*> frames;
    for (int i = 0; i < frameCount; ++i)
    {
        // 拼接文件名: "elixir_anim_0.png" 或 "gold_anim_0.png"
        std::string name = StringUtils::format("%s%d.png", framePrefix.c_str(), i);
        
        auto frame = Sprite::create(name)->getSpriteFrame();
        // 如果是从图集(plist)加载: 
        // auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(name);
        
        if (frame) {
            frames.pushBack(frame);
        }
    }

    // 4. 播放动画
    if (!frames.empty())
    {
        auto animation = Animation::createWithSpriteFrames(frames, 0.2f);
        auto animate = Animate::create(animation);
        auto repeat = RepeatForever::create(animate);
        
        repeat->setTag(0x999);
        this->runAction(repeat);

        // 应用缩放
        this->setScale(targetScale);
    }
}

void Building::applyProductionBoost(float multiplier, float durationSec)//设置加速参数
{
    if (m_productionRate <= 0.0f || durationSec <= 0.0f) return;
    if (multiplier <= 1.0f) multiplier = 1.0f;

    m_rateMultiplier = multiplier;
    m_rateBoostTimer = durationSec;
}

void Building::applyConstructionBoost(float multiplier, float durationSec)
{
    if (durationSec <= 0.0f) return;
    if (multiplier <= 1.0f) multiplier = 1.0f;

    m_constructionSpeedMultiplier = multiplier;
    m_constructionBoostTimer = durationSec;
    
    // 如果正在施工，通知 GameManager 更新剩余时间
    if (m_isUpgrading)
    {
        GameManager::getInstance()->applyBuildingUpgradeBoost(m_type, this->getPosition(), multiplier, durationSec);
    }
}

// 【新增】设置军营等级（用于从存档恢复）
void Building::setBarrackLevel(int level) {
    if (level < 0 || level > 3) return;
    m_barrackLevel = level;

    // 根据等级设置cost上限
    if (BARRACK_UPGRADE_CONFIGS.find(level) != BARRACK_UPGRADE_CONFIGS.end()) {
        m_maxCostLimit = BARRACK_UPGRADE_CONFIGS.at(level).maxCostLimit;
    }

    // 更新当前使用的cost
    updateCurrentCostUsed();
}

// 【新增】更新当前使用的cost
void Building::updateCurrentCostUsed() {
    m_currentCostUsed = 0;
    auto gm = GameManager::getInstance();

    // 遍历所有兵种，计算总cost
    for (const auto& pair : Troop::TROOP_CONFIGS) {
        TroopType type = pair.first;
        int troopCost = pair.second.cost;
        int troopCount = gm->getTroopCount(type);
        m_currentCostUsed += troopCost * troopCount;

        CCLOG("Troop %d: count=%d, cost=%d, total=%d",
            (int)type, troopCount, troopCost, m_currentCostUsed);
    }

    CCLOG("Final cost usage: %d/%d", m_currentCostUsed, m_maxCostLimit);

    // 发送事件通知UI更新
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_COST_UPDATED");
}

// 【新增】检查是否可以升级军营
bool Building::canUpgradeBarrack() const {
    // 只有军营才能升级
    if (m_type != BuildingType::BARRACKS) return false;

    // 已达最高级
    if (m_barrackLevel >= 3) return false;

    // 检查下一级的升级配置
    int nextLevel = m_barrackLevel + 1;
    if (BARRACK_UPGRADE_CONFIGS.find(nextLevel) != BARRACK_UPGRADE_CONFIGS.end()) {
        int upgradeCost = BARRACK_UPGRADE_CONFIGS.at(nextLevel).goldCost;
        int currentGold = GameManager::getInstance()->getGold();
        return currentGold >= upgradeCost;
    }

    return false;
}

// 【新增】升级军营
void Building::upgradeBarrack() {
    if (!canUpgradeBarrack()) return;

    int nextLevel = m_barrackLevel + 1;
    auto config = BARRACK_UPGRADE_CONFIGS.at(nextLevel);

    // 扣除升级费用
    GameManager::getInstance()->addGold(-config.goldCost);

    // 更新军营等级和cost上限
    m_barrackLevel = nextLevel;
    m_maxCostLimit = config.maxCostLimit;

    CCLOG("Barracks upgraded to level %d, new cost limit: %d", m_barrackLevel, m_maxCostLimit);

    // 重新计算当前cost使用（可能因为升级解锁了新兵种）
    updateCurrentCostUsed();

    // 播放升级特效
    auto scaleUp = ScaleTo::create(0.2f, this->getScale() * 1.2f);
    auto scaleDown = ScaleTo::create(0.2f, this->getScale());
    this->runAction(Sequence::create(scaleUp, scaleDown, nullptr));

    // 发送事件通知UI更新
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_BARRACK_UPGRADED");
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("EVENT_COST_UPDATED");
}

void Building::updateBuildingTexture()
{
    std::string filename;

    // 简单的命名规则示例： TownHall_1.png, TownHall_2.png
    // 如果你还没有分等级的图，可以暂时仍用旧图，只做变大效果
    switch (m_type)
    {
    case BuildingType::TOWN_HALL:
        // 暂时用固定图片，以后改成: filename = StringUtils::format("TownHall_%d.png", m_level);
        filename = "TownHall.png";
        break;

    case BuildingType::CANNON:
        // 修正：之前你写成了 ArcherTower.png
        filename = "Cannon.png";
        break;

    case BuildingType::GOLD_MINE:
        // 以后改成: filename = StringUtils::format("gold_anim_%d_0.png", m_level);
        filename = "gold_anim_0.png";
        break;

    case BuildingType::GOLD_STORAGE:
        filename = "GoldStorage.png";
        break;

    case BuildingType::ARCHER_TOWER:
        filename = "ArcherTower.png";
        break;

    case BuildingType::WALL:
        // 墙壁通常随着等级变化样子变化很明显
        // filename = StringUtils::format("Wall_%d.png", m_level);
        filename = "Wall.png";
        break;

    case BuildingType::BARRACKS:
        // filename = StringUtils::format("Barracks_%d.png", m_level);
        filename = "Barracks.png";
        break;

    case BuildingType::ELIXIR_COLLECTOR:
        // 以后改成: filename = StringUtils::format("elixir_anim_%d_2.png", m_level);
        filename = "elixir_anim_2.png";
        break;

    case BuildingType::ELIXIR_STORAGE:
        filename = "ElixirStorage.png";
        break;

    case BuildingType::TRAP:
        filename = "Trap.png";
        break;

    default:
        filename = "CloseNormal.png";
        break;
    }

    // 重新设置纹理
    this->setTexture(filename);

    // 【关键】因为图片尺寸可能变了，需要重新调整缩放以保持一致的视觉大小
    // 假设标准大小还是 150 像素
    if (this->getContentSize().width > 0) {
        this->setScale(150.0f / this->getContentSize().width);
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

void Building::startUpgradeProcess()
{
    if (m_isUpgrading) return; // 防止重复触发

    float upgradeDuration = 10.0f;
    showConstructionAnimation();
    GameManager::getInstance()->scheduleBuildingUpgrade(this, upgradeDuration);

    // 【新增】如果当前有加速状态，立即应用到新任务
    if (m_constructionBoostTimer > 0.0f)
    {
        GameManager::getInstance()->applyBuildingUpgradeBoost(m_type, this->getPosition(), m_constructionSpeedMultiplier, m_constructionBoostTimer);
    }

    CCLOG("Upgrade started... waiting for %f seconds", upgradeDuration);
}