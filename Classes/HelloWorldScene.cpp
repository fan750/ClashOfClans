#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "GameEntity.h" // 根据你的实际路径，可能是 "GameEntity.h"
#include "Building.h"
#include "Troop.h"
#include "GameManager.h"
#include "GameUI.h"
#include "BattleScene.h"
#include "ui/CocosGUI.h"
USING_NS_CC;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 1. 初始化账户：给玩家 1000 块启动资金
    GameManager::getInstance()->initAccount(1000, 1000);

    // 2. 创建并添加 UI 层
    auto uiLayer = GameUI::create();
    // ZOrder 设为 100，保证它永远盖在兵种和建筑上面
    this->addChild(uiLayer, 100);

    auto townHall = Building::create(BuildingType::TOWN_HALL);
    townHall->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    this->addChild(townHall);

    // 2. 创建一个金矿 (放在左边)
    auto mine = Building::create(BuildingType::GOLD_MINE);
    mine->setPosition(Vec2(visibleSize.width / 2 - 100, visibleSize.height / 2));
    this->addChild(mine);

    // 3. 创建一个加农炮 (放在右边)
    auto cannon = Building::create(BuildingType::CANNON);
    cannon->setPosition(Vec2(visibleSize.width / 2 + 100, visibleSize.height / 2));
    this->addChild(cannon);

    auto wall = Building::create(BuildingType::WALL);
    wall->setPosition(Vec2(visibleSize.width / 2 , visibleSize.height / 2+50));
    this->addChild(wall);

    
    // 4. 测试升级
    townHall->upgrade();
    
    auto attackBtn = ui::Button::create("CloseNormal.png"); // 暂时用关闭按钮的图代替
    attackBtn->setTitleText("ATTACK!");
    attackBtn->setTitleFontSize(30);
    attackBtn->setPosition(Vec2(visibleSize.width - 100, 100)); // 右下角

    attackBtn->addClickEventListener([=](Ref* sender) {
        // 点击后，切换到战斗场景
        // 我们稍后创建 BattleScene
        auto scene = BattleScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
        });

    this->addChild(attackBtn);
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);


}
