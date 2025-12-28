#include "OpenScene.h"
#include "SimpleAudioEngine.h"
#include"MainModeScene.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

// 创建场景
Scene* Load::createScene()
{
    return Load::create();
}

// 静态函数：资源加载错误提示
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in MainModeScene.cpp\n");
}

// 初始化场景
bool Load::init()
{
    // 初始化父类
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
        // 设置背景图片：居中并缩放以铺满屏幕
        Size texSize = sprite->getContentSize();
        float scaleX = visibleSize.width / texSize.width;
        float scaleY = visibleSize.height / texSize.height;
        float scale = std::max(scaleX, scaleY);
        sprite->setScale(scale);
        sprite->setPosition(visibleSize / 2);
        layer->addChild(sprite);
    }

    // 创建并配置开始按钮
    auto beginBtn = ui::Button::create("enter.png");

    beginBtn->setPosition(Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.35f));
    beginBtn->setScale(0.1f);

    beginBtn->setPressedActionEnabled(true);

    // 绑定点击事件
    beginBtn->addClickEventListener(CC_CALLBACK_1(Load::onAssaultMenuCallback, this));

    // 按钮直接加到 layer 上
    layer->addChild(beginBtn);

    this->addChild(layer);
    return true;
}

// 关闭场景（退出程序）
void Load::menuCloseCallback(Ref* pSender)
{
    // 退出游戏
    Director::getInstance()->end();
}

// 开始按钮回调：处理加载并切换场景
void Load::onAssaultMenuCallback(Ref* pSender)
{
    // 获取点击的按钮并将其禁用，防止玩家重复点击
    auto item = dynamic_cast<MenuItem*>(pSender);
    if (item)
    {
        // 变灰不可点
        item->setEnabled(false);
    }

    // 获取屏幕尺寸
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建“正在加载”的提示文字
    auto loadingLabel = Label::createWithSystemFont("Loading...", "Arial", 48);
    loadingLabel->setPosition(visibleSize / 2); // 放在屏幕中间
    loadingLabel->setColor(Color3B::WHITE);     // 设置颜色
    this->addChild(loadingLabel, 100);          // 层级设高一点，确保在最上层

    // 给加载文字加个闪烁动画
    auto fadeAction = Sequence::create(FadeOut::create(0.5f), FadeIn::create(0.5f), nullptr);
    loadingLabel->runAction(RepeatForever::create(fadeAction));

    // 开启定时器，延迟 2.0 秒后执行 enterMainMode 函数
    this->scheduleOnce(CC_SCHEDULE_SELECTOR(Load::enterMainMode), 2.0f);
}

// 定时器回调：执行场景切换
void Load::enterMainMode(float dt)
{
    // 切换到主场景
    Director::getInstance()->replaceScene(MainMode::createScene());
}