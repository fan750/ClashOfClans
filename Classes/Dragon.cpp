// Dragon.cpp
#include "Dragon.h"

USING_NS_CC;

Dragon::Dragon()
{
    m_type = TroopType::DRAGON;
}

void Dragon::initProperties()
{
    m_cost = 5;
    m_minBarrackLevel = 3;
    m_movementType = Troop::MovementType::AIR; // 飞龙是空军

    m_baseHp = 300;
    m_baseDamage = 60;
    m_moveSpeed = 100.0f;
    m_attackRange = 120.0f;
    m_attackInterval = 1.5f;

    this->setHpBarOffsetY(-200.0f);
    this->setHpBarOffsetX(-220.0f);
    m_hpBarWidth = 4.0f;
    m_hpBarHeight = 3.0f;
}

void Dragon::initAnimations()
{
    m_walkPlist = "dragon.plist";
    m_attackPlist = "dragon.plist";

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
        this->setScale(scaleFactor * 8.0f); // 飞龙很大
        m_baseScale = this->getScale();

        const int WALK_ACTION_TAG = 0x1001;

        auto animate = Animate::create(animation);
        auto walkAction = RepeatForever::create(animate);
        walkAction->setTag(WALK_ACTION_TAG);
        this->runAction(walkAction);
    }
}