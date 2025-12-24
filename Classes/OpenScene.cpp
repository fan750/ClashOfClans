//OpenScene.cpp
#include "OpenScene.h"
#include "SimpleAudioEngine.h"
#include"MainModeScene.h"
#include "ui/CocosGUI.h"

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
        float scale = std::max(scaleX, scaleY);
        sprite->setScale(scale);
        sprite->setPosition(visibleSize / 2);
        layer->addChild(sprite);
    }
    auto beginBtn = ui::Button::create("enter.png");

    beginBtn->setPosition(Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.35f));
    beginBtn->setScale(0.1f);

    beginBtn->setPressedActionEnabled(true);
    // 你还可以调整缩放幅度 (默认是缩小一点点)
    // beginBtn->setZoomScale(0.1f); 

    // 绑定点击事件
    // 注意：addClickEventListener 对应的是点击释放后的逻辑，正好配合我们的切换场景
    beginBtn->addClickEventListener(CC_CALLBACK_1(Load::onAssaultMenuCallback, this));

    // ui::Button 不需要 Menu 容器，直接加到 layer 上
    layer->addChild(beginBtn);

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
    // A. 获取点击的按钮，并将其禁用或隐藏，防止玩家重复点击
    auto item = dynamic_cast<MenuItem*>(pSender);
    if (item)
    {
        // 方法1：直接变灰不可点
        item->setEnabled(false);
        // 方法2：如果你想让按钮直接消失，用 item->setVisible(false);
    }

    // B. 获取屏幕尺寸
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // C. 创建“正在加载”的提示（这里用文字演示，也可以换成转圈圈的图片）
    auto loadingLabel = Label::createWithSystemFont("Loading...", "Arial", 48);
    loadingLabel->setPosition(visibleSize / 2); // 放在屏幕中间
    loadingLabel->setColor(Color3B::WHITE);     // 设置颜色
    this->addChild(loadingLabel, 100);          // 层级设高一点，确保在最上层

    // D. (可选) 给加载文字加个闪烁动画，看起来更生动
    auto fadeAction = Sequence::create(FadeOut::create(0.5f), FadeIn::create(0.5f), nullptr);
    loadingLabel->runAction(RepeatForever::create(fadeAction));

    // E. 【核心】开启一个定时器，延迟 2.0秒 后执行 enterMainMode 函数
    // 这里的 2.0f 是延迟时间，你可以根据需要修改
    this->scheduleOnce(CC_SCHEDULE_SELECTOR(Load::enterMainMode), 2.0f);
}

// 2. 【新增】实现真正的场景切换逻辑
void Load::enterMainMode(float dt)
{
    // 这里才是真正切换场景的地方
    Director::getInstance()->replaceScene(MainMode::createScene());
}