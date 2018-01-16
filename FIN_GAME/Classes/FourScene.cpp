#include "FourScene.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "ui/UIWidget.h"
#include "StartScene.h"
#include "StopScene.h"

#define PTM_RATIO 32.0f

USING_NS_CC;

using namespace cocostudio::timeline;
using namespace ui;

Scene* FourScene::createScene(const int score)
{
	auto scene = Scene::create();
	auto layer = FourScene::create();
	char Score[20] = "";
	sprintf(Score, "%d", score);
	layer->score = score;
	auto score_text = (cocos2d::ui::Text *)layer->FourBackground->getChildByName("score");
	score_text->setText(Score);
	scene->addChild(layer);
	return scene;
}
FourScene::~FourScene()
{
	if (_b2World != nullptr) delete _b2World;
}

bool FourScene::init()
{
	visibleSize = Director::getInstance()->getVisibleSize();
	origin = Director::getInstance()->getVisibleOrigin();

	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}
	for (int i = 0; i < 5; i++) {
		_bBird[i] = rand() % 2;
		_fSpeed[i] = rand() % 5 + 5;
	}

	FourBackground = CSLoader::createNode("Four.csb");
	addChild(FourBackground);
	PntLoc = FourBackground->getPosition();

	//Button
	AirBtn = CButton::create();
	AirBtn->setButtonInfo("new_cloud_normal.png", "new_cloud_on.png", "new_cloud.png", Point(1180.0f, 685.0f), true);
	AirBtn->setScale(1.0f);
	this->addChild(AirBtn, 10);
	MagnetBtn = CButton::create();
	MagnetBtn->setButtonInfo("new_magnet_normal.png", "new_magnet_on.png", "new_magnet_normal.png", Point(1250.0f, 685.0f), true);
	MagnetBtn->setScale(1.0f);
	this->addChild(MagnetBtn, 10);
	StopBtn = CButton::create();
	StopBtn->setButtonInfo("new_stop.png", "new_stop_on.png", "new_stop.png", Point(30.0f, 30.0f), true);
	StopBtn->setScale(0.8f);
	this->addChild(StopBtn, 10);
	SkipBtn = CButton::create();
	SkipBtn->setButtonInfo("skip_normal.png", "skip_on.png", "skip_normal.png", Point(1230.0f, 30.0f), true);
	SkipBtn->setScale(0.8f);
	this->addChild(SkipBtn, 10);

	//B2World
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);	//重力方向
	bool AllowSleep = true;					//允許睡著
	_b2World = new b2World(Gravity);		//創建世界
	_b2World->SetAllowSleeping(AllowSleep);	//設定物件允許睡著

	readSceneFile();
	setupRopeJoint();
	setupBirds();
	setupStartJoint();
	CreateFish();
	setupWinSensor();
	CreatePlayer();

	if (BOX2D_DEBUG) {
		//DebugDrawInit
		_DebugDraw = nullptr;
		_DebugDraw = new GLESDebugDraw(PTM_RATIO);
		//設定DebugDraw
		_b2World->SetDebugDraw(_DebugDraw);
		//選擇繪製型別
		uint32 flags = 0;
		flags += GLESDebugDraw::e_shapeBit;						//繪製形狀
		flags += GLESDebugDraw::e_pairBit;
		flags += GLESDebugDraw::e_jointBit;
		flags += GLESDebugDraw::e_centerOfMassBit;
		flags += GLESDebugDraw::e_aabbBit;
		//設定繪製類型
		_DebugDraw->SetFlags(flags);
	}

	_listener1 = EventListenerTouchOneByOne::create();	//創建一個一對一的事件聆聽器
	_listener1->onTouchBegan = CC_CALLBACK_2(FourScene::onTouchBegan, this);		//加入觸碰開始事件
	_listener1->onTouchMoved = CC_CALLBACK_2(FourScene::onTouchMoved, this);		//加入觸碰移動事件
	_listener1->onTouchEnded = CC_CALLBACK_2(FourScene::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(FourScene::doStep));

	return true;
}
void FourScene::doStep(float dt) {
	_fGameTime += dt;

	//磁鐵魚
	if (NewMagnetBody != NULL && MagnetBody != NULL)
	{
		auto waterheight = (Sprite *)FourBackground->getChildByName("wall_1");
		if (fishSprite->getPosition().y > waterheight->getPosition().y) {
			_fFishTime += dt; 
			if (fish != NULL) {
				_bFishOpen = true;
				_b2World->DestroyJoint(fish);
				fish = NULL;
			}
			if (NewMagnetBody != NULL && MagnetBody != NULL) {
				b2PrismaticJointDef JointDef;
				JointDef.Initialize(NewMagnetBody, MagnetBody, NewMagnetBody->GetPosition(), b2Vec2((NewMagnetBody->GetPosition().x) - (MagnetBody->GetPosition().x), (NewMagnetBody->GetPosition().y) - (MagnetBody->GetPosition().y)));
				MagnetJoint = _b2World->CreateJoint(&JointDef);
			}
		}
		if (_bFishOpen) {
			MagnetBody->SetGravityScale(0);
			if (MagnetBody->GetPosition().y < NewMagnetBody->GetPosition().y)
				MagnetBody->SetTransform(b2Vec2(MagnetBody->GetPosition().x, (MagnetBody->GetPosition().y) + dt * 30), MagnetBody->GetAngle());
			else
				MagnetBody->SetTransform(b2Vec2(MagnetBody->GetPosition().x, (MagnetBody->GetPosition().y) - dt * 30), MagnetBody->GetAngle());
			if (_fFishTime >= 5) {
				_bFishOpen = false;
				if (MagnetJoint != NULL) {
					_b2World->DestroyJoint(MagnetJoint);
					MagnetJoint = NULL;
				}
				if (NewMagnetBody != NULL) {
					NewMagnetSprite->setVisible(false);
					removeChild(NewMagnetSprite);
					_b2World->DestroyBody(NewMagnetBody);
					NewMagnetBody = NULL;
				}
				if (MagnetBody != NULL) {
					CreateGhost(fishSprite->getPosition(), 1);
					fishSprite->setVisible(false);
					removeChild(fishSprite);
					_b2World->DestroyBody(MagnetBody);
					MagnetBody = NULL;
				}
			}
		}
	}
	if (MagnetBody == NULL) {
		_fCreateFishTime += dt;
		if (_fCreateFishTime >= 3) {
			_fFishTime = 0;
			_fCreateFishTime = 0;
			CreateFish();
		}
	}

	//小鳥
	for (int i = 0; i < 5; i++) {
		if (dynamicBirdBody[i] != NULL) {
			auto waterheight = (Sprite *)FourBackground->getChildByName("wall_1");
			if (MagnetBody != NULL &&fishSprite->getPosition().y > waterheight->getPosition().y && _bFishOpen) {
				_fLoc = fishSprite->getPosition().x;
				if (_fLoc <= 300)_fLoc = 300;
				if (_fLoc >= 1100)_fLoc = 1100;
				if (dynamicBirdBody[i]->GetPosition().x*PTM_RATIO > _fLoc)
					dynamicBirdBody[i]->SetTransform(b2Vec2((dynamicBirdBody[i]->GetPosition().x) - dt * _fSpeed[i], (dynamicBirdBody[i]->GetPosition().y)), dynamicBirdBody[i]->GetAngle());
				else
					dynamicBirdBody[i]->SetTransform(b2Vec2((dynamicBirdBody[i]->GetPosition().x) + dt * _fSpeed[i], (dynamicBirdBody[i]->GetPosition().y)), dynamicBirdBody[i]->GetAngle());
			}
			else {
				if (!_bBird[i]) {
					dynamicBirdBody[i]->SetTransform(b2Vec2((dynamicBirdBody[i]->GetPosition().x) - dt * _fSpeed[i], (dynamicBirdBody[i]->GetPosition().y)), dynamicBirdBody[i]->GetAngle());
				}
				if (dynamicBirdBody[i]->GetPosition().x*PTM_RATIO < 300 && !_bBird[i]) {
					_bBird[i] = true;
					_fSpeed[i] = rand() % 10;
				}
				if (_bBird[i]) {
					dynamicBirdBody[i]->SetTransform(b2Vec2((dynamicBirdBody[i]->GetPosition().x) + dt * _fSpeed[i], (dynamicBirdBody[i]->GetPosition().y)), dynamicBirdBody[i]->GetAngle());
				}
				if (dynamicBirdBody[i]->GetPosition().x*PTM_RATIO > 1100 && _bBird) {
					_bBird[i] = false;
					_fSpeed[i] = rand() % 10;
				}
			}
		}
	}

	int velocityIterations = 8; // 速度迭代次數
	int positionIterations = 1; // 位置迭代次數，迭代次數一般設定為8~10 越高越真實但效率越差
	_b2World->Step(dt, velocityIterations, positionIterations);
	for (b2Body* body = _b2World->GetBodyList(); body; )
	{
		if (body->GetUserData() != NULL) {
			Sprite *ballData = (Sprite*)body->GetUserData();
			ballData->setPosition(body->GetPosition().x*PTM_RATIO, body->GetPosition().y*PTM_RATIO);
			ballData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
		}
		// 跑出螢幕外面就讓物體從 b2World 中移除
		if (body->GetType() == b2BodyType::b2_dynamicBody && body != playerBody && body != MagnetBody && body != dynamicBirdBody[0] && body != dynamicBirdBody[1] && body != dynamicBirdBody[2] && body != dynamicBirdBody[3] && body != dynamicBirdBody[4]) {
			float x = body->GetPosition().x * PTM_RATIO;
			float y = body->GetPosition().y * PTM_RATIO;
			Sprite* spriteData = (Sprite *)body->GetUserData();
			if (x > visibleSize.width || x < 0 || y >  visibleSize.height || y < 0) {
				if (body->GetUserData() != NULL) {
					Sprite* spriteData = (Sprite *)body->GetUserData();
					spriteData->setVisible(false);
					this->removeChild(spriteData);
				}
				_b2World->DestroyBody(body);
				body = NULL;
			}
			else if (!spriteData->isVisible()) {
				this->removeChild(spriteData);
				_b2World->DestroyBody(body);
				body = NULL;
			}
			else body = body->GetNext(); //否則就繼續更新下一個Body
		}
		//鳥死亡
		//else if (body == dynamicBirdBody[0] || body == dynamicBirdBody[1] || body == dynamicBirdBody[2] || body == dynamicBirdBody[3] || body == dynamicBirdBody[4]) {
		//	float x = body->GetPosition().x * PTM_RATIO;
		//	float y = body->GetPosition().y * PTM_RATIO;
		//	if (_contactListener._bBird) {
		//		if (body->GetUserData() != NULL) {
		//			Sprite* spriteData = (Sprite *)body->GetUserData();
		//			CreateGhost(spriteData->getPosition(), 2);
		//			spriteData->setVisible(false);
		//			this->removeChild(spriteData);
		//			_contactListener._bBird = false;
		//		}
		//		_b2World->DestroyBody(body);
		//		body = NULL;
		//	}
		//	else body = body->GetNext(); //否則就繼續更新下一個Body
		//}
		//死亡
		else if (body == playerBody) {
			float x = body->GetPosition().x * PTM_RATIO;
			float y = body->GetPosition().y * PTM_RATIO;
			if (x > visibleSize.width || x < 0 || y < 0 || _contactListener._bPlayerAccident) {
				_contactListener._bPlayerAccident = false;
				if (body->GetUserData() != NULL) {
					Sprite* spriteData = (Sprite *)body->GetUserData();
					CreateGhost(spriteData->getPosition(), 0);
					spriteData->setVisible(false);
					this->removeChild(spriteData);
				}
				_b2World->DestroyBody(body);
				playerBody = NULL;
				body = NULL;
			}
			else body = body->GetNext(); //否則就繼續更新下一個Body
		}
		else body = body->GetNext(); //否則就繼續更新下一個Body
	}
	if (_contactListener.win) {
		StartScene();
	}
}
CContactListener_Four::CContactListener_Four()
{
}
void CContactListener_Four::setCollisionTargetBird(int i, cocos2d::Sprite &targetSprite) {
	BirdSprite[i] = &targetSprite;
}
void CContactListener_Four::setCollisionTargetPlayer(cocos2d::Sprite &targetSprite) {
	_Playersprite = &targetSprite;
}
void CContactListener_Four::setCollisionTarget(cocos2d::Sprite &targetSprite)
{
	if (_HeadtargetSprite == NULL) {
		_HeadtargetSprite = new AirDiet_Four;
		_HeadtargetSprite->_sprite = &targetSprite;
		_HeadtargetSprite->_NexttargetSprite = NULL;
		_NewtargetSprite = _HeadtargetSprite;
	}
	else {
		struct AirDiet_Four * Current = new AirDiet_Four;
		Current->_sprite = &targetSprite;
		Current->_NexttargetSprite = NULL;
		if (_NewtargetSprite == NULL)for (_NewtargetSprite = _HeadtargetSprite; _NewtargetSprite->_NexttargetSprite != NULL; _NewtargetSprite = _NewtargetSprite->_NexttargetSprite) {}
		_NewtargetSprite->_NexttargetSprite = Current;
		_NewtargetSprite = Current;
	}
}

