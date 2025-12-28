#include "Troop.h"
#include "BattleManager.h"
#include "GameManager.h"
#include "Barbarian.h" // 包含所有子类头文件以便工厂创建
#include "Archer.h"
#include "Giant.h"
#include "Bomberman.h"
#include "Dragon.h"
USING_NS_CC;

// 定义静态成员变量
std::map<TroopType, std::string> Troop::s_staticNames;
std::map<TroopType, int> Troop::s_staticCosts;
std::map<TroopType, int> Troop::s_staticMinLevels;

// 初始化静态数据表
// 此处的数值与各子类 initProperties 中的数值保持一致
void Troop::initStaticData()
{
    // 初始化野蛮人数据
    s_staticNames[TroopType::BARBARIAN] = "Barbarian";
    s_staticCosts[TroopType::BARBARIAN] = 1;
    s_staticMinLevels[TroopType::BARBARIAN] = 1;

    // 初始化弓箭手数据
    s_staticNames[TroopType::ARCHER] = "Archer";
    s_staticCosts[TroopType::ARCHER] = 1;
    s_staticMinLevels[TroopType::ARCHER] = 1;

    // 初始化巨人数据
    s_staticNames[TroopType::GIANT] = "Giant";
    s_staticCosts[TroopType::GIANT] = 3;
    s_staticMinLevels[TroopType::GIANT] = 2;

    // 初始化炸弹人数据
    s_staticNames[TroopType::BOMBERMAN] = "Bomberman";
    s_staticCosts[TroopType::BOMBERMAN] = 2;
    s_staticMinLevels[TroopType::BOMBERMAN] = 2;

    // 初始化飞龙数据
    s_staticNames[TroopType::DRAGON] = "Dragon";
    s_staticCosts[TroopType::DRAGON] = 5;
    s_staticMinLevels[TroopType::DRAGON] = 3;
}

// 利用 C++ 的“静态构造”技巧，确保在第一次使用前自动初始化
struct TroopStaticInitializer
{
    TroopStaticInitializer()
    {
        Troop::initStaticData();
    }
};

// 定义一个静态变量，全局只构造一次
static TroopStaticInitializer g_troopInitializer;

