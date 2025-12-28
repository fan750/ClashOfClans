#ifndef __ARCHER_H__
#define __ARCHER_H__

#include "Troop.h"

class Archer : public Troop
{
public:
    Archer();                                                                     // 构造函数
    ~Archer() {}                                                                  // 析构函数

    void initProperties() override;                                               // 初始化兵种属性
    void initAnimations() override;                                               // 初始化动画资源
    void performAttackBehavior() override;                                        // 执行远程攻击行为
    std::string Archer::getTroopName() const override { return "Archer"; }        // 获取兵种名称
};

#endif // __ARCHER_H__