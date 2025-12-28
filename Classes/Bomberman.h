#ifndef __BOMBERMAN_H__
#define __BOMBERMAN_H__

#include "Troop.h"

class Bomberman : public Troop
{
public:
    Bomberman();                                                                 // 构造函数
    ~Bomberman() {}                                                              // 析构函数

    void initProperties() override;                                              // 初始化兵种属性
    void initAnimations() override;                                              // 初始化动画资源
    std::string Bomberman::getTroopName() const override { return "Bomberman"; } // 获取兵种名称

    void acquireTarget() override;                                               // 重写索敌逻辑（优先攻击城墙）
    void performAttackBehavior() override;                                       // 重写攻击行为（自爆）
};

#endif // __BOMBERMAN_H__