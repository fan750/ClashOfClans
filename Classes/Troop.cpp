#include "Troop.h"
#include "BattleManager.h" // 确保引用了 BattleManager
#include <fstream> // 添加文件流支持
#include <algorithm>

USING_NS_CC;

Troop::Troop()
    : m_type(TroopType::BARBARIAN)
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

Troop* Troop::create(TroopType type) {
    Troop* pRet = new(std::nothrow) Troop();
    if (pRet) {
        pRet->m_type = type;
        if (pRet->init()) {
            pRet->autorelease();
            return pRet;
        }
    }
    delete pRet;
    return nullptr;
}

static int extractFirstNumber(const std::string& s) {
    int n = 0;
    bool found = false;
    for (char c : s) {
        if (c >= '0' && c <= '9') {
            found = true;
            n = n * 10 + (c - '0');
        } else if (found) {
            break;
        }
    }
    return n;
}

bool Troop::init() {
    if (!GameEntity::init()) return false;

    initTroopProperties();

    // decide plist names per troop type
    std::string walkPlist;
    std::string attackPlist;
    switch (m_type) {
    case TroopType::BARBARIAN:
        walkPlist = "hero1-walk.plist";
        attackPlist = "hero1-attack.plist";
        break;
    case TroopType::ARCHER:
        walkPlist = "hero2-walk.plist";
        attackPlist = "hero2-attack.plist";
        break;
    case TroopType::GIANT:
        walkPlist = "hero3-walk.plist";
        attackPlist = "hero3-attack.plist";
        break;
    case TroopType::BOMBERMAN:
        walkPlist = "hero4-walk.plist";
        attackPlist = ""; // no attack
        break;
    default:
        walkPlist = "hero1-walk.plist";
        attackPlist = "hero1-attack.plist";
        break;
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
        std::sort(keyed.begin(), keyed.end(), [](const std::pair<int,std::string>& a, const std::pair<int,std::string>& b){
            if (a.first != b.first) return a.first < b.first;
            return a.second < b.second;
        });
        for (const auto& p : keyed) {
            auto f = SpriteFrameCache::getInstance()->getSpriteFrameByName(p.second);
            if (f) {
                frames.pushBack(f);
                CCLOG("Troop::init - walk frame added: %s", p.second.c_str());
            } else {
                CCLOG("Troop::init - walk frame not in cache: %s", p.second.c_str());
            }
        }
    } else {
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
        m_baseScale = this->getScale();
        CCLOG("Troop::init - applied scale factor %f, baseScale=%f", scaleFactor, m_baseScale);

        const int WALK_ACTION_TAG = 0x1001;
        auto walkAction = RepeatForever::create(animate);
        walkAction->setTag(WALK_ACTION_TAG);
        this->runAction(walkAction);
    } else {
        CCLOG("Troop::init - no walk frames, running fallback action");
        auto seq = Sequence::create(ScaleTo::create(0.4f, 1.2f), ScaleTo::create(0.4f, 1.0f), nullptr);
        this->runAction(RepeatForever::create(seq));
    }

    BattleManager::getInstance()->addTroop(this);
    this->scheduleUpdate();
    return true;
}

void Troop::onDeath() {
    BattleManager::getInstance()->removeTroop(this);
    GameEntity::onDeath();
}

void Troop::initTroopProperties() {
    // 默认设置
    this->setTextureRect(Rect(0, 0, 20, 20));

    if (m_type == TroopType::BARBARIAN) {
        // keep original sprite color
        setProperties(100, CampType::ENEMY);
        m_moveSpeed = 100.0f;
        m_attackRange = 40.0f;
        m_damage = 50;
        m_attackInterval = 1.0f;
    }
    else if (m_type == TroopType::ARCHER) {
        // keep original sprite color
        this->setTextureRect(Rect(0, 0, 15, 20));
        setProperties(60, CampType::ENEMY);
        m_moveSpeed = 120.0f;
        m_attackRange = 150.0f;
        m_damage = 25;
        m_attackInterval = 0.8f;
    }
    else if (m_type == TroopType::GIANT) {
        // keep original sprite color
        this->setTextureRect(Rect(0, 0, 40, 40));
        setProperties(200, CampType::ENEMY);
        m_moveSpeed = 80.0f;
        m_attackRange = 40.0f;
        m_damage = 40;
        m_attackInterval = 1.5f;
    }
    else if (m_type == TroopType::BOMBERMAN) {
        // keep original sprite color
        this->setTextureRect(Rect(0, 0, 15, 15));
        setProperties(40, CampType::ENEMY);
        m_moveSpeed = 150.0f;
        m_attackRange = 20.0f;
        m_damage = 500;
        m_attackInterval = 0.1f;
    }
}

