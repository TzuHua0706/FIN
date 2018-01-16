#ifndef __FOURScene_H__
#define __FOURScene_H__

#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "Common/CButton.h"
#include "Common/GLES-Render.h"

struct AirDraw_Four
{
	cocos2d::Point pos;
	struct AirDraw_Four * NextAir;
};
struct water_Four
{
	cocos2d::Sprite * _sprite;
	float _ftime = 0;
	struct water_Four * NextWater;
};
struct AirDiet_Four {
	cocos2d::Sprite * _sprite;
	bool _bAir = false;
	struct AirDiet_Four * _NexttargetSprite;
};
class CContactListener_Four : public b2ContactListener
{
public:
	struct AirDiet_Four * _HeadtargetSprite = NULL;
	struct AirDiet_Four * _NewtargetSprite;
	cocos2d::Sprite * _Playersprite;
	bool win = false;

	CContactListener_Four();
	//碰撞開始
	virtual void BeginContact(b2Contact* contact);
	//碰撞結束
	virtual void EndContact(b2Contact* contact);
	void setCollisionTarget(cocos2d::Sprite &targetSprite);
	void setCollisionTargetPlayer(cocos2d::Sprite &targetSprite);
};
class FourScene : public cocos2d::Layer
{
private:
	CButton * SkipBtn;
	CButton * AirBtn;
	CButton * MagnetBtn;
	b2World* _b2World;
	b2Body *playerBody;
	b2Body *MagnetBody;
	b2Body *NewMagnetBody;
	b2Body* ropeBody[17];
	b2Body *dynamicBirdBody[5];
	cocos2d::Size visibleSize;
	cocos2d::Vec2 origin;
	cocos2d::Node * FourBackground;
	cocos2d::Point PntLoc;
	cocos2d::Sprite * PlayerSprite;
	cocos2d::Sprite * NewMagnetSprite;
	cocos2d::Sprite * fishSprite;
	float _fGameTime = 0;
	float _fFishTime = 0;
	float _fCreateFishTime = 0;
	float _fSpeed[5]; 
	float _fLoc;
	bool _bAirOpen = false;
	bool _bMagnetOpen = false;
	bool _bFishOpen = false;
	bool _bBird[5];

	b2Joint * fish;
	b2Joint * MagnetJoint;

	CContactListener_Four _contactListener;
	struct AirDraw_Four * HeadAir;
	struct AirDraw_Four * NewAir;

	GLESDebugDraw* _DebugDraw;
	virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags);
public:

	~FourScene();
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	void doStep(float dt);

	void StartScene();
	void readSceneFile();
	void setupWinSensor();
	void setupRopeJoint();
	void setupStartJoint();
	void setupBirds();
	void CreateFish();
	void CreateAir();
	void CreateMagnet(cocos2d::Point loc); //磁鐵

	cocos2d::EventListenerTouchOneByOne *_listener1;
	bool onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰開始事件
	void onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰移動事件
	void onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰結束事件 

																	   // implement the "static create()" method manually
	CREATE_FUNC(FourScene);

};

#endif // __FourScene_SCENE_H__