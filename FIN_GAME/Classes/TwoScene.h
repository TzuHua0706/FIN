#ifndef __TWOSCENE_H__
#define __TWOSCENE_H__

#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "Common/CButton.h"
#include "Common/GLES-Render.h"

struct AirDraw_Two
{
	cocos2d::Point pos;
	struct AirDraw_Two * NextAir;
};
struct water_Two
{
	cocos2d::Sprite * _sprite;
	float _ftime = 0;
	struct water_Two * NextWater;
};
struct AirDiet_Two {
	cocos2d::Sprite * _sprite;
	struct AirDiet_Two * _NexttargetSprite;
};
class CContactListener_Two : public b2ContactListener
{
public:
	struct AirDiet_Two * _HeadtargetSprite = NULL;
	struct AirDiet_Two * _NewtargetSprite;
	cocos2d::Sprite * _Playersprite;
	bool win = false;

	CContactListener_Two();
	//碰撞開始
	virtual void BeginContact(b2Contact* contact);
	//碰撞結束
	virtual void EndContact(b2Contact* contact);
	void setCollisionTarget(cocos2d::Sprite &targetSprite);
	void setCollisionTargetPlayer(cocos2d::Sprite &targetSprite);
};
class TwoScene : public cocos2d::Layer
{
private:
	CButton * SkipBtn;
	CButton * StopBtn;
	CButton * AirBtn;
	CButton * MagnetBtn;
	b2World* _b2World;
	b2Body *playerBody;
	b2Body *MagnetBody;
	b2Body *NewMagnetBody;
	cocos2d::Size visibleSize;
	cocos2d::Vec2 origin;
	cocos2d::Node * TwoBackground;
	cocos2d::Point PntLoc;
	cocos2d::Sprite * PlayerSprite;
	cocos2d::Sprite * NewMagnetSprite;
	cocos2d::Sprite * ghostSprite = nullptr;
	int score = 0;
	float _fGameTime = 0;
	float _fWaterTime = 0;
	bool _bAirOpen = false;
	bool _bMagnetOpen = false;
	bool _bPlayerGo = false;

	CContactListener_Two _contactListener;
	struct AirDraw_Two * HeadAir;
	struct AirDraw_Two * NewAir;
	struct water_Two * HeadWater;
	struct water_Two * NewWater;

	GLESDebugDraw* _DebugDraw;
	virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags);
public:

	~TwoScene();
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene(const int score);

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	void doStep(float dt);

	void nextScene();
	void stopScene();
	void readSceneFile();
	void setupRopeJoint();
	void setupPulleyJoint();
	void setupWinSensor();
	void CreateAir();
	void CreateWater();
	void CreateMagnet(cocos2d::Point loc); //磁鐵
	void CreatePlayer();
	void CreateGhost(cocos2d::Point loc);
	void ghostFinished();

	cocos2d::EventListenerTouchOneByOne *_listener1;
	bool onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰開始事件
	void onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰移動事件
	void onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰結束事件 

																	   // implement the "static create()" method manually
	CREATE_FUNC(TwoScene);

};

#endif // __TwoScene_SCENE_H__
