#ifndef __STARTSCENE_H__
#define __STARTSCENE_H__

#include "cocos2d.h"
#include "Common/CButton.h"

class StartScene : public cocos2d::Layer
{
private:
	CButton * OnePage;
public:

	~StartScene();
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	void doStep(float dt);

	cocos2d::EventListenerTouchOneByOne *_listener1;
	bool onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //Ĳ�I�}�l�ƥ�
	void onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //Ĳ�I���ʨƥ�
	void onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //Ĳ�I�����ƥ� 

																	   // implement the "static create()" method manually
	CREATE_FUNC(StartScene);
};

#endif // __STARTScene_SCENE_H__