//Building.cpp
#include "Building.h"
#include "BattleManager.h"
#include"Troop.h"
#include"GameManager.h"
USING_NS_CC;

Building::Building()
    : m_type(BuildingType::TOWN_HALL)
    , m_level(1)
    , m_timer(0.0f)
    , m_upgradeBtn(nullptr)
    , m_goldListener(nullptr)
    , m_baseScale(1.0f)
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

    // 记录基础缩放（initBuildingProperties 已经对 scale 做了初始设定）
    m_baseScale = this->getScale();

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
    // 如果是存储建筑，死的时候要扣除上限
    if (m_type == BuildingType::ELIXIR_STORAGE)
    {
        GameManager::getInstance()->modifyMaxElixir(-1000);
    }
    else if (m_type == BuildingType::GOLD_STORAGE)
    {
        GameManager::getInstance()->modifyMaxGold(-1000);
    }

    BattleManager::getInstance()->removeBuilding(this);

    // 如果是主基地模式，还需要通知 GameManager 移除记录 (addHomeBuilding 的反向操作)
    // 这里暂时省略，视你是否实现了移除建筑功能而定

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
    case BuildingType::GOLD_MINE:        filename = "GoldMine.png";        hp = 600;  break;
    case BuildingType::ARCHER_TOWER:     filename = "ArcherTower.png";     hp = 700;  break;
    case BuildingType::WALL:             filename = "Wall.png";            hp = 1000; break;
    case BuildingType::BARRACKS:         filename = "Barracks.png";        hp = 800; break;
    case BuildingType::ELIXIR_COLLECTOR: filename = "ElixirCollector.png"; hp = 600;
        m_productionRate = 10.0f;
        m_maxStorage = 100.0f;
        break;

    case BuildingType::ELIXIR_STORAGE:   filename = "ElixirStorage.png";   hp = 1500;
        m_productionRate = 0;
        GameManager::getInstance()->modifyMaxElixir(1000);
        break;
    default:break;
    }

    // 3. 加载图片纹理
    this->setTexture(filename);

    // 4. 设置其他属性
    this->setProperties(hp, CampType::PLAYER);

    // 使用统一固定的血条尺寸，避免不同建筑因图片大小或缩放导致血条不一致
    const float DEFAULT_HP_BAR_WIDTH = 120.0f;  // 可根据 UI 需求调整为合适像素
    const float DEFAULT_HP_BAR_HEIGHT = 12.0f;  // 宽高比可自定义

    m_hpBarWidth = DEFAULT_HP_BAR_WIDTH;
    m_hpBarHeight = DEFAULT_HP_BAR_HEIGHT;

    // 图片可能很大(比如 500x500)，我们需要把它缩放到合适的大小(比如 64x64)
    // 假设你想让所有建筑大约占 60x60 像素：
    float targetSize = 150.0f;
    Size contentSize = this->getContentSize(); // 获取图片原始大小
    if (contentSize.width > 0)
    {
        this->setScale(targetSize / contentSize.width);
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

void Building::upgrade()
{
    m_level++;
    m_maxHp += 200; // 升级加血上限
    m_currentHp = m_maxHp; // 并回血
    CCLOG("Building Upgraded to Level %d! Max HP is now %d", m_level, m_maxHp);

    // 稍微变大一点表示升级
    this->setScale(this->getScale() * 1.1f);

    // 同步到 GameManager 存档记录（位置精确匹配）
    GameManager::getInstance()->updateHomeBuildingLevel(m_type, this->getPosition(), m_level);
}

void Building::updateLogic(float dt)
{
    m_timer += dt;

    // 机制：自动生产 & 存满即停

      // 只有资源类建筑才生产
    // 核心机制：存满即停
    if (m_productionRate > 0)
    {
        if (m_currentStored >= m_maxStorage)
        {
            m_currentStored = m_maxStorage;
            return; // 满了就不跑下面的累加代码了 -> 停止生产
        }
        // ... 累加代码 ...


        // 2. 累加生产
        // 我们不直接用 m_currentStored += rate * dt，因为 int 丢失精度
        // 我们用一个累加器，攒够 1 块钱再加进去
        m_productionAccumulator += m_productionRate * dt;

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
            int cost = this->getUpgradeCost();
            if (GameManager::getInstance()->getGold() >= cost)
            {
                GameManager::getInstance()->addGold(-cost);
                this->upgrade();
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
    default: baseHp = 500; break;
    }

    m_maxHp = baseHp + (m_level - 1) * 200;
    m_currentHp = m_maxHp;

    // 根据等级修改显示大小（每级放大 10%）
    float newScale = m_baseScale * std::pow(1.1f, m_level - 1);
    this->setScale(newScale);
}