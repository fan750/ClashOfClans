//Troop.cpp
// 修复：使用 cocos2d::Vector 而非 std::vector，且这些方法是成员函数
#include "Troop.h"
#include "BattleManager.h"
#include <algorithm>
#include <fstream> // ensure ofstream is defined

USING_NS_CC;

Troop::Troop()
    : m_type(TroopType::BARBARIAN)
    , m_movementType(TroopMovementType::GROUND)
    , m_moveSpeed(0.0f)
    , m_attackRange(0.0f)
    , m_damage(0)
    , m_attackInterval(1.0f)
    , m_attackTimer(0.0f)
    , m_target(nullptr)
    , m_baseScale(1.0f)
    , m_isAttacking(false)
{
}

Troop::~Troop() {
}

Troop* Troop::create(TroopType type)
{
    Troop* pRet = new(std::nothrow) Troop();
    if (pRet)
    {
        pRet->m_type = type;
        if (pRet->init())
        {
            pRet->autorelease();
            return pRet;
        }
    }
    delete pRet;
    return nullptr;
}

static int extractFirstNumber(const std::string& s)
{
    int n = 0;
    bool found = false;
    for (char c : s)
    {
        if (c >= '0' && c <= '9')
        {
            found = true;
            n = n * 10 + (c - '0');
        }
        else if (found)
        {
            break;
        }
    }
    return n;
}

bool Troop::init()
{
    if (!GameEntity::init()) return false;

    initTroopProperties();
    const float DEFAULT_HP_BAR_WIDTH = 300.0f;  // 可根据 UI 需求调整为合适像素
    const float DEFAULT_HP_BAR_HEIGHT = 120.0f;  // 宽高比可自定义

    m_hpBarWidth = DEFAULT_HP_BAR_WIDTH;
    m_hpBarHeight = DEFAULT_HP_BAR_HEIGHT;

    // 【新增】飞龙单独重设血条大小：保持长宽比，缩小到当前的 1/3
    if (m_type == TroopType::DRAGON) {
        m_hpBarWidth *= (1.0f / 3.0f);
        m_hpBarHeight *= (1.0f / 3.0f);
    }
    // decide plist names per troop type
    std::string walkPlist;
    std::string attackPlist;
    switch (m_type)
    {
    case TroopType::BARBARIAN: walkPlist = "hero1-walk.plist";  attackPlist = "hero1-attack.plist";  break;
    case TroopType::ARCHER:    walkPlist = "hero2-walk.plist";  attackPlist = "hero2-attack.plist";  break;
    case TroopType::GIANT:     walkPlist = "hero3-walk.plist";  attackPlist = "hero3-attack.plist";  break;
    case TroopType::BOMBERMAN: walkPlist = "hero4-walk.plist";  attackPlist = "";                    break;
    case TroopType::DRAGON:    walkPlist = "dragon.plist";      attackPlist = "dragon.plist";        break;
    }

    // store in instance members
    m_walkPlist = walkPlist;
    m_attackPlist = attackPlist;

    // load walk plist
    if (!walkPlist.empty()) SpriteFrameCache::getInstance()->addSpriteFramesWithFile(walkPlist);
    CCLOG("Troop::init - attempted to load walk plist: %s", walkPlist.c_str());

    // Read frames from plist 'frames' and sort by numeric suffix
    Vector<SpriteFrame*> frames;
    ValueMap vm = FileUtils::getInstance()->getValueMapFromFile(walkPlist);
    if (vm.find("frames") != vm.end()) {
        auto framesMap = vm["frames"].asValueMap();
        std::vector<std::pair<int, std::string>> keyed;
        keyed.reserve(framesMap.size());
        for (const auto& kv : framesMap) {
            int num = extractFirstNumber(kv.first);
            keyed.emplace_back(num, kv.first);
        }
        std::sort(keyed.begin(), keyed.end(), [](const std::pair<int, std::string>& a, const std::pair<int, std::string>& b) {
            if (a.first != b.first) return a.first < b.first;
            return a.second < b.second;
            });
        for (const auto& p : keyed) {
            auto f = SpriteFrameCache::getInstance()->getSpriteFrameByName(p.second);
            if (f) {
                frames.pushBack(f);
                CCLOG("Troop::init - walk frame added: %s", p.second.c_str());
            }
            else {
                CCLOG("Troop::init - walk frame not in cache: %s", p.second.c_str());
            }
        }
    }
    else {
        CCLOG("Troop::init - walk plist has no 'frames' key: %s", walkPlist.c_str());
    }

    CCLOG("Troop::init - total walk frames: %u", (unsigned)frames.size());

    // write debug file with chosen plists
    std::string writable = FileUtils::getInstance()->getWritablePath();
    std::string logPath = writable + "troop_debug.txt";
    std::ofstream os(logPath, std::ios::app);
    if (os.is_open()) {
        os << "Troop::init debug:\n";
        os << "walkPlist: " << walkPlist << "\n";
        os << "attackPlist: " << attackPlist << "\n";
        os << "Total walk frames: " << frames.size() << "\n";
        os << "plist full path: " << FileUtils::getInstance()->fullPathForFilename(walkPlist) << "\n";
        os << "---------------------------------------------\n";
        os.close();
        CCLOG("Troop::init - wrote debug file to %s", logPath.c_str());
    }

    // start walk animation if frames present
    if (!frames.empty()) {
        auto animation = Animation::createWithSpriteFrames(frames, 0.08f);
        AnimationCache::getInstance()->addAnimation(animation, "troop_walk_anim");
        auto animate = Animate::create(animation);

        float scaleFactor = 1.0f / 20.0f;
        this->setScale(this->getScale() * scaleFactor);
        
        // 【新增】按兵种单独调整大小
        if (m_type == TroopType::DRAGON) {
            this->setScale(this->getScale() * 4.0f);
        } else if (m_type == TroopType::GIANT) {
            this->setScale(this->getScale() * 2.0f);
        }

        m_baseScale = this->getScale();
        CCLOG("Troop::init - applied scale factor %f, baseScale=%f", scaleFactor, m_baseScale);

        const int WALK_ACTION_TAG = 0x1001;
        auto walkAction = RepeatForever::create(animate);
        walkAction->setTag(WALK_ACTION_TAG);
        this->runAction(walkAction);
    }
    else {
        CCLOG("Troop::init - no walk frames, running fallback action");
        auto seq = Sequence::create(ScaleTo::create(0.4f, 1.2f), ScaleTo::create(0.4f, 1.0f), nullptr);
        this->runAction(RepeatForever::create(seq));
    }

    BattleManager::getInstance()->addTroop(this);
    this->scheduleUpdate();
    return true;
}

