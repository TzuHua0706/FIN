#include "ThreeScene.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "FourScene.h"

#define PTM_RATIO 32.0f

USING_NS_CC;

using namespace cocostudio::timeline;

Scene* ThreeScene::createScene()
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = ThreeScene::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}
ThreeScene::~ThreeScene()
{
	if (_b2World != nullptr) delete _b2World;
}

bool ThreeScene::init()
{
	visibleSize = Director::getInstance()->getVisibleSize();
	origin = Director::getInstance()->getVisibleOrigin();

	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}

	ThreeBackground = CSLoader::createNode("Three.csb");
	addChild(ThreeBackground);
	PntLoc = ThreeBackground->getPosition();

	//Button
	AirBtn = CButton::create();
	AirBtn->setButtonInfo("new_cloud_normal.png", "new_cloud_on.png", "new_cloud.png", Point(1180.0f, 685.0f), true);
	AirBtn->setScale(1.0f);
	this->addChild(AirBtn, 10);
	MagnetBtn = CButton::create();
	MagnetBtn->setButtonInfo("new_magnet_normal.png", "new_magnet_on.png", "new_magnet_normal.png", Point(1250.0f, 685.0f), true);
	MagnetBtn->setScale(1.0f);
	this->addChild(MagnetBtn, 10);
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
	setupStartJoint();
	setupLightJoint();
	setupBridgeSensor();
	setupWinSensor();

	// 先建立 ballSprite 的 Sprite 並加入場景中
	auto PlayerSprite = (Sprite *)ThreeBackground->getChildByName("player");
	PlayerSprite->setScale(0.75f);
	// 設定圖示的位置，稍後必須用程式碼計算跟著動態物體改變位置
	Point player_loc = PlayerSprite->getPosition();
	// 建立一個簡單的動態球體
	b2BodyDef bodyDef;// 先以結構 b2BodyDef 宣告一個 Body 的變數
	bodyDef.type = b2_dynamicBody; // 設定為動態物體
	bodyDef.userData = PlayerSprite;// 設定 Sprite 為動態物體的顯示圖示
	bodyDef.position.Set(player_loc.x / PTM_RATIO, player_loc.y / PTM_RATIO);
	// 以 bodyDef 在 b2World 中建立實體並傳回該實體的指標
	playerBody = _b2World->CreateBody(&bodyDef);
	// 設定該物體的外型
	// 根據 Sprite 圖形的大小來設定圓形的半徑
	b2PolygonShape rectShape;
	//b2CircleShape ballShape;
	Size ts = PlayerSprite->getContentSize();
	//float scale = PlayerSprite->getScale();
	float scaleX = PlayerSprite->getScaleX();	// 讀取矩形畫框有對 X 軸縮放
	float scaleY = PlayerSprite->getScaleY();	// 讀取矩形畫框有對 Y 軸縮放
	float bw = (ts.width - 4)* scaleX;
	float bh = (ts.height - 4)* scaleY;
	// 設定剛體的範圍是一個 BOX （可以縮放成矩形）
	rectShape.SetAsBox(bw*0.5f / PTM_RATIO, bh*0.5f / PTM_RATIO);
	//ballShape.m_radius = 0.5f * (ts.width - 4) *0.5f*scale / PTM_RATIO;
	// 以 b2FixtureDef  結構宣告剛體結構變數，並設定剛體的相關物理係數
	b2FixtureDef fixtureDef;
	//fixtureDef.shape = &ballShape;// 指定剛體的外型為圓形
	fixtureDef.shape = &rectShape;
	fixtureDef.restitution = 0.5f;// 設定彈性係數
	fixtureDef.density = 0.1f;// 設定密度
	fixtureDef.friction = 0.01f;// 設定摩擦係數
	playerBody->CreateFixture(&fixtureDef);// 在 Body 上產生這個剛體的設定

	_contactListener.setCollisionTargetPlayer(*PlayerSprite);

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
	_listener1->onTouchBegan = CC_CALLBACK_2(ThreeScene::onTouchBegan, this);		//加入觸碰開始事件
	_listener1->onTouchMoved = CC_CALLBACK_2(ThreeScene::onTouchMoved, this);		//加入觸碰移動事件
	_listener1->onTouchEnded = CC_CALLBACK_2(ThreeScene::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(ThreeScene::doStep));

	return true;
}
void ThreeScene::doStep(float dt) {
	_fGameTime += dt;
	_fCarTime += dt;
	auto lightSprite = (Sprite *)ThreeBackground->getChildByName("light");
	if (lightSprite->getPositionX() < 1163) {
		_bCarGo = false;
		_fCarTime = 0;
	}
	else { _bCarGo = true; }
	if (_fCarTime >= 3 && _bCarGo && _bCarOpen) {
		_bCarOpen = false;
		setupCar();
	}
	if (NewMagnetBody != NULL)
	{
		if (MagnetBody->GetPosition().y < NewMagnetBody->GetPosition().y)
			MagnetBody->SetTransform(b2Vec2(MagnetBody->GetPosition().x, (MagnetBody->GetPosition().y) + dt * 30), MagnetBody->GetAngle());
		else
			MagnetBody->SetTransform(b2Vec2(MagnetBody->GetPosition().x, (MagnetBody->GetPosition().y) - dt * 30), MagnetBody->GetAngle());
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
			if (body == dynamicwheelBody) {
				b2Vec2 loc = dynamicwheelBody->GetPosition();
				dynamicwheelBody->SetTransform(b2Vec2((loc.x) - dt * 2, loc.y), dynamicwheelBody->GetAngle());
			}
		}
		// 跑出螢幕外面就讓物體從 b2World 中移除
		if (body->GetType() == b2BodyType::b2_dynamicBody && body != playerBody) {
			float x = body->GetPosition().x * PTM_RATIO;
			float y = body->GetPosition().y * PTM_RATIO;
			Sprite* spriteData = (Sprite *)body->GetUserData();
			if (x > visibleSize.width || x < 0 || y >  visibleSize.height || y < 0) {
				if (body->GetUserData() != NULL) {
					Sprite* spriteData = (Sprite *)body->GetUserData();
					spriteData->setVisible(false);
					this->removeChild(spriteData);
				}
				if (body == dynamicwheelBody) { 
					_bCarOpen = true;
					_fCarTime = 0;
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
		else body = body->GetNext(); //否則就繼續更新下一個Body
	}
	if (_contactListener._iBridgeNum >= 3) {
		auto ropeSprite = (Sprite *)ThreeBackground->getChildByName("rope_1");
		ropeSprite->setVisible(false);
	}
	if (_contactListener.win) {
		nextScene();
	}
}
CContactListener_Three::CContactListener_Three()
{
}
void CContactListener_Three::setCollisionTargetPlayer(cocos2d::Sprite &targetSprite) {
	_Playersprite = &targetSprite;
}
void CContactListener_Three::setCollisionTargetCar(cocos2d::Sprite &targetSprite) {
	_carsprite = &targetSprite;
}
void CContactListener_Three::setCollisionTarget(cocos2d::Sprite &targetSprite)
{
	if (_HeadtargetSprite == NULL) {
		_HeadtargetSprite = new AirDiet_Three;
		_HeadtargetSprite->_bAir = true;
		_HeadtargetSprite->_sprite = &targetSprite;
		_HeadtargetSprite->_NexttargetSprite = NULL;
		_NewtargetSprite = _HeadtargetSprite;
	}
	else {
		struct AirDiet_Three * Current = new AirDiet_Three;
		Current->_bAir = true;
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
void CContactListener_Three::BeginContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();
	if (BodyB->GetUserData() != _carsprite&&BodyA->GetUserData() != _carsprite) {
		for (_NewtargetSprite = _HeadtargetSprite; _NewtargetSprite != NULL; _NewtargetSprite = _NewtargetSprite->_NexttargetSprite) {
			if (BodyA->GetUserData() == _NewtargetSprite->_sprite && _NewtargetSprite->_bAir) {
				_NewtargetSprite->_sprite->setVisible(false);
				_NewtargetSprite->_bAir = false;
			}
			else if (BodyB->GetUserData() == _NewtargetSprite->_sprite && _NewtargetSprite->_bAir) {
				_NewtargetSprite->_sprite->setVisible(false);
				_NewtargetSprite->_bAir = false;
			}
		}
	}
	if (BodyA->GetFixtureList()->GetDensity() == 10000.0f) {
		if (BodyB->GetUserData() == _carsprite)
			_iBridgeNum++;
	}
	if (BodyA->GetFixtureList()->GetDensity() == -10000.0f) {
		if (BodyB->GetUserData() == _Playersprite)
			win = true;
	}
}

//碰撞結束
void CContactListener_Three::EndContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();
}
void ThreeScene::setupWinSensor() {
	auto sensorSprite = (Sprite *)ThreeBackground->getChildByName("win_sensor");
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
void ThreeScene::readSceneFile() {
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

	for (size_t i = 1; i <= 5; i++)
	{
		// 產生所需要的 Sprite file name int plist 
		// 此處取得的都是相對於 csbRoot 所在位置的相對座標
		// 在計算 edgeShape 的相對應座標時，必須進行轉換
		sprintf(tmp, "wall_%d", i);
		auto rectSprite = (Sprite *)ThreeBackground->getChildByName(tmp);
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
void ThreeScene::setupStartJoint() {
	b2BodyDef staticbodyDef;
	staticbodyDef.type = b2_staticBody;
	staticbodyDef.userData = NULL;
	b2FixtureDef fixtureDef;
	b2BodyDef dynamicbodyDef;

	//板子
	auto boxSprite = (Sprite *)ThreeBackground->getChildByName("wall_start");
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
	auto ballSprite = (Sprite *)ThreeBackground->getChildByName("circle_start");
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
void ThreeScene::setupLightJoint() {
	b2BodyDef staticbodyDef;
	staticbodyDef.type = b2_staticBody;
	staticbodyDef.userData = NULL;
	b2FixtureDef fixtureDef;
	b2BodyDef dynamicbodyDef;
	//燈
	auto lightSprite = (Sprite *)ThreeBackground->getChildByName("light");
	Size sizelight = lightSprite->getContentSize();
	Point loclight = lightSprite->getPosition();
	float scalelight = lightSprite->getScale();
	staticbodyDef.position.Set(loclight.x / PTM_RATIO, loclight.y / PTM_RATIO);
	b2Body *staticlightBody = _b2World->CreateBody(&staticbodyDef);
	b2CircleShape ballShape;
	ballShape.m_radius = 0.1f*(sizelight.width - 4) *0.5f*scalelight / PTM_RATIO;
	fixtureDef.shape = &ballShape;
	staticlightBody->CreateFixture(&fixtureDef);

	dynamicbodyDef.type = b2_dynamicBody;
	dynamicbodyDef.position.Set(loclight.x / PTM_RATIO, loclight.y / PTM_RATIO);
	dynamicbodyDef.userData = lightSprite;
	b2Body *dynamiclightBody = _b2World->CreateBody(&dynamicbodyDef);
	ballShape.m_radius = (sizelight.width - 4) *0.5f*scalelight / PTM_RATIO;
	fixtureDef.shape = &ballShape;
	fixtureDef.density = 1.0f;// 設定密度
	dynamiclightBody->CreateFixture(&fixtureDef);

	b2PrismaticJointDef PrJoint; // 平移關節
	PrJoint.Initialize(staticlightBody, dynamiclightBody, dynamiclightBody->GetWorldCenter(), b2Vec2(1.0f, 0));
	b2PrismaticJoint* PriJoint;
	PriJoint = (b2PrismaticJoint*)_b2World->CreateJoint(&PrJoint);

	//燈btn
	auto lightbtnSprite = (Sprite *)ThreeBackground->getChildByName("light_btn");
	Size sizelightbtn = lightbtnSprite->getContentSize();
	Point loclightbtn = lightbtnSprite->getPosition();
	float scalelightbtn = lightbtnSprite->getScale();
	staticbodyDef.position.Set(loclightbtn.x / PTM_RATIO, loclightbtn.y / PTM_RATIO);
	b2Body *staticlightbtnBody = _b2World->CreateBody(&staticbodyDef);
	ballShape.m_radius = 0.1f*(sizelightbtn.width - 4) *0.5f*scalelightbtn / PTM_RATIO;
	fixtureDef.shape = &ballShape;
	staticlightbtnBody->CreateFixture(&fixtureDef);

	dynamicbodyDef.type = b2_dynamicBody;
	dynamicbodyDef.position.Set(loclightbtn.x / PTM_RATIO, loclightbtn.y / PTM_RATIO);
	dynamicbodyDef.userData = lightbtnSprite;
	b2Body *dynamiclightbtnBody = _b2World->CreateBody(&dynamicbodyDef);
	ballShape.m_radius = (sizelightbtn.width - 4) *0.5f*scalelightbtn / PTM_RATIO;
	fixtureDef.shape = &ballShape;
	fixtureDef.density = 1.0f;// 設定密度
	dynamiclightbtnBody->CreateFixture(&fixtureDef);

	b2RevoluteJointDef RJoint;
	RJoint.Initialize(staticlightbtnBody, dynamiclightbtnBody, dynamiclightbtnBody->GetWorldCenter());
	b2RevoluteJoint*  RevJoint;
	RevJoint = (b2RevoluteJoint*)_b2World->CreateJoint(&RJoint);

	b2GearJointDef GJoint;
	GJoint.bodyA = dynamiclightBody;
	GJoint.bodyB = dynamiclightbtnBody;
	GJoint.joint1 = PriJoint;
	GJoint.joint2 = RevJoint;
	GJoint.ratio = -1;
	_b2World->CreateJoint(&GJoint);

}
void ThreeScene::setupRopeJoint() {
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.userData = NULL;
	b2PolygonShape boxShape;
	b2FixtureDef fixtureDef;

	//Head
	auto ropeHeadSprite = (Sprite *)ThreeBackground->getChildByName("rope_head");
	Size sizeHead = ropeHeadSprite->getContentSize();
	Point locHead = ropeHeadSprite->getPosition();
	float scaleHeadX = ropeHeadSprite->getScaleX();
	float scaleHeadY = ropeHeadSprite->getScaleY();
	bodyDef.position.Set(locHead.x / PTM_RATIO, locHead.y / PTM_RATIO);
	bodyDef.userData = ropeHeadSprite;
	b2Body *ropeHeadBody = _b2World->CreateBody(&bodyDef);
	boxShape.SetAsBox((sizeHead.width - 4)*scaleHeadX*0.5f / PTM_RATIO, (sizeHead.height - 4)*scaleHeadY*0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	ropeHeadBody->CreateFixture(&fixtureDef);

	//End
	auto ropeEndSprite = (Sprite *)ThreeBackground->getChildByName("rope_end");
	Size sizeEnd = ropeEndSprite->getContentSize();
	Point locEnd = ropeEndSprite->getPosition();
	float scaleEndX = ropeEndSprite->getScaleX();
	float scaleEndY = ropeEndSprite->getScaleY();
	bodyDef.position.Set(locEnd.x / PTM_RATIO, locEnd.y / PTM_RATIO);
	bodyDef.userData = ropeEndSprite;
	b2Body *ropeEndBody = _b2World->CreateBody(&bodyDef);
	ropeEndBody->SetGravityScale(0);
	boxShape.SetAsBox((sizeEnd.width - 4)*scaleEndX*0.5f / PTM_RATIO, (sizeEnd.height - 4)*scaleEndY*0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 1.0f;// 設定密度
	ropeEndBody->CreateFixture(&fixtureDef);

	//產生繩子關節
	b2RopeJointDef JointDef;
	JointDef.bodyA = ropeHeadBody;
	JointDef.bodyB = ropeEndBody;
	JointDef.localAnchorA = b2Vec2(-sizeHead.width*scaleHeadX*0.5f / PTM_RATIO, 0);
	JointDef.localAnchorB = b2Vec2(sizeEnd.width*scaleEndX*0.5f / PTM_RATIO, 0);
	JointDef.maxLength = (locHead.y - locEnd.y) / PTM_RATIO;
	JointDef.collideConnected = true;
	b2RopeJoint* J = (b2RopeJoint*)_b2World->CreateJoint(&JointDef);

	// 中間再以線段相連，
	char tmp[20] = "";
	Sprite *ropeSprite[50];
	Point loc[50];
	Size  size[50];
	b2Body* ropeBody[50];

	bodyDef.type = b2_dynamicBody;
	// 因為是繩子所以重量不要太重
	fixtureDef.density = 1.0f;  fixtureDef.friction = 1.0f; fixtureDef.restitution = 0.0f;
	fixtureDef.shape = &boxShape;
	// 產生一系列的繩子段落 rope01_01 ~ rope01_15，同時接起來
	for (int i = 0; i < 50; i++)
	{
		sprintf(tmp, "rope_%d", i + 1);
		ropeSprite[i] = (Sprite *)ThreeBackground->getChildByName(tmp);
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
	revJoint.bodyA = ropeHeadBody;
	revJoint.localAnchorA.Set(-sizeHead.width*scaleHeadX*0.5f / PTM_RATIO, -sizeHead.height*scaleHeadX*0.5f / PTM_RATIO);
	revJoint.bodyB = ropeBody[0];
	revJoint.localAnchorB.Set(0, locAnchor);
	_b2World->CreateJoint(&revJoint);
	for (int i = 0; i < 49; i++) {
		revJoint.bodyA = ropeBody[i];
		revJoint.localAnchorA.Set(0, -locAnchor);
		revJoint.bodyB = ropeBody[i + 1];
		revJoint.localAnchorB.Set(0, locAnchor);
		_b2World->CreateJoint(&revJoint);
	}
	revJoint.bodyA = ropeBody[49];
	revJoint.localAnchorA.Set(0, -locAnchor);
	revJoint.bodyB = ropeEndBody;
	revJoint.localAnchorB.Set(sizeEnd.width*scaleEndX*0.5f / PTM_RATIO, 0);
	_b2World->CreateJoint(&revJoint);
}
void ThreeScene::setupBridgeSensor() {
	auto sensorSprite = (Sprite *)ThreeBackground->getChildByName("sensor");
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
	SensorFixtureDef.density = 10000; // 故意設定成這個值，方便碰觸時候的判斷
	SensorBody->CreateFixture(&SensorFixtureDef);
}
void ThreeScene::setupCar() {
	b2BodyDef staticbodyDef;
	staticbodyDef.type = b2_staticBody;
	staticbodyDef.userData = NULL;
	b2FixtureDef fixtureDef;
	b2BodyDef dynamicbodyDef;
	//wheel
	b2CircleShape ballShape;
	auto wSprite = (Sprite *)ThreeBackground->getChildByName("wheel_1");
	Size sizewheel = wSprite->getContentSize();
	Point locwheel = wSprite->getPosition();
	float scalewheel = wSprite->getScale();
	auto wheelSprite = Sprite::createWithSpriteFrameName("wheel.png");
	wheelSprite->setScale(scalewheel);
	this->addChild(wheelSprite, 2);
	dynamicbodyDef.type = b2_dynamicBody;
	dynamicbodyDef.position.Set(locwheel.x / PTM_RATIO, locwheel.y / PTM_RATIO);
	dynamicbodyDef.userData = wheelSprite;
	dynamicwheelBody = _b2World->CreateBody(&dynamicbodyDef);
	ballShape.m_radius = (sizewheel.width - 4) *0.5f*scalewheel / PTM_RATIO;
	fixtureDef.shape = &ballShape;
	fixtureDef.restitution = 0.0f;// 設定彈性係數
	fixtureDef.density = 1.0f;// 設定密度
	fixtureDef.friction = 0.01f;// 設定摩擦係數
	dynamicwheelBody->CreateFixture(&fixtureDef);

	wSprite = (Sprite *)ThreeBackground->getChildByName("wheel_2");
	sizewheel = wSprite->getContentSize();
	locwheel = wSprite->getPosition();
	scalewheel = wSprite->getScale();
	auto wheelSprite_2 = Sprite::createWithSpriteFrameName("wheel.png");
	wheelSprite_2->setScale(scalewheel);
	this->addChild(wheelSprite_2, 2);
	dynamicbodyDef.type = b2_dynamicBody;
	dynamicbodyDef.position.Set(locwheel.x / PTM_RATIO, locwheel.y / PTM_RATIO);
	dynamicbodyDef.userData = wheelSprite_2;
	auto wheelBody_2 = _b2World->CreateBody(&dynamicbodyDef);
	ballShape.m_radius = (sizewheel.width - 4) *0.5f*scalewheel / PTM_RATIO;
	fixtureDef.shape = &ballShape;
	fixtureDef.restitution = 0.0f;// 設定彈性係數
	fixtureDef.density = 1.0f;// 設定密度
	fixtureDef.friction = 0.01f;// 設定摩擦係數
	wheelBody_2->CreateFixture(&fixtureDef);
	
	//Car
	wSprite = (Sprite *)ThreeBackground->getChildByName("car");
	Size sizecar = wSprite->getContentSize();
	Point loccar = wSprite->getPosition();
	float scalecarX = wSprite->getScaleX();
	float scalecarY = wSprite->getScaleY();
	auto carSprite = Sprite::createWithSpriteFrameName("car.png");
	carSprite->setScale(scalecarX, scalecarY);
	this->addChild(carSprite, 2);
	dynamicbodyDef.type = b2_dynamicBody;
	dynamicbodyDef.position.Set(loccar.x / PTM_RATIO, loccar.y / PTM_RATIO);
	dynamicbodyDef.userData = carSprite;
	auto carBody = _b2World->CreateBody(&dynamicbodyDef);
	b2PolygonShape boxShape;
	boxShape.SetAsBox(-1*(sizecar.width - 4)*scalecarX*0.5f / PTM_RATIO, (sizecar.height - 4)*scalecarY*0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	fixtureDef.restitution = 0.0f;// 設定彈性係數
	fixtureDef.density = 0.1f;// 設定密度
	fixtureDef.friction = 0.01f;// 設定摩擦係數
	carBody->CreateFixture(&fixtureDef);
	_contactListener.setCollisionTargetCar(*carSprite);

	b2DistanceJointDef JointDef;
	JointDef.Initialize(dynamicwheelBody, carBody, dynamicwheelBody->GetPosition(), carBody->GetPosition());
	_b2World->CreateJoint(&JointDef);

	JointDef.Initialize(wheelBody_2, carBody, wheelBody_2->GetPosition(), carBody->GetPosition());
	_b2World->CreateJoint(&JointDef);

	JointDef.Initialize(wheelBody_2, dynamicwheelBody, wheelBody_2->GetPosition(), dynamicwheelBody->GetPosition());
	_b2World->CreateJoint(&JointDef);

}
void ThreeScene::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
bool ThreeScene::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) {
	Point touchLoc = pTouch->getLocation();
	if (AirBtn->touchesBegin(touchLoc)) {
		_bMagnetOpen = false;
		MagnetBtn->closebtn();
	}
	if (_bAirOpen) {
		HeadAir = new AirDraw_Three;
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
	return true;
}
void ThreeScene::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) {
	Point touchLoc = pTouch->getLocation();
	if (_bAirOpen) {
		if (ccpDistance(NewAir->pos, touchLoc) >= 32) {
			struct AirDraw_Three * Current = new AirDraw_Three;
			Current->pos = touchLoc;
			Current->NextAir = NULL;
			NewAir->NextAir = Current;
			NewAir = Current;
		}
	}
}
void ThreeScene::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) {
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
		nextScene();
	}
}
void ThreeScene::CreateAir() {
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
void ThreeScene::CreateMagnet(Point loc) {
	if (NewMagnetSprite != NULL) {
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
	b2PrismaticJointDef JointDef;
	JointDef.Initialize(NewMagnetBody, MagnetBody, NewMagnetBody->GetPosition(), b2Vec2((NewMagnetBody->GetPosition().x) - (MagnetBody->GetPosition().x), (NewMagnetBody->GetPosition().y) - (MagnetBody->GetPosition().y)));
	_b2World->CreateJoint(&JointDef);
}
void ThreeScene::nextScene() {
	// 先將這個 SCENE 的 Update(這裡使用 OnFrameMove, 從 schedule update 中移出)
	this->unschedule(schedule_selector(ThreeScene::doStep));
	//SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("mainscene.plist");
	// 上面這行何時需要放在這裡稍後會說明
	// 設定場景切換的特效
	TransitionFade *pageTurn = TransitionFade::create(1.0f, FourScene::createScene());
	Director::getInstance()->replaceScene(pageTurn);
	//SimpleAudioEngine::getInstance()->stopBackgroundMusic();
}