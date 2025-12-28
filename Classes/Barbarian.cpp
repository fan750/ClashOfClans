#include "Barbarian.h"

USING_NS_CC;

// 构造函数
Barbarian::Barbarian()
{
    m_type = TroopType::BARBARIAN;
}

// 初始化兵种属性
void Barbarian::initProperties()
{
    // 设置兵种属性
    m_cost = 1;
    m_minBarrackLevel = 1;
    m_movementType = Troop::MovementType::GROUND;

    // 设置战斗数值
    m_baseHp = 100;
    m_baseDamage = 50;
    m_moveSpeed = 100.0f;
    m_attackRange = 40.0f;
    m_attackInterval = 1.0f;

    // 设置视觉属性
    this->setHpBarOffsetY(500.0f);
    this->setHpBarOffsetX(450.0f);
    m_hpBarWidth = 15.0f;
    m_hpBarHeight = 10.0f;
}

// 初始化动画资源
void Barbarian::initAnimations()
{
    // 设定资源名
    m_walkPlist = "hero1-walk.plist";
    m_attackPlist = "hero1-attack.plist";

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
            auto f = SpriteFrameCache::getInstance()->getSpriteFrameByName(p.second);
            if (f)
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
        auto animate = Animate::create(animation);

        // 计算缩放并运行动作
        float scaleFactor = 1.0f / 20.0f;
        this->setScale(this->getScale() * scaleFactor);
        m_baseScale = this->getScale();

        const int WALK_ACTION_TAG = 0x1001;
        auto walkAction = RepeatForever::create(animate);
        walkAction->setTag(WALK_ACTION_TAG);
        this->runAction(walkAction);
    }
}