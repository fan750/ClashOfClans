#ifndef __BARBARIAN_H__
#define __BARBARIAN_H__

#include "Troop.h"

class Barbarian : public Troop
{
public:
    Barbarian();                                                      // 构造函数
    ~Barbarian() {}                                                   // 析构函数

    void initProperties() override;                                   // 初始化兵种属性
    void initAnimations() override;                                   // 初始化动画资源
    std::string getTroopName() const override { return "Barbarian"; } // 获取兵种名称

    // 使用基类的默认索敌和攻击行为
};

#endif // __BARBARIAN_H__