void Troop::onDeath()
{
    BattleManager::getInstance()->removeTroop(this);
    GameEntity::onDeath();
}

void Troop::initTroopProperties() {
    // 默认设置
    this->setTextureRect(Rect(0, 0, 20, 20));

    if (m_type == TroopType::BARBARIAN)
    {
        // keep original sprite color
        setProperties(100, CampType::ENEMY);
        m_moveSpeed = 100.0f;
        m_attackRange = 40.0f;
        m_damage = 50;
        m_attackInterval = 1.0f;
        m_movementType = TroopMovementType::GROUND;
    }
    else if (m_type == TroopType::ARCHER)
    {
        // keep original sprite color
        this->setTextureRect(Rect(0, 0, 15, 20));
        setProperties(60, CampType::ENEMY);
        m_moveSpeed = 120.0f;
        m_attackRange = 150.0f;
        m_damage = 25;
        m_attackInterval = 0.8f;
        m_movementType = TroopMovementType::GROUND;
    }
    else if (m_type == TroopType::GIANT)
    {
        // keep original sprite color
        this->setTextureRect(Rect(0, 0, 40, 40));
        setProperties(200, CampType::ENEMY);
        m_moveSpeed = 80.0f;
        m_attackRange = 40.0f;
        m_damage = 40;
        m_attackInterval = 1.5f;
        m_movementType = TroopMovementType::GROUND;
    }
    else if (m_type == TroopType::BOMBERMAN)
    {
        // keep original sprite color
        this->setTextureRect(Rect(0, 0, 15, 15));
        setProperties(40, CampType::ENEMY);
        m_moveSpeed = 150.0f;
        m_attackRange = 20.0f;
        m_damage = 500;
        m_attackInterval = 0.1f;
        m_movementType = TroopMovementType::GROUND;
    }
    else if (m_type == TroopType::DRAGON)
    {
        // 可以用一张龙的图片代替
        this->setTextureRect(Rect(0, 0, 50, 50));
        setProperties(300, CampType::ENEMY);
        m_moveSpeed = 100.0f;
        m_attackRange = 120.0f;
        m_damage = 60; // 中等伤害
        m_attackInterval = 1.5f;
        m_movementType = TroopMovementType::AIR; // 设置为空中单位
    }
}

