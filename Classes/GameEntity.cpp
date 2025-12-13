//GameEntity.cpp
#include "GameEntity.h"
USING_NS_CC;


GameEntity::GameEntity()
    : m_maxHp(100)
    , m_currentHp(100)
    , m_camp(CampType::NEUTRAL)
    , m_hpBarDraw(nullptr)
    , m_hpBarWidth(80.0f)
    , m_hpBarHeight(40.0f) // Initialize height
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
    if (m_hpBarDraw) return;
    m_hpBarDraw = DrawNode::create();
    // 初始添加到父节点留到子类或场景中，当 entity 被添加到父节点时，会有父节点
    this->addChild(m_hpBarDraw, 100);
}

void GameEntity::updateHpBar()
{
    if (!m_hpBarDraw) return;

    m_hpBarDraw->clear();

    // 计算在实体顶部的本地偏移
    Size contentSize = this->getContentSize();
    float topOffset = contentSize.height * this->getScaleY() / 2.0f + 10.0f;

    // 将 DrawNode 保持为实体的子节点，这样会随着实体移动而移动，确保位置在正上方
    m_hpBarDraw->setPosition(Vec2(0, topOffset));

    // Use member height if provided, otherwise fallback
    float barHeight = (m_hpBarHeight > 0.0f) ? m_hpBarHeight : 6.0f;

    Vec2 bgStart = Vec2(-m_hpBarWidth / 2.0f, 0);
    Vec2 bgEnd = Vec2(m_hpBarWidth / 2.0f, barHeight);
    m_hpBarDraw->drawSolidRect(bgStart, bgEnd, Color4F(0.2f, 0.2f, 0.2f, 1.0f));

    float hpPercent = 1.0f;
    if (m_maxHp > 0) hpPercent = static_cast<float>(m_currentHp) / static_cast<float>(m_maxHp);
    hpPercent = std::max(0.0f, std::min(1.0f, hpPercent));

    float fgWidth = m_hpBarWidth * hpPercent;
    Vec2 fgEnd = Vec2(-m_hpBarWidth / 2.0f + fgWidth, barHeight);
    m_hpBarDraw->drawSolidRect(Vec2(-m_hpBarWidth / 2.0f, 0), fgEnd, Color4F(0.0f, 1.0f, 0.0f, 1.0f));
}

void GameEntity::removeHpBar()
{
    if (m_hpBarDraw)
    {
        m_hpBarDraw->removeFromParent();
        m_hpBarDraw = nullptr;
    }
}