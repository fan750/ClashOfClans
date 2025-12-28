#include "MenuBuilder.h"
#include "cocos2d.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

// 创建单个图片按钮并包装成 Menu
Menu* MenuBuilder::createImageMenu(const std::string& normal,
    const std::string& selected,
    const std::function<void(Ref*)>& callback,
    const Vec2& position,
    float scale)
{
    // 创建菜单项并绑定回调
    auto item = MenuItemImage::create(normal, selected, [callback](Ref* ref)
        {
            if (callback)
            {
                callback(ref);
            }
        });
    // 设置位置与缩放
    item->setPosition(position);
    item->setScale(scale);
    // 创建菜单容器并归位
    auto menu = Menu::create(item, NULL);
    menu->setPosition(Vec2::ZERO);
    return menu;
}

// 创建切换按钮（两个状态）并包装成 Menu
Menu* MenuBuilder::createToggleMenu(const std::string& normal,
    const std::string& selected,
    const std::function<void(Ref*)>& callback,
    const Vec2& position,
    float scale)
{
    // 创建精灵资源
    auto spriteNormal = Sprite::create(normal);
    auto spriteSelected = Sprite::create(selected);
    // 创建两种状态的菜单项
    auto normalItem = MenuItemSprite::create(spriteNormal, spriteNormal);
    auto selectedItem = MenuItemSprite::create(spriteSelected, spriteSelected);

    // 创建切换菜单项并绑定回调
    auto toggle = MenuItemToggle::createWithCallback([callback](Ref* ref)
        {
            if (callback)
            {
                callback(ref);
            }
        }, normalItem, selectedItem, NULL);
    // 设置位置与缩放
    toggle->setPosition(position);
    toggle->setScale(scale);

    // 创建菜单容器并归位
    auto menu = Menu::create(toggle, NULL);
    menu->setPosition(Vec2::ZERO);
    return menu;
}