void Troop::setTarget(Building* target)
{
    m_target = target;
}

void Troop::updateLogic(float dt) {
    // 索敌逻辑
    if (!m_target || m_target->isDead()) {
        m_target = nullptr;

        if (m_type == TroopType::BOMBERMAN) {
            // 炸弹人只找墙
            m_target = BattleManager::getInstance()->findClosestBuildingOfType(this->getPosition(), BuildingType::WALL);
        }
        else {
            // 其他人找最近的
            m_target = BattleManager::getInstance()->findClosestBuilding(this->getPosition());
        }

        if (!m_target) return;
    }

    Vec2 myPos = this->getPosition();
    Vec2 targetPos = m_target->getPosition();
    float distance = myPos.distance(targetPos);

    if (distance <= m_attackRange) {
        attackTarget(dt);
    }
    else {
        moveTowardsTarget(dt);
    }
}

void Troop::moveTowardsTarget(float dt) {
    if (!m_target) return;

    // 【回滚到最简单的逻辑】直接走直线，不搞那些复杂的防重叠了
    Vec2 myPos = this->getPosition();
    Vec2 targetPos = m_target->getPosition();

    Vec2 direction = targetPos - myPos;
    direction.normalize();

    Vec2 newPos = myPos + direction * m_moveSpeed * dt;
    this->setPosition(newPos);
}

void Troop::attackTarget(float dt) 
{
    m_attackTimer += dt;

    if (m_attackTimer >= m_attackInterval) 
    {
        m_attackTimer = 0;

        // 1. 如果目标指针为空，直接返回
        if (!m_target) 
        {
            return;
        }

        // 2. 如果目标逻辑上已经死亡
        if (m_target->getCurrentHp() <= 0) {
            this->setTarget(nullptr);
            return;
        }

        if (m_isAttacking) return; // already attacking

        if (m_type == TroopType::BOMBERMAN)
        {
            CCLOG("Bomberman Exploded!");
            Vec2 explosionCenter = this->getPosition();

            // 注意：这里使用了传入 'this' 的版本，确保 BattleManager::dealAreaDamage 支持第4个参数
            BattleManager::getInstance()->dealAreaDamage(explosionCenter, 100.0f, m_damage);

            // 防止重复触发攻击
            m_isAttacking = true;

            auto seq = Sequence::create
            (
                Spawn::create(ScaleTo::create(0.1f, m_baseScale * 3.0f), TintTo::create(0.1f, Color3B::RED), nullptr),
                CallFunc::create
                ([this]()
                    {
                        //动画结束后调用死亡逻辑
                        this->m_currentHp = 0; // 确保血量为0
                        this->onDeath(); // 调用死亡方法，会触发BattleManager::removeTroop()
                    }
                ),
                nullptr
            );
            this->runAction(seq);
            return;
        }
        // --- 普通攻击 (弓箭手 & 野蛮人 & 巨人) ---
        else
        {
            Vec2 targetPos = m_target->getPosition();

            // 造成伤害
            m_target->takeDamage(m_damage);

            // 弓箭手特效
            if (m_type == TroopType::ARCHER)
            {
                auto arrow = Sprite::create();
                arrow->setTextureRect(Rect(0, 0, 10, 2));
                arrow->setColor(Color3B::YELLOW);
                arrow->setPosition(this->getPosition());
                this->getParent()->addChild(arrow);

                float distance = this->getPosition().distance(targetPos);
                float duration = distance / 400.0f;
                arrow->runAction(Sequence::create(MoveTo::create(duration, targetPos), RemoveSelf::create(), nullptr));
            }
            // 近战攻击动画
            else
            {
                m_isAttacking = true;
                const int WALK_ACTION_TAG = 0x1001;
                this->stopActionByTag(WALK_ACTION_TAG);

                std::string attackPlist = m_attackPlist;
                if (!attackPlist.empty()) SpriteFrameCache::getInstance()->addSpriteFramesWithFile(attackPlist);

                Vector<SpriteFrame*> attackFrames;
                if (!attackPlist.empty()) 
                {
                    ValueMap avm = FileUtils::getInstance()->getValueMapFromFile(attackPlist);
                    if (avm.find("frames") != avm.end())
                    {
                        auto framesMap = avm["frames"].asValueMap();
                        std::vector<std::string> keys;
                        keys.reserve(framesMap.size());
                        for (const auto& kv : framesMap) keys.push_back(kv.first);
                        std::sort(keys.begin(), keys.end());
                        for (const auto& name : keys) 
                        {
                            auto f = SpriteFrameCache::getInstance()->getSpriteFrameByName(name);
                            if (f) attackFrames.pushBack(f);
                        }
                    }
                }

                if (!attackFrames.empty()) {
                    auto attackAnim = Animation::createWithSpriteFrames(attackFrames, 0.08f);
                    auto attackAnimate = Animate::create(attackAnim);
                    auto restore = CallFunc::create([this, WALK_ACTION_TAG]() {
                        auto walkAnim = AnimationCache::getInstance()->getAnimation("troop_walk_anim");
                        if (walkAnim) {
                            auto repeat = RepeatForever::create(Animate::create(walkAnim));
                            repeat->setTag(WALK_ACTION_TAG);
                            this->runAction(repeat);
                        }
                        this->setScale(m_baseScale);
                        m_isAttacking = false;
                        });

                    this->runAction(Sequence::create(attackAnimate, restore, nullptr));
                }
                else {
                    auto up = ScaleTo::create(0.1f, m_baseScale * 1.2f);
                    auto down = ScaleTo::create(0.1f, m_baseScale);
                    auto seq = Sequence::create(up, down, CallFunc::create([this, WALK_ACTION_TAG]() {
                        auto walkAnim = AnimationCache::getInstance()->getAnimation("troop_walk_anim");
                        if (walkAnim) {
                            auto repeat = RepeatForever::create(Animate::create(walkAnim));
                            repeat->setTag(WALK_ACTION_TAG);
                            this->runAction(repeat);
                        }
                        m_isAttacking = false;
                        }), nullptr);
                    this->runAction(seq);
                }
            }
        }
    }
}

