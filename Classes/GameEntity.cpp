//GameEntity.cpp
#include "GameEntity.h"
USING_NS_CC;


GameEntity::GameEntity()
    : m_maxHp(100)
    , m_currentHp(100)
    , m_camp(CampType::NEUTRAL)
    ,m_hpBgSprite(nullptr)    
    , m_hpBarTimer(nullptr)
    , m_hpBarWidth(80.0f)
    , m_hpBarHeight(40.0f) // Initialize height
    , m_hpBarOffsetY(15.0f) // 默认给个 15 的基础高度
    ,m_hpBarOffsetX(15.0f)
{
}

GameEntity::~GameEntity()
{
    removeHpBar();
}

bool GameEntity::init()
{
    if (!Sprite::init()) { return false; } // 调用父类 Sprite 的 init
    initHpBar();                           // 初始化血条
    return true;                           // 初始化成功
}

void GameEntity::setProperties(int maxHp, CampType camp) // 设置血量、阵营
{
    this->m_maxHp = maxHp;
    this->m_currentHp = maxHp;
    this->m_camp = camp;
}

void GameEntity::takeDamage(int damage)
{
    if (isDead()) return;
    m_currentHp -= damage;
    CCLOG("Entity Took Damage. Remaining: %d", m_currentHp);

    if (m_currentHp <= 0)
    {
        m_currentHp = 0;
        onDeath();
    }
    else
    {
        // 更新血条显示
        updateHpBar();
    }
}

void GameEntity::onDeath()
{
    removeHpBar();
    this->removeFromParent();
}

void GameEntity::updateLogic(float dt)
{
    // 默认没有额外逻辑
}

void GameEntity::update(float dt)
{
    this->updateLogic(dt);
    // 每帧刷新血条位置（如果存在）
    updateHpBar();
}

// HP bar implementation
void GameEntity::initHpBar()
{
    if (m_hpBarTimer) return;

    // 1. 创建背景
    m_hpBgSprite = Sprite::create("hp_bg.png");
    if (m_hpBgSprite)
    {
        this->addChild(m_hpBgSprite, 100);

        // =========================================================
        // 【核心修改】根据 m_hpBarWidth 强行缩放图片
        // =========================================================
        Size imgSize = m_hpBgSprite->getContentSize();
        if (imgSize.width > 0 && imgSize.height > 0)
        {
            // 如果你在 Building/Troop 里设置了 m_hpBarWidth (比如 120)
            // 这里就会计算出缩放比例，把图片拉伸或缩小到 120
            float scaleX = m_hpBarWidth/100;
            float scaleY = m_hpBarHeight/100 ;
            
            m_hpBgSprite->setScale(scaleX, scaleY);
        }
    }

    // 2. 创建进度条 (完全一样的逻辑)
    auto fillSprite = Sprite::create("hp_fill.png");
    if (fillSprite)
    {
        m_hpBarTimer = ProgressTimer::create(fillSprite);
        m_hpBarTimer->setType(ProgressTimer::Type::BAR);
        m_hpBarTimer->setMidpoint(Vec2(0, 0.5f));
        m_hpBarTimer->setBarChangeRate(Vec2(1, 0));
        m_hpBarTimer->setPercentage(100.0f);
        
        // 【核心修改】进度条也要应用完全一样的缩放，否则会对不齐
        Size imgSize = fillSprite->getContentSize();
        if (imgSize.width > 0 && imgSize.height > 0)
        {
            float scaleX = m_hpBarWidth/100;
            float scaleY = m_hpBarHeight/100;
            m_hpBarTimer->setScale(scaleX, scaleY);
        }

        // 位置设置 (利用上一轮我们加的 Offset)
        // ... (updateHpBar 会负责设置位置，这里可以不设，或者设为0) ...
        
        this->addChild(m_hpBarTimer, 101);
    }
}
void GameEntity::updateHpBar()
{
    if (!m_hpBarTimer) return;

    // 计算基准位置 (物体顶部)
    Size contentSize = this->getContentSize();

    // 【核心修改】位置公式 = (高度的一半 * 缩放) + 手动偏移量
    float topPos = (contentSize.height * this->getScaleY()+500.0f) + m_hpBarOffsetY;

    // 更新位置 (背景和进度条都要设)
    if (m_hpBgSprite) m_hpBgSprite->setPosition(Vec2(400+m_hpBarOffsetX, topPos));
    m_hpBarTimer->setPosition(Vec2(400 + m_hpBarOffsetX, topPos));

    // 2. 计算并设置百分比
    float hpPercent = 100.0f;
    if (m_maxHp > 0)
    {
        hpPercent = (static_cast<float>(m_currentHp) / m_maxHp) * 100.0f;
    }
    // ProgressTimer 会自动裁剪图片
    m_hpBarTimer->setPercentage(hpPercent);
}

// 别忘了在 removeHpBar 里把它们都 remove 掉
void GameEntity::removeHpBar()
{
    if (m_hpBgSprite) {
        m_hpBgSprite->removeFromParent();
        m_hpBgSprite = nullptr;
    }
    if (m_hpBarTimer) {
        m_hpBarTimer->removeFromParent();
        m_hpBarTimer = nullptr;
    }
}