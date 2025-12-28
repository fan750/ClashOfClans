#ifndef __DRAGON_H__
#define __DRAGON_H__

#include "Troop.h"

class Dragon : public Troop
{
public:
    Dragon();                                                                // 构造函数
    ~Dragon() {}                                                             // 析构函数

    void initProperties() override;                                         // 初始化兵种属性
    void initAnimations() override;                                         // 初始化动画资源
    std::string Dragon::getTroopName() const override { return "Dragon"; }  // 获取兵种名称
};

#endif // __DRAGON_H__