#ifndef __GAME_UI_H__
#define __GAME_UI_H__

#include "cocos2d.h"

class GameUI : public cocos2d::Layer {
public:
    virtual bool init() override;
    CREATE_FUNC(GameUI);

    // 刷新显示的文字
    void updateLabels();

private:
    cocos2d::Label* m_goldLabel;
    cocos2d::Label* m_elixirLabel;
};

#endif // __GAME_UI_H__