//
// 只要是兩個 body 的 fixtures 碰撞，就會呼叫這個函式
//
void CContactListener_Four::BeginContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();
	for (_NewtargetSprite = _HeadtargetSprite; _NewtargetSprite != NULL; _NewtargetSprite = _NewtargetSprite->_NexttargetSprite) {
		if (BodyA->GetUserData() == _NewtargetSprite->_sprite && BodyA->GetGravityScale() == -1) {
			_NewtargetSprite->_sprite->setVisible(false);
		}
		else if (BodyB->GetUserData() == _NewtargetSprite->_sprite && BodyB->GetGravityScale() == -1) {
			_NewtargetSprite->_sprite->setVisible(false);
		}
	}
	if (BodyA->GetFixtureList()->GetDensity() == -10000.0f) {
		if (BodyB->GetUserData() == _Playersprite)
			win = true;
	}
	if (BodyA->GetUserData() == _Playersprite) {
		for (int i = 0; i < 5; i++) {
			if (BodyB->GetUserData() == BirdSprite[i]) {
				_bPlayerAccident = true;
				_bBird = true;
			}
		}
	}
	for (int i = 0; i < 5; i++) {
		if (BodyA->GetUserData() == BirdSprite[i]) {
			if (BodyB->GetUserData() == _Playersprite) {
				_bPlayerAccident = true;
				_bBird = true;
			}
		}
	}
}

