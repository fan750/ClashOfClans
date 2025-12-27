// Giant.cpp
#include "Giant.h"

USING_NS_CC;

Giant::Giant()
{
    m_type = TroopType::GIANT;
}

void Giant::initProperties()
{
    m_cost = 3;
    m_minBarrackLevel = 2;
    m_movementType = Troop::MovementType::GROUND;

    m_baseHp = 200;
    m_baseDamage = 40;
    m_moveSpeed = 80.0f;
    m_attackRange = 40.0f;
    m_attackInterval = 1.5f;

    this->setHpBarOffsetY(200.0f);
    this->setHpBarOffsetX(450.0f);
    m_hpBarWidth = 4.0f;
    m_hpBarHeight = 4.0f;
}

void Giant::initAnimations()
{
    m_walkPlist = "hero3-walk.plist";
    m_attackPlist = "hero3-attack.plist";

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
        this->setScale(scaleFactor * 4.0f); // ¾ÞÈËºÜ´ó
        m_baseScale = this->getScale();

        const int WALK_ACTION_TAG = 0x1001;

        auto animate = Animate::create(animation);
        auto walkAction = RepeatForever::create(animate);
        walkAction->setTag(WALK_ACTION_TAG);
        this->runAction(walkAction);
    }
}