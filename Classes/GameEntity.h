//GameEntity.h
#ifndef __GAME_ENTITY_H__
#define __GAME_ENTITY_H__
#include "cocos2d.h"

// 游戏实例阵营类型枚举
enum class CampType
{
    PLAYER,
    ENEMY,
    NEUTRAL
};

class GameEntity : public cocos2d::Sprite
{
protected:
    int m_maxHp;        // 最大血量
    int m_currentHp;    // 当前血量
    CampType m_camp;    // 阵营
    void initHpBar();   // 血条初始化
    void updateHpBar(); // 更新血条
    void removeHpBar(); // 移除血条

    cocos2d::DrawNode* m_hpBarDraw;
    float m_hpBarWidth;
    float m_hpBarHeight; // 添加高度成员，Building.cpp 中使用
    // 血条垂直偏移量
    float m_hpBarOffsetY;
    float m_hpBarOffsetX;
public:
    GameEntity();
    virtual ~GameEntity();
    virtual bool init() override;                    // 初始化
    void setProperties(int maxHp, CampType camp);    // 设置基础属性
    virtual void takeDamage(int damage);             // 受伤
    virtual void onDeath();                          // 死亡
    virtual void updateLogic(float dt);              // 更新逻辑
    virtual void update(float dt) override;          // 更新

    bool isDead() const { return m_currentHp <= 0; } // 判断是否死亡
    CampType getCamp() const { return m_camp; }      // 获取实例类型
    int getCurrentHp() const { return m_currentHp; } // 获取当前血量
    int getMaxHp() const { return m_maxHp; }         // 获取最大血量
    // 设置偏移量的函数供子类调用
    void setHpBarOffsetY(float y) { m_hpBarOffsetY = y; }
    void setHpBarOffsetX(float x) { m_hpBarOffsetX = x; }

    // 新增:
    cocos2d::Sprite* m_hpBgSprite;           // 血条背景图
    cocos2d::ProgressTimer* m_hpBarTimer;    // 血条进度条

    CREATE_FUNC(GameEntity);

};

#endif // __GAME_ENTITY_H__