// 工厂方法：根据类型创建具体的兵种子类
Troop* Troop::create(TroopType type)
{
    Troop* pRet = nullptr;
    switch (type)
    {
    case TroopType::BARBARIAN:  pRet = new Barbarian(); break;
    case TroopType::ARCHER:     pRet = new Archer();    break;
    case TroopType::GIANT:      pRet = new Giant();     break;
    case TroopType::BOMBERMAN:  pRet = new Bomberman(); break;
    case TroopType::DRAGON:     pRet = new Dragon();    break;
    }

    if (pRet && pRet->init())
    {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

// 构造函数
Troop::Troop()
    : m_type(TroopType::BARBARIAN)
    , m_movementType(GROUND)
    , m_cost(0)
    , m_minBarrackLevel(1)
    , m_moveSpeed(0.0f)
    , m_attackRange(0.0f)
    , m_damage(0)
    , m_attackInterval(1.0f)
    , m_attackTimer(0.0f)
    , m_target(nullptr)
    , m_baseScale(1.0f)
    , m_isAttacking(false)
    , m_baseHp(0)
    , m_baseDamage(0)
{}

// 析构函数
Troop::~Troop() {}

// 初始化逻辑
bool Troop::init()
{
    if (!GameEntity::init())
    {
        return false;
    }

    // 调用纯虚函数，由具体子类填入数值和资源
    initProperties();
    initAnimations();

    // 根据类型获取等级并重新计算属性
    int level = getLevelForType(m_type);
    rescaleStatsForLevel(level);

    // 血条样式设置 (保持通用)
    if (m_hpBgSprite)
    {
        Size bgSize = m_hpBgSprite->getContentSize();
        if (bgSize.width > 0 && bgSize.height > 0)
        {
            float sx = m_hpBarWidth / 10.0f;
            float sy = m_hpBarHeight / 10.0f;
            m_hpBgSprite->setScale(sx, sy);
        }
    }

    if (m_hpBarTimer && m_hpBarTimer->getSprite())
    {
        Size fillSize = m_hpBarTimer->getSprite()->getContentSize();
        if (fillSize.width > 0 && fillSize.height > 0)
        {
            float sx = m_hpBarWidth / 10.0f;
            float sy = m_hpBarHeight / 10.0f;
            m_hpBarTimer->setScale(sx, sy);
        }
    }

    // 注册到战斗管理器并开启更新
    BattleManager::getInstance()->addTroop(this);
    this->scheduleUpdate();

    return true;
}

// 死亡逻辑
void Troop::onDeath()
{
    BattleManager::getInstance()->removeTroop(this);
    GameEntity::onDeath();
}

// --- 通用逻辑 ---

// 更新逻辑：索敌、移动或攻击
void Troop::updateLogic(float dt)
{
    // 检查目标是否存在或已死亡
    if (!m_target || m_target->isDead())
    {
        m_target = nullptr;
        acquireTarget(); // 调用虚函数，炸弹人会重写以仅攻击墙
        if (!m_target)
        {
            return;
        }
    }

    Vec2 myPos = this->getPosition();
    Vec2 targetPos = m_target->getPosition();
    float distance = myPos.distance(targetPos);

    if (distance <= m_attackRange)
    {
        attackTarget(dt);
    }
    else
    {
        moveTowardsTarget(dt);
    }
}

// 默认索敌行为：找最近的建筑
void Troop::acquireTarget()
{
    m_target = BattleManager::getInstance()->findClosestBuilding(this->getPosition());
}

// 向目标移动
void Troop::moveTowardsTarget(float dt)
{
    if (!m_target)
    {
        return;
    }

    Vec2 myPos = this->getPosition();
    Vec2 targetPos = m_target->getPosition();

    // 计算方向向量并归一化
    Vec2 direction = targetPos - myPos;
    direction.normalize();

    // 根据速度和时间计算新位置
    Vec2 newPos = myPos + direction * m_moveSpeed * dt;
    this->setPosition(newPos);
}

// 攻击目标
void Troop::attackTarget(float dt)
{
    m_attackTimer += dt;

    // 检查攻击冷却时间
    if (m_attackTimer >= m_attackInterval)
    {
        m_attackTimer = 0;

        if (!m_target || m_target->getCurrentHp() <= 0)
        {
            this->m_target = nullptr;
            return;
        }

        if (m_isAttacking)
        {
            return;
        }

        // 调用子类实现的攻击行为
        performAttackBehavior();
    }
}

// 默认攻击行为：近战伤害
// 弓箭手、炸弹人、飞龙会重写该函数
void Troop::performAttackBehavior()
{
    // 造成伤害
    m_target->takeDamage(m_damage);

    // 默认近战动画
    m_isAttacking = true;
    const int WALK_ACTION_TAG = 0x1001;
    this->stopActionByTag(WALK_ACTION_TAG);

    std::string attackPlist = m_attackPlist;
    if (!attackPlist.empty())
    {
        SpriteFrameCache::getInstance()->addSpriteFramesWithFile(attackPlist);
    }

    Vector<SpriteFrame*> attackFrames;
    if (!attackPlist.empty())
    {
        ValueMap avm = FileUtils::getInstance()->getValueMapFromFile(attackPlist);
        if (avm.find("frames") != avm.end())
        {
            // 获取并排序帧名
            auto framesMap = avm["frames"].asValueMap();
            std::vector<std::string> keys;
            keys.reserve(framesMap.size());
            for (const auto& kv : framesMap)
            {
                keys.push_back(kv.first);
            }
            std::sort(keys.begin(), keys.end());
            for (const auto& name : keys)
            {
                auto f = SpriteFrameCache::getInstance()->getSpriteFrameByName(name);
                if (f)
                {
                    attackFrames.pushBack(f);
                }
            }
        }
    }

    if (!attackFrames.empty())
    {
        // 播放攻击动画序列
        auto attackAnim = Animation::createWithSpriteFrames(attackFrames, 0.08f);
        auto attackAnimate = Animate::create(attackAnim);

        // 攻击结束后恢复行走动画和状态
        auto restore = CallFunc::create([this, WALK_ACTION_TAG]()
            {
                std::string uniqueWalkKey = StringUtils::format("troop_walk_anim_%d", (int)this->m_type);
                auto walkAnim = AnimationCache::getInstance()->getAnimation(uniqueWalkKey);
                if (walkAnim)
                {
                    auto repeat = RepeatForever::create(Animate::create(walkAnim));
                    repeat->setTag(WALK_ACTION_TAG);
                    this->runAction(repeat);
                }
                this->setScale(m_baseScale);
                m_isAttacking = false;
            });

        this->runAction(Sequence::create(attackAnimate, restore, nullptr));
    }
    else
    {
        // 没有动画资源时的默认缩放效果
        auto up = ScaleTo::create(0.1f, m_baseScale * 1.2f);
        auto down = ScaleTo::create(0.1f, m_baseScale);
        auto seq = Sequence::create(up, down, CallFunc::create([this, WALK_ACTION_TAG]()
            {
                std::string uniqueWalkKey = StringUtils::format("troop_walk_anim_%d", (int)this->m_type);
                auto walkAnim = AnimationCache::getInstance()->getAnimation(uniqueWalkKey);
                if (walkAnim)
                {
                    auto repeat = RepeatForever::create(Animate::create(walkAnim));
                    repeat->setTag(WALK_ACTION_TAG);
                    this->runAction(repeat);
                }
                m_isAttacking = false;
            }), nullptr);
        this->runAction(seq);
    }
}

// --- 静态辅助函数实现 ---
// 用于UI层在不创建实际战斗单位的情况下获取配置信息
std::string Troop::getStaticTroopName(TroopType type)
{
    auto it = s_staticNames.find(type);
    if (it != s_staticNames.end())
    {
        return it->second;
    }
    return "Unknown";
}

// 获取静态兵种Cost
int Troop::getStaticTroopCost(TroopType type)
{
    auto it = s_staticCosts.find(type);
    if (it != s_staticCosts.end())
    {
        return it->second;
    }
    return 0;
}

// 获取静态兵种最低军营等级
int Troop::getStaticTroopMinLevel(TroopType type)
{
    auto it = s_staticMinLevels.find(type);
    if (it != s_staticMinLevels.end())
    {
        return it->second;
    }
    return 1; // 默认为 1 级
}

// 根据等级重新计算属性
void Troop::rescaleStatsForLevel(int level)
{
    if (level <= 0)
    {
        level = 1;
    }
    float multiplier = getLevelMultiplier(level);

    // 计算新的最大血量
    int newMaxHp = std::max(1, static_cast<int>(std::round(static_cast<float>(m_baseHp) * multiplier)));
    int oldMaxHp = m_maxHp;

    // 计算血量比例，以便在升级后保持当前血量比例
    float ratio = (oldMaxHp > 0) ? static_cast<float>(m_currentHp) / static_cast<float>(oldMaxHp) : 1.0f;
    ratio = std::max(0.0f, std::min(1.0f, ratio));

    GameEntity::setProperties(newMaxHp, CampType::ENEMY);
    m_currentHp = std::max(1, std::min(newMaxHp, static_cast<int>(std::round(ratio * newMaxHp))));

    // 计算新的伤害值
    m_damage = std::max(1, static_cast<int>(std::round(static_cast<float>(m_baseDamage) * multiplier)));
}

// 获取特定兵种的等级
int Troop::getLevelForType(TroopType type) const
{
    auto gm = GameManager::getInstance();
    switch (type)
    {
    case TroopType::BARBARIAN:  return gm->getBarLevel();  
    case TroopType::ARCHER:     return gm->getArcLevel();
    case TroopType::GIANT:      return gm->getGiantLevel(); 
    case TroopType::BOMBERMAN:  return gm->getBomLevel();
    case TroopType::DRAGON:     return gm->getDragonLevel();
    default: return 1;
    }
}

// 获取等级加成倍率
float Troop::getLevelMultiplier(int level) const
{
    if (level <= 1)
    {
        return 1.0f;
    }
    return 1.0f + 0.1f * static_cast<float>(level - 1);
}