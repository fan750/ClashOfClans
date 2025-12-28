#ifndef MENUBUILDER_H
#define MENUBUILDER_H

#include "cocos2d.h"
#include <functional>
#include <string>

class MenuBuilder
{
public:
    static cocos2d::Menu* createImageMenu(const std::string& normal,
        const std::string& selected,
        const std::function<void(cocos2d::Ref*)>& callback,
        const cocos2d::Vec2& position,
        float scale = 1.0f); // 创建单个图片按钮并包装成 Menu（返回指向 Menu 的指针，调用方负责 addChild）

    static cocos2d::Menu* createToggleMenu(const std::string& normal,
        const std::string& selected,
        const std::function<void(cocos2d::Ref*)>& callback,
        const cocos2d::Vec2& position,
        float scale = 1.0f); // 创建切换按钮（两个状态）并包装成 Menu
};

#endif // MENUBUILDER_H