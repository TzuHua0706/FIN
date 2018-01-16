#ifndef __STOPSCENE_H__
#define __STOPSCENE_H__

#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "ui/UIWidget.h"
#include "Common/CButton.h"

class StopScene : public cocos2d::Layer
{
private:
	CButton * PlayBtn;
	CButton * ReplayBtn;
	CButton * HomeBtn;
	int score;
	int number;
public:
	~StopScene();
	static cocos2d::Scene* createScene(cocos2d::RenderTexture * sc, const int score, const int number);
	virtual bool init();

	//觸碰
	cocos2d::EventListenerTouchOneByOne *_listener1;
	bool onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰開始事件
	void onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰移動事件
	void onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰結束事件 

	CREATE_FUNC(StopScene);
};
#endif // __STOPScene_SCENE_H__
