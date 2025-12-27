//UpgradeTroopUI.h
#ifndef _UPGRADE_UI_
#define _UPGRADE_UI_
#include"cocos2d.h"
#include "ui/CocosGUI.h"
#include"Troop.h"
#include <map>
#include <string>

struct UpgradeTroops {
	std::string name;
	int upgrade_cost;
	TroopType type;
	std::string iconpath;
};
class UpgradeUi : public cocos2d::Layer {
public:
	virtual bool init() override;
	CREATE_FUNC(UpgradeUi);
	void hide();
	void createUpgradeButton(const UpgradeTroops& troops, cocos2d::Vec2 pos, cocos2d::Node* parentnode);
	void onTroopUpgraded();
	void show(); // 显示升级兵种UI
private:
	void refreshUpgradeCosts();
	int calculateUpgradeCost(TroopType type) const;
	cocos2d::ui::Layout* m_mainPanel;
	std::map<TroopType, cocos2d::Label*> m_priceLabels;
	// 保存等级标签的 map
	std::map<TroopType, cocos2d::Label*> m_levelLabels;
};
#endif