//碰撞結束
void CContactListener_Four::EndContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();
}
void FourScene::setupWinSensor() {
	auto sensorSprite = (Sprite *)FourBackground->getChildByName("win_sensor");
	Point loc = sensorSprite->getPosition();
	Size  size = sensorSprite->getContentSize();
	float scale = sensorSprite->getScale();
	sensorSprite->setVisible(false);
	b2BodyDef sensorBodyDef;
	sensorBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	sensorBodyDef.type = b2_staticBody;

	b2Body* SensorBody = _b2World->CreateBody(&sensorBodyDef);
	b2PolygonShape sensorShape;
	sensorShape.SetAsBox(size.width *0.5f * scale / PTM_RATIO, size.height*0.5f*scale / PTM_RATIO);

	b2FixtureDef SensorFixtureDef;
	SensorFixtureDef.shape = &sensorShape;
	SensorFixtureDef.isSensor = true;	// 設定為 Sensor
	SensorFixtureDef.density = -10000; // 故意設定成這個值，方便碰觸時候的判斷
	SensorBody->CreateFixture(&SensorFixtureDef);
}
void FourScene::readSceneFile() {
	char tmp[20] = "";

	// 產生 EdgeShape 的 body
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // 設定這個 Body 為 靜態的
	bodyDef.userData = NULL;
	// 在 b2World 中產生該 Body, 並傳回產生的 b2Body 物件的指標
	// 產生一次，就可以讓後面所有的 Shape 使用
	b2Body *body = _b2World->CreateBody(&bodyDef);

	b2PolygonShape rectShape;
	b2FixtureDef fixtureDef; // 產生 Fixture
	fixtureDef.shape = &rectShape;

	for (size_t i = 1; i <= 1; i++)
	{
		// 產生所需要的 Sprite file name int plist 
		// 此處取得的都是相對於 csbRoot 所在位置的相對座標
		// 在計算 edgeShape 的相對應座標時，必須進行轉換
		sprintf(tmp, "wall_%d", i);
		auto rectSprite = (Sprite *)FourBackground->getChildByName(tmp);
		Size ts = rectSprite->getContentSize();
		Point loc = rectSprite->getPosition();
		float angle = rectSprite->getRotation();
		float scaleX = rectSprite->getScaleX();	// 水平的線段圖示假設都只有對 X 軸放大
		float scaleY = rectSprite->getScaleY();	// 水平的線段圖示假設都只有對 X 軸放大

												// rectShape 的四個端點, 0 右上、 1 左上、 2 左下 3 右下
		Point lep[4], wep[4];
		lep[0].x = (ts.width - 4) / 2.0f;;  lep[0].y = (ts.height - 4) / 2.0f;
		lep[1].x = -(ts.width - 4) / 2.0f;; lep[1].y = (ts.height - 4) / 2.0f;
		lep[2].x = -(ts.width - 4) / 2.0f;; lep[2].y = -(ts.height - 4) / 2.0f;
		lep[3].x = (ts.width - 4) / 2.0f;;  lep[3].y = -(ts.height - 4) / 2.0f;

		// 所有的線段圖示都是是本身的中心點為 (0,0)，
		// 根據縮放、旋轉產生所需要的矩陣
		// 根據寬度計算出兩個端點的座標，然後呈上開矩陣
		// 然後進行旋轉，
		// Step1: 先CHECK 有無旋轉，有旋轉則進行端點的計算
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scaleX;  // 先設定 X 軸的縮放
		modelMatrix.m[5] = scaleY;  // 先設定 Y 軸的縮放
		cocos2d::Mat4::createRotationZ(angle*M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = PntLoc.x + loc.x; //設定 Translation，自己的加上父親的
		modelMatrix.m[7] = PntLoc.y + loc.y; //設定 Translation，自己的加上父親的
		for (size_t j = 0; j < 4; j++)
		{
			wep[j].x = lep[j].x * modelMatrix.m[0] + lep[j].y * modelMatrix.m[1] + modelMatrix.m[3];
			wep[j].y = lep[j].x * modelMatrix.m[4] + lep[j].y * modelMatrix.m[5] + modelMatrix.m[7];
		}
		b2Vec2 vecs[] = {
			b2Vec2(wep[0].x / PTM_RATIO, wep[0].y / PTM_RATIO),
			b2Vec2(wep[1].x / PTM_RATIO, wep[1].y / PTM_RATIO),
			b2Vec2(wep[2].x / PTM_RATIO, wep[2].y / PTM_RATIO),
			b2Vec2(wep[3].x / PTM_RATIO, wep[3].y / PTM_RATIO) };

		rectShape.Set(vecs, 4);
		body->CreateFixture(&fixtureDef);
	}
}
void FourScene::setupBirds() {
	char tmp[20] = "";
	Point loc[5];
	Size  size[5];
	float scaleX[5];
	float scaleY[5];
	b2Body *staticBirdBody[5];
	b2PrismaticJoint* PriJoint[5];

	b2BodyDef staticbodyDef;
	staticbodyDef.type = b2_staticBody;
	staticbodyDef.userData = NULL;
	b2FixtureDef fixtureDef;
	b2BodyDef dynamicbodyDef;
	
	for (int i = 0; i < 5; i++) {
		sprintf(tmp, "bird_%d", i+1);
		birdSprite[i] = (Sprite *)FourBackground->getChildByName(tmp);
		size[i] = birdSprite[i]->getContentSize();
		loc[i] = birdSprite[i]->getPosition();
		scaleX[i] = birdSprite[i]->getScaleX();
		scaleY[i] = birdSprite[i]->getScaleY();
		staticbodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		staticBirdBody[i] = _b2World->CreateBody(&staticbodyDef);
		b2CircleShape ballShape;
		ballShape.m_radius = 0.01f*(size[i].width - 4) *0.5f*scaleX[i] / PTM_RATIO;
		fixtureDef.shape = &ballShape;
		staticBirdBody[i]->CreateFixture(&fixtureDef);


		b2PolygonShape boxShape;
		dynamicbodyDef.type = b2_dynamicBody;
		dynamicbodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		dynamicbodyDef.userData = birdSprite[i];
	    dynamicBirdBody[i] = _b2World->CreateBody(&dynamicbodyDef);
		boxShape.SetAsBox((size[i].width - 4)*scaleX[i]*0.5f / PTM_RATIO, (size[i].height - 4)*scaleY[i]*0.5f / PTM_RATIO);
		fixtureDef.shape = &boxShape;
		fixtureDef.density = 1.0f;// 設定密度
		dynamicBirdBody[i]->CreateFixture(&fixtureDef);

		_contactListener.setCollisionTargetBird(i, *birdSprite[i]);

		b2PrismaticJointDef PrJoint; // 平移關節
		PrJoint.Initialize(staticBirdBody[i], dynamicBirdBody[i], dynamicBirdBody[i]->GetWorldCenter(), b2Vec2(1.0f, 0));
		PriJoint[i] = (b2PrismaticJoint*)_b2World->CreateJoint(&PrJoint);
	}
}
void FourScene::setupRopeJoint() {
	char tmp[20] = "";
	Point locball[3];
	Size  sizeball[3];
	float scaleball[3];
	b2Body *staticballBody[3];
	b2Body *dynamicballBody[3];
	b2RevoluteJoint*  RevJoint[3];

	b2BodyDef staticbodyDef;
	staticbodyDef.type = b2_staticBody;
	staticbodyDef.userData = NULL;
	b2FixtureDef fixtureDef;
	b2BodyDef dynamicbodyDef;

	for (size_t i = 0; i < 3; i++) {
		sprintf(tmp, "circle_fish_%d", i+1);
		auto ballSprite = (Sprite *)FourBackground->getChildByName(tmp);
		sizeball[i] = ballSprite->getContentSize();
		locball[i] = ballSprite->getPosition();
		scaleball[i] = ballSprite->getScale();
		staticbodyDef.position.Set(locball[i].x / PTM_RATIO, locball[i].y / PTM_RATIO);
		staticballBody[i] = _b2World->CreateBody(&staticbodyDef);
		b2CircleShape ballShape;
		ballShape.m_radius = (sizeball[i].width - 4) *0.5f*scaleball[i]*0.1f / PTM_RATIO;
		fixtureDef.shape = &ballShape;
		staticballBody[i]->CreateFixture(&fixtureDef);

		dynamicbodyDef.type = b2_dynamicBody;
		dynamicbodyDef.position.Set(locball[i].x / PTM_RATIO, locball[i].y / PTM_RATIO);
		dynamicbodyDef.userData = ballSprite;
		dynamicballBody[i] = _b2World->CreateBody(&dynamicbodyDef);
		ballShape.m_radius = (sizeball[i].width - 4) *0.5f*scaleball[i] / PTM_RATIO;
		fixtureDef.shape = &ballShape;
		fixtureDef.density = 1.0f;// 設定密度
		dynamicballBody[i]->CreateFixture(&fixtureDef);

		b2RevoluteJointDef RJoint;	// 旋轉關節
		RJoint.Initialize(staticballBody[i], dynamicballBody[i], dynamicballBody[i]->GetWorldCenter());
		RevJoint[i] = (b2RevoluteJoint*)_b2World->CreateJoint(&RJoint);
	}

	b2GearJointDef GJoint;
	GJoint.bodyA = dynamicballBody[0];
	GJoint.bodyB = dynamicballBody[1];
	GJoint.joint1 = RevJoint[0];
	GJoint.joint2 = RevJoint[1];
	GJoint.ratio = -1;
	_b2World->CreateJoint(&GJoint);

	GJoint.bodyA = dynamicballBody[1];
	GJoint.bodyB = dynamicballBody[2];
	GJoint.joint1 = RevJoint[1];
	GJoint.joint2 = RevJoint[2];
	GJoint.ratio = -1;
	_b2World->CreateJoint(&GJoint);


	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.userData = NULL;
	b2PolygonShape boxShape;

	//產生繩子關節
	/*b2RopeJointDef JointDef;
	JointDef.bodyA = dynamicballBody[1];
	JointDef.bodyB = MagnetBody;
	JointDef.localAnchorA = b2Vec2(-sizeball[1].width*scaleball[1]*0.5f / PTM_RATIO, 0);
	JointDef.localAnchorB = b2Vec2(sizeMagnet.width*scaleMagnetX*0.5f / PTM_RATIO, 0);
	JointDef.maxLength = sqrtf(powf((locball[1].y - locMagnet.y) / PTM_RATIO,2)+ powf((locball[1].x - locMagnet.x) / PTM_RATIO, 2));
	JointDef.collideConnected = true;
	b2RopeJoint* J = (b2RopeJoint*)_b2World->CreateJoint(&JointDef);*/

	// 中間再以線段相連，
	Sprite *ropeSprite[17];
	Point loc[17];
	Size  size[17];
	ropeBody[17];

	bodyDef.type = b2_dynamicBody;
	// 因為是繩子所以重量不要太重
	fixtureDef.density = 0.1f;  fixtureDef.friction = 1.0f; fixtureDef.restitution = 0.0f;
	fixtureDef.shape = &boxShape;
	// 產生一系列的繩子段落 rope01_01 ~ rope01_15，同時接起來
	for (int i = 0; i < 17; i++)
	{
		sprintf(tmp, "rope_%d", i + 1);
		ropeSprite[i] = (Sprite *)FourBackground->getChildByName(tmp);
		loc[i] = ropeSprite[i]->getPosition();
		size[i] = ropeSprite[i]->getContentSize();

		bodyDef.position.Set((loc[i].x + PntLoc.x) / PTM_RATIO, (loc[i].y + PntLoc.y) / PTM_RATIO);
		bodyDef.userData = ropeSprite[i];
		ropeBody[i] = _b2World->CreateBody(&bodyDef);
		ropeBody[i]->SetGravityScale(0.1f);
		boxShape.SetAsBox((size[i].width)*0.5f / PTM_RATIO, (size[i].height)*0.5f / PTM_RATIO);
		ropeBody[i]->CreateFixture(&fixtureDef);
	}
	// 利用 RevoluteJoint 將線段彼此連接在一起
	// 先連結 ropeHeadBody 與  ropeBody[0]

	float locAnchor = 0.5f*(size[0].height - 5) / PTM_RATIO;
	b2RevoluteJointDef revJoint;
	revJoint.bodyA = dynamicballBody[1];
	revJoint.localAnchorA.Set(-sizeball[1].width*scaleball[1] * 0.5f / PTM_RATIO, 0);
	revJoint.bodyB = ropeBody[0];
	revJoint.localAnchorB.Set(0, locAnchor);
	_b2World->CreateJoint(&revJoint);
	for (int i = 0; i < 16; i++) {
		revJoint.bodyA = ropeBody[i];
		revJoint.localAnchorA.Set(0, -locAnchor);
		revJoint.bodyB = ropeBody[i + 1];
		revJoint.localAnchorB.Set(0, locAnchor);
		_b2World->CreateJoint(&revJoint);
	}
}
void FourScene::setupStartJoint() {
	b2BodyDef staticbodyDef;
	staticbodyDef.type = b2_staticBody;
	staticbodyDef.userData = NULL;
	b2FixtureDef fixtureDef;
	b2BodyDef dynamicbodyDef;

	//板子
	auto boxSprite = (Sprite *)FourBackground->getChildByName("wall_start");
	Size sizebox = boxSprite->getContentSize();
	Point locbox = boxSprite->getPosition();
	float scaleboxX = boxSprite->getScaleX();
	float scaleboxY = boxSprite->getScaleY();
	staticbodyDef.position.Set(locbox.x / PTM_RATIO, locbox.y / PTM_RATIO);
	b2Body *staticboxBody = _b2World->CreateBody(&staticbodyDef);
	b2CircleShape ballShape;
	ballShape.m_radius = 0.1f * (sizebox.height - 4) *0.5f / PTM_RATIO;
	fixtureDef.shape = &ballShape;
	staticboxBody->CreateFixture(&fixtureDef);
	b2PolygonShape boxShape;
	/*boxShape.SetAsBox((sizebox.width - 4)*scaleboxX*0.5f / PTM_RATIO, (sizebox.height - 4)*scaleboxY*0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	staticboxBody->CreateFixture(&fixtureDef);*/

	dynamicbodyDef.type = b2_dynamicBody;
	dynamicbodyDef.position.Set(locbox.x / PTM_RATIO, locbox.y / PTM_RATIO);
	dynamicbodyDef.userData = boxSprite;
	b2Body *dynamicboxBody = _b2World->CreateBody(&dynamicbodyDef);
	boxShape.SetAsBox((sizebox.width - 4)*scaleboxX*0.5f / PTM_RATIO, (sizebox.height - 4)*scaleboxY*0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 1.0f;// 設定密度
	dynamicboxBody->CreateFixture(&fixtureDef);

	b2RevoluteJointDef RJoint;	// 旋轉關節
	RJoint.Initialize(staticboxBody, dynamicboxBody, dynamicboxBody->GetWorldCenter());
	b2RevoluteJoint*  RevJoint_1;
	RevJoint_1 = (b2RevoluteJoint*)_b2World->CreateJoint(&RJoint);

	//球
	auto ballSprite = (Sprite *)FourBackground->getChildByName("circle_start");
	Size sizeball = ballSprite->getContentSize();
	Point locball = ballSprite->getPosition();
	float scaleball = ballSprite->getScale();
	staticbodyDef.position.Set(locball.x / PTM_RATIO, locball.y / PTM_RATIO);
	b2Body *staticballBody = _b2World->CreateBody(&staticbodyDef);
	//b2CircleShape ballShape;
	ballShape.m_radius = (sizeball.width - 4) *0.5f*scaleball*0.1f / PTM_RATIO;
	fixtureDef.shape = &ballShape;
	staticballBody->CreateFixture(&fixtureDef);

	dynamicbodyDef.type = b2_dynamicBody;
	dynamicbodyDef.position.Set(locball.x / PTM_RATIO, locball.y / PTM_RATIO);
	dynamicbodyDef.userData = ballSprite;
	b2Body *dynamicballBody = _b2World->CreateBody(&dynamicbodyDef);
	ballShape.m_radius = (sizeball.width - 4) *0.5f*scaleball / PTM_RATIO;
	fixtureDef.shape = &ballShape;
	fixtureDef.density = 1.0f;// 設定密度
	dynamicballBody->CreateFixture(&fixtureDef);

	RJoint.Initialize(staticballBody, dynamicballBody, dynamicballBody->GetWorldCenter());
	b2RevoluteJoint*  RevJoint_2;
	RevJoint_2 = (b2RevoluteJoint*)_b2World->CreateJoint(&RJoint);

	//產生齒輪關節(同步同向)
	b2GearJointDef GJoint;
	GJoint.bodyA = dynamicballBody;
	GJoint.bodyB = dynamicboxBody;
	GJoint.joint1 = RevJoint_1;
	GJoint.joint2 = RevJoint_2;
	GJoint.ratio = -1;
	_b2World->CreateJoint(&GJoint);
}
void FourScene::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
bool FourScene::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) {
	Point touchLoc = pTouch->getLocation();
	if (AirBtn->touchesBegin(touchLoc)) {
		_bMagnetOpen = false;
		MagnetBtn->closebtn();
	}
	if (_bAirOpen) {
		HeadAir = new AirDraw_Four;
		HeadAir->pos = touchLoc;
		HeadAir->NextAir = NULL;
		NewAir = HeadAir;
	}
	if (MagnetBtn->touchesBegin(touchLoc)) {
		_bAirOpen = false;
		AirBtn->closebtn();
	}
	if (SkipBtn->touchesBegin(touchLoc)) {
		_bAirOpen = false;
		AirBtn->closebtn();
		_bMagnetOpen = false;
		MagnetBtn->closebtn();
	}
	if (StopBtn->touchesBegin(touchLoc)) {
		_bAirOpen = false;
		AirBtn->closebtn();
		_bMagnetOpen = false;
		MagnetBtn->closebtn();
	}
	return true;
}
void FourScene::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) {
	Point touchLoc = pTouch->getLocation();
	if (_bAirOpen) {
		if (ccpDistance(NewAir->pos, touchLoc) >= 32) {
			struct AirDraw_Four * Current = new AirDraw_Four;
			Current->pos = touchLoc;
			Current->NextAir = NULL;
			NewAir->NextAir = Current;
			NewAir = Current;
		}
	}
}
void FourScene::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) {
	Point touchLoc = pTouch->getLocation();
	if (AirBtn->touchesEnded(touchLoc)) {
		_bAirOpen = !_bAirOpen;
	}
	else if (_bAirOpen) {
		CreateAir();
	}
	if (MagnetBtn->touchesEnded(touchLoc)) {
		_bMagnetOpen = !_bMagnetOpen;
	}
	else if (_bMagnetOpen) {
		CreateMagnet(touchLoc);
	}
	if (SkipBtn->touchesEnded(touchLoc)) {
		StartScene();
	}
	if (StopBtn->touchesEnded(touchLoc)) {
		stopScene();
	}
}
void FourScene::CreatePlayer() {
	auto locSprite = (Sprite *)FourBackground->getChildByName("player");
	Point loc = locSprite->getPosition();
	PlayerSprite = Sprite::createWithSpriteFrameName("new_ballon.png");
	PlayerSprite->setScale(0.75f);
	PlayerSprite->setVisible(true);
	this->addChild(PlayerSprite, 2);
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.userData = PlayerSprite;
	bodyDef.position.Set(PntLoc.x + loc.x / PTM_RATIO, PntLoc.y + loc.y / PTM_RATIO);
	playerBody = _b2World->CreateBody(&bodyDef);
	b2CircleShape ballShape;
	Size ballsize = PlayerSprite->getContentSize();
	ballShape.m_radius = 0.75f * (ballsize.width - 4) *0.5f / PTM_RATIO;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &ballShape;
	fixtureDef.restitution = 0.5f;
	fixtureDef.density = 0.1f;
	fixtureDef.friction = 0.15f;
	playerBody->CreateFixture(&fixtureDef);

	_contactListener.setCollisionTargetPlayer(*PlayerSprite);
}