void Troop::playWalkAnimation()
{
    Vector<SpriteFrame*> frames;
    if (!m_walkPlist.empty()) {
        SpriteFrameCache::getInstance()->addSpriteFramesWithFile(m_walkPlist);
    }
    for (int i = 0; i < 8; ++i) {
        std::string frameName = StringUtils::format("dragon_%d.png", i);
        auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);
        if (frame) frames.pushBack(frame);
    }
    if (!frames.empty()) {
        auto animation = Animation::createWithSpriteFrames(frames, 0.12f);
        auto animate = Animate::create(animation);
        const int WALK_ACTION_TAG = 0x1001;
        this->stopActionByTag(WALK_ACTION_TAG);
        auto action = RepeatForever::create(animate);
        action->setTag(WALK_ACTION_TAG);
        this->runAction(action);
    }
}

void Troop::playAttackAnimationOnce()
{
    Vector<SpriteFrame*> frames;
    if (!m_attackPlist.empty()) {
        SpriteFrameCache::getInstance()->addSpriteFramesWithFile(m_attackPlist);
    }
    for (int i = 0; i < 8; ++i) {
        std::string frameName = StringUtils::format("dragon_attack_%d.png", i);
        auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);
        if (!frame) {
            frameName = StringUtils::format("dragon_%d.png", i);
            frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);
        }
        if (frame) frames.pushBack(frame);
    }
    if (!frames.empty()) {
        auto animation = Animation::createWithSpriteFrames(frames, 0.10f);
        auto animate = Animate::create(animation);
        const int ATTACK_ACTION_TAG = 0x1002;
        this->stopActionByTag(ATTACK_ACTION_TAG);
        auto seq = Sequence::create(animate, CallFunc::create([this]() {
            this->playWalkAnimation();
        }), nullptr);
        seq->setTag(ATTACK_ACTION_TAG);
        this->runAction(seq);
    }
}