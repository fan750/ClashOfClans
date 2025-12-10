//MenuBuilder.cpp
#include "MenuBuilder.h"
#include "cocos2d.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

Menu* MenuBuilder::createImageMenu(const std::string& normal,
    const std::string& selected,
    const std::function<void(Ref*)>& callback,
    const Vec2& position,
    float scale) {
    auto item = MenuItemImage::create(normal, selected, [callback](Ref* ref) {
        if (callback) callback(ref);
        });
    item->setPosition(position);
    item->setScale(scale);
    auto menu = Menu::create(item, NULL);
    menu->setPosition(Vec2::ZERO);
    return menu;
}

Menu* MenuBuilder::createToggleMenu(const std::string& normal,
    const std::string& selected,
    const std::function<void(Ref*)>& callback,
    const Vec2& position,
    float scale) {
    auto spriteNormal = Sprite::create(normal);
    auto spriteSelected = Sprite::create(selected);
    auto normalItem = MenuItemSprite::create(spriteNormal, spriteNormal);
    auto selectedItem = MenuItemSprite::create(spriteSelected, spriteSelected);

    auto toggle = MenuItemToggle::createWithCallback([callback](Ref* ref) {
        if (callback) callback(ref);
        }, normalItem, selectedItem, NULL);
    toggle->setPosition(position);
    toggle->setScale(scale);

    auto menu = Menu::create(toggle, NULL);
    menu->setPosition(Vec2::ZERO);
    return menu;
}
