// Archer.cpp
#include "Archer.h"

USING_NS_CC;

Archer::Archer()
{
    m_type = TroopType::ARCHER;
}

void Archer::initProperties()
{
    m_cost = 1;
    m_minBarrackLevel = 1;
    m_movementType = Troop::MovementType::GROUND;

    m_baseHp = 60;
    m_baseDamage = 25;
    m_moveSpeed = 120.0f;
    m_attackRange = 150.0f; // 远程
    m_attackInterval = 0.8f;

    this->setHpBarOffsetY(500.0f);
    this->setHpBarOffsetX(450.0f);
    m_hpBarWidth = 15.0f;
    m_hpBarHeight = 10.0f;
}

void Archer::initAnimations()//动画
{
    m_walkPlist = "hero2-walk.plist";
    m_attackPlist = "hero2-attack.plist";

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

        auto animate = Animate::create(animation);
        auto walkAction = RepeatForever::create(animate);
        walkAction->setTag(WALK_ACTION_TAG);
        this->runAction(walkAction);
    }
}

void Archer::performAttackBehavior()
{
    // 1. 造成伤害
    m_target->takeDamage(m_damage);

    // 2. 弓箭手特效：生成箭矢
    Vec2 targetPos = m_target->getPosition();
    auto arrow = Sprite::create();
    arrow->setTextureRect(Rect(0, 0, 10, 2));
    arrow->setColor(Color3B::YELLOW);
    arrow->setPosition(this->getPosition());
    this->getParent()->addChild(arrow);

    float distance = this->getPosition().distance(targetPos);
    float duration = distance / 400.0f;
    arrow->runAction(Sequence::create(MoveTo::create(duration, targetPos), RemoveSelf::create(), nullptr));
}