void Troop::setTarget(Building* target) {
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

void Troop::attackTarget(float dt) {
    m_attackTimer += dt;

    if (m_attackTimer >= m_attackInterval) {
        m_attackTimer = 0;

        if (m_isAttacking) return; // already attacking

        if (m_type == TroopType::BOMBERMAN) {
            CCLOG("Bomberman Exploded!");
            Vec2 explosionCenter = this->getPosition();
            BattleManager::getInstance()->dealAreaDamage(explosionCenter, 100.0f, m_damage);

            auto seq = Sequence::create(
                Spawn::create(ScaleTo::create(0.1f, m_baseScale * 3.0f), TintTo::create(0.1f, Color3B::RED), nullptr),
                RemoveSelf::create(),
                nullptr
            );
            this->runAction(seq);
            return;
        }

        // 普通攻击
        Vec2 targetPos = m_target->getPosition(); // 1. 先存位置
        m_target->takeDamage(m_damage);           // 2. 再伤害

        // 动画
        if (m_type == TroopType::ARCHER) {
            auto arrow = Sprite::create();
            arrow->setTextureRect(Rect(0, 0, 10, 2));
            arrow->setColor(Color3B::YELLOW);
            arrow->setPosition(this->getPosition());
            this->getParent()->addChild(arrow);

            float distance = this->getPosition().distance(targetPos);
            float duration = distance / 400.0f;
            arrow->runAction(Sequence::create(MoveTo::create(duration, targetPos), RemoveSelf::create(), nullptr));
        }
        else {
            // melee: play attack animation once
            m_isAttacking = true;
            const int WALK_ACTION_TAG = 0x1001;
            this->stopActionByTag(WALK_ACTION_TAG);

            // 根据 m_type 选择对应的 attack plist
            std::string attackPlist = m_attackPlist;
            if (!attackPlist.empty()) SpriteFrameCache::getInstance()->addSpriteFramesWithFile(attackPlist);

            // Build attack frames from plist 'frames' keys
            Vector<SpriteFrame*> attackFrames;
            if (!attackPlist.empty()) {
                ValueMap avm = FileUtils::getInstance()->getValueMapFromFile(attackPlist);
                if (avm.find("frames") != avm.end()) {
                    auto framesMap = avm["frames"].asValueMap();
                    std::vector<std::string> keys;
                    keys.reserve(framesMap.size());
                    for (const auto& kv : framesMap) keys.push_back(kv.first);
                    std::sort(keys.begin(), keys.end());
                    for (const auto& name : keys) {
                        auto f = SpriteFrameCache::getInstance()->getSpriteFrameByName(name);
                        if (f) attackFrames.pushBack(f);
                    }
                }
            }

            if (!attackFrames.empty()) {
                auto attackAnim = Animation::createWithSpriteFrames(attackFrames, 0.08f);
                auto attackAnimate = Animate::create(attackAnim);
                // After attack, restore walk animation and clear flag
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
            } else {
                // fallback: simple scale feedback, relative to base scale
                auto up = ScaleTo::create(0.1f, m_baseScale * 1.2f);
                auto down = ScaleTo::create(0.1f, m_baseScale);
                auto seq = Sequence::create(up, down, CallFunc::create([this, WALK_ACTION_TAG]() {
                    // restart walk
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