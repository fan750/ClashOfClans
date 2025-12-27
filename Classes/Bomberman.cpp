// Bomberman.cpp
#include "Bomberman.h"
#include "BattleManager.h"
USING_NS_CC;

Bomberman::Bomberman()
{
    m_type = TroopType::BOMBERMAN;
}

void Bomberman::initProperties()
{
    m_cost = 2;
    m_minBarrackLevel = 2;
    m_movementType = Troop::MovementType::GROUND;

    m_baseHp = 40;
    m_baseDamage = 500; // 爆炸伤害高
    m_moveSpeed = 150.0f;
    m_attackRange = 20.0f;
    m_attackInterval = 0.1f;

    this->setHpBarOffsetY(500.0f);
    this->setHpBarOffsetX(450.0f);
    m_hpBarWidth = 15.0f;
    m_hpBarHeight = 10.0f;
}

void Bomberman::initAnimations()
{
    m_walkPlist = "hero4-walk.plist";
    m_attackPlist = ""; // 没有攻击动画，直接爆炸

    if (!m_walkPlist.empty()) SpriteFrameCache::getInstance()->addSpriteFramesWithFile(m_walkPlist);

    Vector<SpriteFrame*> frames;
    ValueMap vm = FileUtils::getInstance()->getValueMapFromFile(m_walkPlist);
    if (vm.find("frames") != vm.end()) {
        auto framesMap = vm["frames"].asValueMap();
        std::vector<std::pair<int, std::string>> keyed;
        for (const auto& kv : framesMap) {
            int num = 0;
            sscanf(kv.first.c_str(), "%d", &num);
            keyed.emplace_back(num, kv.first);
        }
        std::sort(keyed.begin(), keyed.end());
        for (const auto& p : keyed) {
            if (auto f = SpriteFrameCache::getInstance()->getSpriteFrameByName(p.second))
                frames.pushBack(f);
        }
    }

    if (!frames.empty()) {
        auto animation = Animation::createWithSpriteFrames(frames, 0.08f);
        std::string uniqueWalkKey = StringUtils::format("troop_walk_anim_%d", (int)m_type);
        AnimationCache::getInstance()->addAnimation(animation, uniqueWalkKey);

        float scaleFactor = 1.0f / 20.0f;
        this->setScale(scaleFactor);
        m_baseScale = this->getScale();

        const int WALK_ACTION_TAG = 0x1001;

        // 修正点：不要链式调用 ->setTag
        auto animate = Animate::create(animation);
        auto walkAction = RepeatForever::create(animate);
        walkAction->setTag(WALK_ACTION_TAG);
        this->runAction(walkAction);
    }
    else {
        // Fallback action (如果读取不到plist)
        const int WALK_ACTION_TAG = 0x1001;
        auto fallbackAction = RepeatForever::create(Sequence::create(ScaleTo::create(0.4f, 1.2f), ScaleTo::create(0.4f, 1.0f), nullptr));
        fallbackAction->setTag(WALK_ACTION_TAG);
        this->runAction(fallbackAction);
    }
}

void Bomberman::acquireTarget()
{
    // 炸弹人只找墙
    m_target = BattleManager::getInstance()->findClosestBuildingOfType(this->getPosition(), BuildingType::WALL);
}

void Bomberman::performAttackBehavior()
{
    CCLOG("Bomberman Exploded!");
    m_isAttacking = true; // 防止重复

    // 爆炸动画序列
    auto seq = Sequence::create
    (
        // 1. 先变大变红 (0.1s)
        Spawn::create(ScaleTo::create(0.1f, m_baseScale * 3.0f), TintTo::create(0.1f, Color3B::RED), nullptr),
        // 2. 回调：造成伤害并死亡
        CallFunc::create([this]() {
            // 将伤害逻辑延迟到动画结束后执行,避免了因移除其他单位导致的迭代器失效崩溃
            BattleManager::getInstance()->dealAreaDamage(this->getPosition(), 100.0f, m_damage);
            
            this->m_currentHp = 0;
            this->onDeath();
        }),
        nullptr
    );
    this->runAction(seq);
}