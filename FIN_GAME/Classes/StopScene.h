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

	//Ĳ�I
	cocos2d::EventListenerTouchOneByOne *_listener1;
	bool onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //Ĳ�I�}�l�ƥ�
	void onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //Ĳ�I���ʨƥ�
	void onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //Ĳ�I�����ƥ� 

	CREATE_FUNC(StopScene);
};
#endif // __STOPScene_SCENE_H__
