#include "Bomberman.h"
#include "BattleManager.h"
USING_NS_CC;

// 构造函数
Bomberman::Bomberman()
{
    m_type = TroopType::BOMBERMAN;
}

// 初始化兵种属性
void Bomberman::initProperties()
{
    // 设置基础属性
    m_cost = 2;
    m_minBarrackLevel = 2;
    m_movementType = Troop::MovementType::GROUND;

    // 设置战斗数值（爆炸伤害高）
    m_baseHp = 40;
    m_baseDamage = 500;
    m_moveSpeed = 150.0f;
    m_attackRange = 20.0f;
    m_attackInterval = 0.1f;

    // 设置血条样式与偏移
    this->setHpBarOffsetY(500.0f);
    this->setHpBarOffsetX(450.0f);
    m_hpBarWidth = 15.0f;
    m_hpBarHeight = 10.0f;
}

// 初始化动画资源
void Bomberman::initAnimations()
{
    // 设定资源名（无攻击动画，直接爆炸）
    m_walkPlist = "hero4-walk.plist";
    m_attackPlist = "";

    if (!m_walkPlist.empty())
    {
        SpriteFrameCache::getInstance()->addSpriteFramesWithFile(m_walkPlist);
    }

    Vector<SpriteFrame*> frames;
    ValueMap vm = FileUtils::getInstance()->getValueMapFromFile(m_walkPlist);
    if (vm.find("frames") != vm.end())
    {
        // 解析帧数据
        auto framesMap = vm["frames"].asValueMap();
        std::vector<std::pair<int, std::string>> keyed;
        for (const auto& kv : framesMap)
        {
            int num = 0;
            sscanf(kv.first.c_str(), "%d", &num);
            keyed.emplace_back(num, kv.first);
        }
        // 对帧名进行排序
        std::sort(keyed.begin(), keyed.end());
        for (const auto& p : keyed)
        {
            if (auto f = SpriteFrameCache::getInstance()->getSpriteFrameByName(p.second))
            {
                frames.pushBack(f);
            }
        }
    }

    if (!frames.empty())
    {
        // 创建并缓存行走动画
        auto animation = Animation::createWithSpriteFrames(frames, 0.08f);
        std::string uniqueWalkKey = StringUtils::format("troop_walk_anim_%d", (int)m_type);
        AnimationCache::getInstance()->addAnimation(animation, uniqueWalkKey);

        // 设置缩放
        float scaleFactor = 1.0f / 20.0f;
        this->setScale(scaleFactor);
        m_baseScale = this->getScale();

        const int WALK_ACTION_TAG = 0x1001;

        // 避免链式调用 ->setTag，先赋值再执行
        auto animate = Animate::create(animation);
        auto walkAction = RepeatForever::create(animate);
        walkAction->setTag(WALK_ACTION_TAG);
        this->runAction(walkAction);
    }
    else
    {
        // Fallback action (如果读取不到plist)
        const int WALK_ACTION_TAG = 0x1001;
        auto fallbackAction = RepeatForever::create(Sequence::create(ScaleTo::create(0.4f, 1.2f), ScaleTo::create(0.4f, 1.0f), nullptr));
        fallbackAction->setTag(WALK_ACTION_TAG);
        this->runAction(fallbackAction);
    }
}

// 重写索敌逻辑：炸弹人优先攻击围墙
void Bomberman::acquireTarget()
{
    m_target = BattleManager::getInstance()->findClosestBuildingOfType(this->getPosition(), BuildingType::WALL);
}

// 重写攻击行为：炸弹人自爆造成范围伤害
void Bomberman::performAttackBehavior()
{
    // 标记为正在攻击，防止重复触发
    m_isAttacking = true;

    // 爆炸动画序列
    auto seq = Sequence::create
    (
        // 先变大变红 (0.1s)
        Spawn::create(ScaleTo::create(0.1f, m_baseScale * 3.0f), TintTo::create(0.1f, Color3B::RED), nullptr),
        // 回调：造成伤害并死亡
        CallFunc::create([this]()
            {
                // 将伤害逻辑延迟到动画结束后执行，避免因移除其他单位导致的迭代器失效崩溃
                BattleManager::getInstance()->dealAreaDamage(this->getPosition(), 100.0f, m_damage);

                this->m_currentHp = 0;
                this->onDeath();
            }),
        nullptr
    );
    this->runAction(seq);
}