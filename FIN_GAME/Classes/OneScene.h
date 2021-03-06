#ifndef __ONESCENE_H__
#define __ONESCENE_H__

#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "Common/CButton.h"
#include "Common/GLES-Render.h"

struct AirDraw
{
	cocos2d::Point pos;
	struct AirDraw * NextAir;
};
struct AirDiet {
	cocos2d::Sprite * _sprite;
	struct AirDiet * _NexttargetSprite;
};
class CContactListener : public b2ContactListener
{
public:
	struct AirDiet * _HeadtargetSprite;
	struct AirDiet * _NewtargetSprite;
	cocos2d::Sprite * _Playersprite;
	CContactListener();
	bool win = false;
	//碰撞開始
	virtual void BeginContact(b2Contact* contact);
	//碰撞結束
	virtual void EndContact(b2Contact* contact);
	void setCollisionTarget(cocos2d::Sprite &targetSprite);
	void setCollisionTargetPlayer(cocos2d::Sprite &targetSprite);
};

class OneScene : public cocos2d::Layer
{
private:
	CButton * SkipBtn;
	CButton * StopBtn;
	CButton * AirBtn;
	CButton * MagnetBtn;
	b2World* _b2World;
	b2Body *rectBody;
	cocos2d::Size visibleSize;
	cocos2d::Vec2 origin;
	cocos2d::Node * OneBackground;
	cocos2d::Point PntLoc;
	cocos2d::Sprite * PlayerSprite;
	cocos2d::Sprite * ghostSprite = nullptr;
	int score = 0;
	float _fGameTime = 0;
	bool _bAirOpen = false;
	bool _bPlayerGo = false;
	bool _bStart = false;

	CContactListener _contactListener;
	struct AirDraw * HeadAir;
	struct AirDraw * NewAir;

	GLESDebugDraw* _DebugDraw;
	virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags);
public:

	~OneScene();
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene(const int score);

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	void doStep(float dt);

	void nextScene();
	void stopScene();
	void readSceneFile();
	void setupWinSensor();
	void CreateAir();
	void CreatePlayer();
	void CreateGhost(cocos2d::Point loc);
	void ghostFinished();

	cocos2d::EventListenerTouchOneByOne *_listener1;
	bool onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰開始事件
	void onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰移動事件
	void onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰結束事件 

																	   // implement the "static create()" method manually
	CREATE_FUNC(OneScene);

};

#endif // __OneScene_SCENE_H__
