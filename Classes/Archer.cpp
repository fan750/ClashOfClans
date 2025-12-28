#include "Archer.h"

USING_NS_CC;

// 构造函数
Archer::Archer()
{
    m_type = TroopType::ARCHER;
}

// 初始化兵种属性
void Archer::initProperties()
{
    // 设置基础属性
    m_cost = 1;
    m_minBarrackLevel = 1;
    m_movementType = Troop::MovementType::GROUND;

    // 设置战斗数值
    m_baseHp = 60;
    m_baseDamage = 25;
    m_moveSpeed = 120.0f;
    m_attackRange = 150.0f; // 远程攻击
    m_attackInterval = 0.8f;

    // 设置血条样式与偏移
    this->setHpBarOffsetY(500.0f);
    this->setHpBarOffsetX(450.0f);
    m_hpBarWidth = 15.0f;
    m_hpBarHeight = 10.0f;
}

// 初始化动画资源
void Archer::initAnimations()
{
    // 加载动画资源文件
    m_walkPlist = "hero2-walk.plist";
    m_attackPlist = "hero2-attack.plist";

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

        // 设置缩放并运行动作
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

// 执行远程攻击行为
void Archer::performAttackBehavior()
{
    // 造成伤害
    m_target->takeDamage(m_damage);

    // 弓箭手特效：生成箭矢
    Vec2 targetPos = m_target->getPosition();
    auto arrow = Sprite::create();
    arrow->setTextureRect(Rect(0, 0, 10, 2));  // 设置箭矢形状
    arrow->setColor(Color3B::YELLOW);          // 设置箭矢颜色
    arrow->setPosition(this->getPosition());
    this->getParent()->addChild(arrow);

    // 计算飞行时间并移动箭矢
    float distance = this->getPosition().distance(targetPos);
    float duration = distance / 400.0f;
    arrow->runAction(Sequence::create(MoveTo::create(duration, targetPos), RemoveSelf::create(), nullptr));
}