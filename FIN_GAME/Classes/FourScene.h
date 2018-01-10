#ifndef __FOURScene_H__
#define __FOURScene_H__

#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "Common/CButton.h"
#include "Common/GLES-Render.h"

class FourScene : public cocos2d::Layer
{
private:
	b2World* _b2World;
	b2Body *rectBody;
	cocos2d::Size visibleSize;
	cocos2d::Vec2 origin;
	cocos2d::Node * TwoBackground;
	cocos2d::Point PntLoc;
	cocos2d::Sprite * PlayerSprite;
	cocos2d::Sprite * AirSprite;
	cocos2d::Point BeginLoc;
	float _fGameTime = 0;
	bool _bAirOpen = false;
	bool _bPlayerGo = false;

	GLESDebugDraw* _DebugDraw;
	virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags);
public:

	~FourScene();
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	void doStep(float dt);

	void readSceneFile();
	void CreateAir(cocos2d::Point Bpos, cocos2d::Point Epos);

	cocos2d::EventListenerTouchOneByOne *_listener1;
	bool onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰開始事件
	void onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰移動事件
	void onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰結束事件 

																	   // implement the "static create()" method manually
	CREATE_FUNC(FourScene);

};

#endif // __FourScene_SCENE_H__