void FourScene::CreateGhost(Point loc, int who) {
	switch (who) {
	case 0:
		if (ghostSprite == nullptr) {
			ghostSprite = Sprite::createWithSpriteFrameName("new_ghost.png");
			ghostSprite->setScale(0.75f);
			ghostSprite->setPosition(loc);
			this->addChild(ghostSprite, 2);
			auto MoveAction = cocos2d::MoveTo::create(3.0f, Point(30, 685));
			auto callback = CallFunc::create(this, callfunc_selector(FourScene::ghostFinished));
			ghostSprite->runAction(Sequence::create(MoveAction, callback, NULL));
		}
		break;
	case 1:
		if (fishghostSprite == nullptr) {
			fishghostSprite = Sprite::createWithSpriteFrameName("new_ghost.png");
			fishghostSprite->setScale(0.75f);
			fishghostSprite->setPosition(loc);
			this->addChild(fishghostSprite, 2);
			auto MoveAction = cocos2d::MoveTo::create(3.0f, Point(30, 685));
			auto callback = CallFunc::create(this, callfunc_selector(FourScene::fishghostFinished));
			fishghostSprite->runAction(Sequence::create(MoveAction, callback, NULL));
		}
		break;
	case 2:
		if (birdghostSprite == nullptr) {
			birdghostSprite = Sprite::createWithSpriteFrameName("new_ghost.png");
			birdghostSprite->setScale(0.75f);
			birdghostSprite->setPosition(loc);
			this->addChild(birdghostSprite, 2);
			auto MoveAction = cocos2d::MoveTo::create(3.0f, Point(30, 685));
			auto callback = CallFunc::create(this, callfunc_selector(FourScene::birdghostFinished));
			birdghostSprite->runAction(Sequence::create(MoveAction, callback, NULL));
		}
		break;
	}
}
void FourScene::CreateAir() {
	for (NewAir = HeadAir; NewAir != NULL; NewAir = NewAir->NextAir) {
		auto AirSprite = Sprite::createWithSpriteFrameName("new_cloud.png");
		AirSprite->setScale(1.0f);
		this->addChild(AirSprite, 2);
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.userData = AirSprite;
		bodyDef.position.Set((NewAir->pos).x / PTM_RATIO, (NewAir->pos).y / PTM_RATIO);
		b2Body *AirBody = _b2World->CreateBody(&bodyDef);
		AirBody->SetGravityScale(-1.0f);
		b2CircleShape ballShape;
		Size ballsize = AirSprite->getContentSize();
		ballShape.m_radius = 0.5f * (ballsize.width - 4) *0.5f / PTM_RATIO;
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &ballShape;
		fixtureDef.restitution = 0.75f;
		fixtureDef.density = 0.3f;
		fixtureDef.friction = 0.15f;
		AirBody->CreateFixture(&fixtureDef);
		_b2World->SetContactListener(&_contactListener);
		_contactListener.setCollisionTarget(*AirSprite);
	}
}
void FourScene::CreateMagnet(Point loc) {
	if (NewMagnetBody != NULL) {
		removeChild(NewMagnetSprite);
		_b2World->DestroyBody(NewMagnetBody);
		NewMagnetBody = NULL;
	}
	NewMagnetSprite = Sprite::createWithSpriteFrameName("new_magnet.png");
	NewMagnetSprite->setScale(1.0);
	this->addChild(NewMagnetSprite, 2);
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.userData = NewMagnetSprite;
	bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	NewMagnetBody = _b2World->CreateBody(&bodyDef);
	NewMagnetBody->SetGravityScale(0);
	b2PolygonShape boxShape;
	boxShape.SetAsBox((NewMagnetSprite->getContentSize().width - 4)*0.5*0.4 / PTM_RATIO, (NewMagnetSprite->getContentSize().height - 4)*0.5*0.4 / PTM_RATIO);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	NewMagnetBody->CreateFixture(&fixtureDef);

	/*if (MagnetBody != NULL) {
		b2PrismaticJointDef JointDef;
		JointDef.Initialize(NewMagnetBody, MagnetBody, NewMagnetBody->GetPosition(), b2Vec2((NewMagnetBody->GetPosition().x) - (MagnetBody->GetPosition().x), (NewMagnetBody->GetPosition().y) - (MagnetBody->GetPosition().y)));
		MagnetJoint = _b2World->CreateJoint(&JointDef);
	}*/
}
void FourScene::CreateFish() {

	b2FixtureDef fixtureDef;
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.userData = NULL;
	b2PolygonShape boxShape;

	//磁鐵
	auto locSprite = (Sprite *)FourBackground->getChildByName("rope_17");
	auto MSprite = (Sprite *)FourBackground->getChildByName("ployblock_1");
	Size sizeMagnet = MSprite->getContentSize();
	Point locMagnet = locSprite->getPosition();
	float scaleMagnetX = MSprite->getScaleX();
	float scaleMagnetY = MSprite->getScaleY();
	fishSprite = Sprite::createWithSpriteFrameName("new_fish.png");
	fishSprite->setScale(scaleMagnetX, scaleMagnetY);
	this->addChild(fishSprite, 2);
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(locMagnet.x / PTM_RATIO, locMagnet.y / PTM_RATIO);
	bodyDef.userData = fishSprite;
	MagnetBody = _b2World->CreateBody(&bodyDef);
	MagnetBody->SetGravityScale(0.4);
	boxShape.SetAsBox((sizeMagnet.width - 4)*scaleMagnetX*0.5f / PTM_RATIO, (sizeMagnet.height - 4)*scaleMagnetY*0.5f / PTM_RATIO);
	fixtureDef.density = 0.1f;// 設定密度
	fixtureDef.shape = &boxShape;
	MagnetBody->CreateFixture(&fixtureDef);

	/*if (NewMagnetBody != NULL) {
		b2PrismaticJointDef JointDef;
		JointDef.Initialize(NewMagnetBody, MagnetBody, NewMagnetBody->GetPosition(), b2Vec2((NewMagnetBody->GetPosition().x) - (MagnetBody->GetPosition().x), (NewMagnetBody->GetPosition().y) - (MagnetBody->GetPosition().y)));
		MagnetJoint = _b2World->CreateJoint(&JointDef);
	}*/

	b2DistanceJointDef JointDef;
	JointDef.Initialize(ropeBody[16], MagnetBody, ropeBody[16]->GetPosition(), MagnetBody->GetPosition());
	fish = _b2World->CreateJoint(&JointDef);
}
void FourScene::StartScene() {
	// 先將這個 SCENE 的 Update(這裡使用 OnFrameMove, 從 schedule update 中移出)
	this->unschedule(schedule_selector(FourScene::doStep));
	//SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("mainscene.plist");
	// 上面這行何時需要放在這裡稍後會說明
	// 設定場景切換的特效
	TransitionFade *pageTurn = TransitionFade::create(1.0f, StartScene::createScene());
	Director::getInstance()->replaceScene(pageTurn);
	//SimpleAudioEngine::getInstance()->stopBackgroundMusic();
}
void FourScene::stopScene() {
	RenderTexture *renderTexture = RenderTexture::create(visibleSize.width, visibleSize.height);
	renderTexture->begin();
	this->getParent()->visit();
	renderTexture->end();
	auto scene = StopScene::createScene(renderTexture, score, 4);
	Director::sharedDirector()->pushScene(scene);
}
void FourScene::ghostFinished() {
	_fGameTime = 0;
	CreatePlayer();
	score++;
	char Score[20] = "";
	sprintf(Score, "%d", score);
	auto score_text = (cocos2d::ui::Text *)FourBackground->getChildByName("score");
	score_text->setText(Score);
	this->removeChild(ghostSprite);
	ghostSprite = nullptr;
}
void FourScene::fishghostFinished() {
	score++;
	char Score[20] = "";
	sprintf(Score, "%d", score);
	auto score_text = (cocos2d::ui::Text *)FourBackground->getChildByName("score");
	score_text->setText(Score);
	this->removeChild(fishghostSprite);
	fishghostSprite = nullptr;
}
void FourScene::birdghostFinished() {
	score++;
	char Score[20] = "";
	sprintf(Score, "%d", score);
	auto score_text = (cocos2d::ui::Text *)FourBackground->getChildByName("score");
	score_text->setText(Score);
	this->removeChild(birdghostSprite);
	birdghostSprite = nullptr;
}