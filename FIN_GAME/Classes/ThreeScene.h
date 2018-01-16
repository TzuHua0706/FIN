#ifndef __THREESCENE_H__
#define __THREESCENE_H__

#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "Common/CButton.h"
#include "Common/GLES-Render.h"

struct AirDraw_Three
{
	cocos2d::Point pos;
	struct AirDraw_Three * NextAir;
};
struct water_Three
{
	cocos2d::Sprite * _sprite;
	float _ftime = 0;
	struct water_Three * NextWater;
};
struct AirDiet_Three {
	cocos2d::Sprite * _sprite;
	struct AirDiet_Three * _NexttargetSprite;
};
class CContactListener_Three : public b2ContactListener
{
public:
	struct AirDiet_Three * _HeadtargetSprite = NULL;
	struct AirDiet_Three * _NewtargetSprite;
	cocos2d::Sprite * _Playersprite = nullptr;
	cocos2d::Sprite * _carsprite = nullptr;
	int _iBridgeNum = 0;
	bool _bCarAccident = false;
	bool _bPlayerAccident = false;
	bool win = false;

	CContactListener_Three();
	//碰撞開始
	virtual void BeginContact(b2Contact* contact);
	//碰撞結束
	virtual void EndContact(b2Contact* contact);
	void setCollisionTarget(cocos2d::Sprite &targetSprite);
	void setCollisionTargetCar(cocos2d::Sprite &targetSprite);
	void setCollisionTargetPlayer(cocos2d::Sprite &targetSprite);
};
class ThreeScene : public cocos2d::Layer
{
private:
	CButton * SkipBtn;
	CButton * StopBtn;
	CButton * AirBtn;
	CButton * MagnetBtn;
	b2World* _b2World;
	b2Body *playerBody = NULL;
	b2Body *MagnetBody = NULL;
	b2Body *NewMagnetBody;
	b2Body *dynamicwheelBody = NULL;
	b2Body *wheelBody_2 = NULL;
	b2Body *carBody = NULL;
	cocos2d::Size visibleSize;
	cocos2d::Vec2 origin;
	cocos2d::Node * ThreeBackground;
	cocos2d::Point PntLoc;
	cocos2d::Sprite * PlayerSprite;
	cocos2d::Sprite * NewMagnetSprite;
	cocos2d::Sprite * ghostSprite = nullptr;
	cocos2d::Sprite * carghostSprite = nullptr;
	int _icloud = 0;
	int score = 0;
	float _fcloudTime = 1;
	float _fGameTime = 0;
	float _fCarTime = 0;
	bool _bAirOpen = false;
	bool _bMagnetOpen = false;
	bool _bCarOpen = true; //車子存不存在(true-不存在、false-已存在)
	bool _bCarGo = false; //車子能否生成、燈的位子

	CContactListener_Three _contactListener;
	struct AirDraw_Three * HeadAir;
	struct AirDraw_Three * NewAir;

	GLESDebugDraw* _DebugDraw;
	virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags);
public:

	~ThreeScene();
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene(const int score);

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	void doStep(float dt);

	void nextScene();
	void stopScene();
	void readSceneFile();
	void setupWinSensor();
	void setupRopeJoint();
	void setupStartJoint();
	void setupLightJoint();
	void setupBridgeSensor();
	void setupCar();
	void CreateAir();
	void CreateMagnet(cocos2d::Point loc); //磁鐵
	void CreatePlayer();
	void CreateGhost(cocos2d::Point loc , int who); //0氣球死 1車死
	void ghostFinished();
	void carghostFinished();

	cocos2d::EventListenerTouchOneByOne *_listener1;
	bool onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰開始事件
	void onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰移動事件
	void onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰結束事件 

																	   // implement the "static create()" method manually
	CREATE_FUNC(ThreeScene);

};

#endif // __ThreeScene_SCENE_H__
