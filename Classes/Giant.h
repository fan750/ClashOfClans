#ifndef __GIANT_H__
#define __GIANT_H__

#include "Troop.h"

class Giant : public Troop
{
public:
    Giant();                                                             // 构造函数
    ~Giant() {}                                                          // 析构函数

<<<<<<< HEAD
    void initProperties() override;                                      // 初始化兵种属性
    void initAnimations() override;                                      // 初始化动画资源
    std::string Giant::getTroopName() const override { return "Giant"; } // 获取兵种名称
=======
    void initProperties() override;
    void initAnimations() override;
    std::string getTroopName() const override { return "Giant"; }
>>>>>>> b6d862d1c7cedb35d16bec246b9cb65549d270c7
};

#endif // __GIANT_H__