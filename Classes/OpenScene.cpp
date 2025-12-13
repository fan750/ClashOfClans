//OpenScene.cpp
#include "OpenScene.h"
#include "SimpleAudioEngine.h"
#include"MainModeScene.h"

USING_NS_CC;

Scene* Load::createScene()
{
    return Load::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in MainModeScene.cpp\n");
}

// on "init" you need to initialize your instance
bool Load::init()
{
    //////////////////////////////
    // 1. super init first
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto layer = Layer::create();
    auto sprite = Sprite::create("load.jpg");
    if (sprite == nullptr)
    {
        problemLoading("load.jpg");
    }
    else
    {
        // position the sprite on the center of the screen
        Size texSize = sprite->getContentSize();
        float scaleX = visibleSize.width / texSize.width;
        float scaleY = visibleSize.height / texSize.height;
        float scale = std::max(scaleX, scaleY); // ??¡À???¡¤????¨º??????
        sprite->setScale(scale);
        sprite->setPosition(visibleSize / 2);
        layer->addChild(sprite);
    }
    auto beginitem = MenuItemImage::create("play.png", "play.png", CC_CALLBACK_1(Load::onAssaultMenuCallback, this));
    beginitem->setPosition(Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.2f));
    beginitem->setScale(1.0f);
    auto menu = Menu::create(beginitem, NULL);
    menu->setPosition(Vec2::ZERO);
    layer->addChild(menu);
    this->addChild(layer);
    return true;
}


void Load::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);



}

void Load::onAssaultMenuCallback(Ref* pSender)
{
    Director::getInstance()->replaceScene(MainMode::createScene());
}