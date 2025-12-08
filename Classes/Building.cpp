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
{
}

Building::~Building() {
    // 确保移除监听
    if (m_goldListener) {
        _eventDispatcher->removeEventListener(m_goldListener);
        m_goldListener = nullptr;
    }
}

Building* Building::create(BuildingType type) {
    Building* pRet = new(std::nothrow) Building();
    if (pRet) {
        pRet->m_type = type; // 先设置类型
        if (pRet->init()) {
            pRet->autorelease();
            return pRet;
        }
    }
    delete pRet;
    return nullptr;
}

bool Building::init() {
    if (!GameEntity::init()) {
        return false;
    }

    // 初始化属性
    initBuildingProperties();

    // 开启每帧更新 (用于生产资源或攻击)
    this->scheduleUpdate();

    // 告诉管理者：我出生了！
    BattleManager::getInstance()->addBuilding(this);
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

void Building::onDeath() {
    // 如果是存储建筑，死的时候要扣除上限
    if (m_type == BuildingType::ELIXIR_STORAGE) {
        GameManager::getInstance()->modifyMaxElixir(-1000);
    }
    else if (m_type == BuildingType::GOLD_STORAGE) {
        GameManager::getInstance()->modifyMaxGold(-1000);
    }

    BattleManager::getInstance()->removeBuilding(this);

    // 如果是主基地模式，还需要通知 GameManager 移除记录 (addHomeBuilding 的反向操作)
    // 这里暂时省略，视你是否实现了移除建筑功能而定

    GameEntity::onDeath();
}

void Building::initBuildingProperties() {
    // 1. 定义一个变量存文件名
   // 默认属性
    std::string filename = "CloseNormal.png";
    int hp = 500;
    m_productionRate = 0;
    m_maxStorage = 0;     // 这是“内部暂存”上限 (收集器用)
    m_currentStored = 0;

   
    switch (m_type) {
    case BuildingType::TOWN_HALL:
        filename = "TownHall.png";
        hp = 2000;
        break;
    case BuildingType::CANNON:
        filename = "Cannon.png";
        hp = 800;
        break;
    case BuildingType::GOLD_MINE:
        filename = "GoldMine.png";
        hp = 600;
        break;
    case BuildingType::ARCHER_TOWER:
        filename = "ArcherTower.png";
        hp = 700;
        break;
    case BuildingType::WALL:
        filename = "Wall.png";
        hp = 1000;
        break;
    case BuildingType::ELIXIR_COLLECTOR:
        filename = "ElixirCollector.png"; // 找个紫色的图
        hp = 600;
        // 核心机制：自动生产 & 内部暂存
        m_productionRate = 10.0f;  // 产出速率
        m_maxStorage = 100.0f;     // 内部暂存满了就停
        break;

    case BuildingType::ELIXIR_STORAGE:
        filename = "ElixirStorage.png"; // 找个大罐子的图
        hp = 1500; // 圣水瓶血厚
        // 核心机制：被动保管 (不生产)
        m_productionRate = 0;

        // 核心机制：增加总上限
        // 建造出来时，让大管家增加 1000 容量
        GameManager::getInstance()->modifyMaxElixir(1000);
        break;
    case BuildingType::BARRACKS:
        filename = "Barracks.png";
        hp = 1000;
        break;
    default:
        break;
    }

    // 3. 【核心修改】加载图片纹理
    this->setTexture(filename);

    // 4. 设置其他属性
    this->setProperties(hp, CampType::PLAYER);

    // 5. 【可选】调整大小 (Scale)
    // 图片可能很大(比如 500x500)，我们需要把它缩放到合适的大小(比如 64x64)
    // 假设你想让所有建筑大约占 60x60 像素：
    float targetSize = 100.0f;
    Size contentSize = this->getContentSize(); // 获取图片原始大小
    if (contentSize.width > 0) {
        this->setScale(targetSize / contentSize.width);
    }
    // 默认非资源建筑属性
    m_productionRate = 0;
    m_maxStorage = 0;
    m_currentStored = 0;
    m_productionAccumulator = 0;

    // 【新增】配置金矿属性 (数值参考 CoC 1级金矿)
    if (m_type == BuildingType::GOLD_MINE) {
        // 假设：1级金矿每小时200，容量500
        // 为了演示效果，我们调快一点：每秒产 10，容量 100
        m_productionRate = 10.0f;
        m_maxStorage = 100.0f;
    }
    else if (m_type == BuildingType::ELIXIR_COLLECTOR) {
        m_productionRate = 10.0f;
        m_maxStorage = 100.0f;
    }
}

void Building::upgrade() {
    m_level++;
    m_maxHp += 200; // 升级加血上限
    m_currentHp += 200; // 并回血
    CCLOG("Building Upgraded to Level %d! Max HP is now %d", m_level, m_maxHp);

    // 稍微变大一点表示升级
    this->setScale(this->getScale() * 1.1f);
}

void Building::updateLogic(float dt) {
    m_timer += dt;

    // 机制：自动生产 & 存满即停

      // 只有资源类建筑才生产
    // 核心机制：存满即停
    if (m_productionRate > 0) 
    {
        if (m_currentStored >= m_maxStorage) {
            m_currentStored = m_maxStorage;
            return; // 满了就不跑下面的累加代码了 -> 停止生产
        }
        // ... 累加代码 ...
   

        // 2. 累加生产
        // 我们不直接用 m_currentStored += rate * dt，因为 int 丢失精度
        // 我们用一个累加器，攒够 1 块钱再加进去
        m_productionAccumulator += m_productionRate * dt;

        if (m_productionAccumulator >= 1.0f) {
            int amountToAdd = (int)m_productionAccumulator;
            m_currentStored += amountToAdd;
            m_productionAccumulator -= amountToAdd; // 扣除已加部分

            // 再次检查上限
            if (m_currentStored >= m_maxStorage) {
                m_currentStored = m_maxStorage;
            }

            // (可选) 可以在这里更新头顶的小图标，如果满了显示“收集气泡”
        }
    }
    // 2. 【新增】防御塔逻辑
    else if (m_type == BuildingType::CANNON || m_type == BuildingType::ARCHER_TOWER) {

        // 只有冷却时间到了才攻击 (假设攻速是 1.0秒)
        if (m_timer >= 1.0f) {

            // 设定射程 (加农炮一般射程是 150-200 左右)
            float range = 20.0f;

            // 询问 BattleManager：我射程内有没有敌人？
            Troop* target = BattleManager::getInstance()->findClosestTroop(this->getPosition(), range);

            if (target) {
                // 找到了！开火！
                m_timer = 0; // 重置冷却

                // 造成伤害 (假设加农炮伤害 80)
                CCLOG("Cannon Fired at Troop!");
                target->takeDamage(80);

                // 【视觉反馈】做一个简单的闪烁效果，表示它开火了
                auto seq = Sequence::create(TintTo::create(0.1f, Color3B::ORANGE), TintTo::create(0.1f, this->getColor()), nullptr);
                this->runAction(seq);
            }
        }
    }
}

// 机制：玩家收集 & 转入金库
int Building::collectResource() {
    // 1. 如果没存货，收集个寂寞
    if (m_currentStored <= 0) return 0;

    // 2. 取出所有存货 (取整)
    int amountToCollect = (int)m_currentStored;

    // 3. 转入国库 (GameManager)
    if (m_type == BuildingType::GOLD_MINE) {
        GameManager::getInstance()->addGold(amountToCollect);
    }
    else if (m_type == BuildingType::ELIXIR_COLLECTOR) {
        GameManager::getInstance()->addElixir(amountToCollect);
    }

    // 4. 清空内部存储，恢复生产
    m_currentStored = 0.0f;
    m_productionAccumulator = 0.0f;

    CCLOG("Collected %d resources!", amountToCollect);
    return amountToCollect;
}

bool Building::isFull() const {
    return m_currentStored >= m_maxStorage;
}

int Building::getUpgradeCost() const {
    // 简单的升级价格公式：基础 200 * 等级
    return 200 * m_level;
}

void Building::showUpgradeButton() {
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
    m_upgradeBtn->addClickEventListener([this](Ref*) {
        int cost = this->getUpgradeCost();
        if (GameManager::getInstance()->getGold() >= cost) {
            GameManager::getInstance()->addGold(-cost);
            this->upgrade();
            this->hideUpgradeButton();
        }
        else {
            CCLOG("Not enough gold to upgrade");
        }
    });

    // 添加到建筑节点，这样会随着建筑移动
    this->addChild(m_upgradeBtn, 300);

    // 注册金币更新监听用于自动显示/隐藏（绑定到建筑，随建筑自动移除）
    if (!m_goldListener) {
        m_goldListener = EventListenerCustom::create("EVENT_UPDATE_GOLD", [this](EventCustom* event) {
            this->updateUpgradeButtonVisibility();
        });
        _eventDispatcher->addEventListenerWithSceneGraphPriority(m_goldListener, this);
    }
}

void Building::hideUpgradeButton() {
    if (m_upgradeBtn) {
        m_upgradeBtn->removeFromParent();
        m_upgradeBtn = nullptr;
    }

    if (m_goldListener) {
        _eventDispatcher->removeEventListener(m_goldListener);
        m_goldListener = nullptr;
    }
}

void Building::updateUpgradeButtonVisibility() {
    // Ensure we have a listener so future gold changes re-evaluate visibility
    if (!m_goldListener) {
        m_goldListener = EventListenerCustom::create("EVENT_UPDATE_GOLD", [this](EventCustom* event) {
            this->updateUpgradeButtonVisibility();
        });
        _eventDispatcher->addEventListenerWithSceneGraphPriority(m_goldListener, this);
    }

    int cost = getUpgradeCost();
    bool affordable = GameManager::getInstance()->getGold() >= cost;

    if (affordable) {
        // show if not already shown
        if (!m_upgradeBtn) {
            showUpgradeButton();
        } else {
            // update existing button text/state
            m_upgradeBtn->setTitleText(std::string("UPGRADE\n$") + std::to_string(cost));
            m_upgradeBtn->setEnabled(true);
            m_upgradeBtn->setBright(true);
        }
    }
    else {
        // hide if exists
        if (m_upgradeBtn) {
            hideUpgradeButton();
        }
    }
}