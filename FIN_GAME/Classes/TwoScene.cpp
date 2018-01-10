#include "TwoScene.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

#define PTM_RATIO 32.0f

USING_NS_CC;

using namespace cocostudio::timeline;

Scene* TwoScene::createScene()
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = TwoScene::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}
TwoScene::~TwoScene()
{
	if (_b2World != nullptr) delete _b2World;
}

bool TwoScene::init()
{
	visibleSize = Director::getInstance()->getVisibleSize();
	origin = Director::getInstance()->getVisibleOrigin();

	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}

	TwoBackground = CSLoader::createNode("Two.csb");
	addChild(TwoBackground);
	PntLoc = TwoBackground->getPosition();

	//B2World
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);	//重力方向
	bool AllowSleep = true;					//允許睡著
	_b2World = new b2World(Gravity);		//創建世界
	_b2World->SetAllowSleeping(AllowSleep);	//設定物件允許睡著

	readSceneFile();

	// 先建立 ballSprite 的 Sprite 並加入場景中
	PlayerSprite = (Sprite *)TwoBackground->getChildByName("player");
	PlayerSprite->setScale(0.75f);
	// 設定圖示的位置，稍後必須用程式碼計算跟著動態物體改變位置
	Point player_loc = PlayerSprite->getPosition();
	// 建立一個簡單的動態球體
	b2BodyDef bodyDef;// 先以結構 b2BodyDef 宣告一個 Body 的變數
	bodyDef.type = b2_dynamicBody; // 設定為動態物體
	bodyDef.userData = PlayerSprite;// 設定 Sprite 為動態物體的顯示圖示
	bodyDef.position.Set(player_loc.x / PTM_RATIO, player_loc.y / PTM_RATIO);
	// 以 bodyDef 在 b2World 中建立實體並傳回該實體的指標
	rectBody = _b2World->CreateBody(&bodyDef);
	// 設定該物體的外型
	// 根據 Sprite 圖形的大小來設定圓形的半徑
	b2PolygonShape rectShape;
	Size ts = PlayerSprite->getContentSize();
	float scaleX = PlayerSprite->getScaleX();	// 讀取矩形畫框有對 X 軸縮放
	float scaleY = PlayerSprite->getScaleY();	// 讀取矩形畫框有對 Y 軸縮放
	float bw = (ts.width - 4)* scaleX;
	float bh = (ts.height - 4)* scaleY;
	// 設定剛體的範圍是一個 BOX （可以縮放成矩形）
	rectShape.SetAsBox(bw*0.5f / PTM_RATIO, bh*0.5f / PTM_RATIO);
	// 以 b2FixtureDef  結構宣告剛體結構變數，並設定剛體的相關物理係數
	b2FixtureDef fixtureDef;
	//fixtureDef.shape = &ballShape;// 指定剛體的外型為圓形
	fixtureDef.shape = &rectShape;
	fixtureDef.restitution = 0.5f;// 設定彈性係數
	fixtureDef.density = 1.0f;// 設定密度
	fixtureDef.friction = 0.15f;// 設定摩擦係數
	rectBody->CreateFixture(&fixtureDef);// 在 Body 上產生這個剛體的設定


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
	_listener1->onTouchBegan = CC_CALLBACK_2(TwoScene::onTouchBegan, this);		//加入觸碰開始事件
	_listener1->onTouchMoved = CC_CALLBACK_2(TwoScene::onTouchMoved, this);		//加入觸碰移動事件
	_listener1->onTouchEnded = CC_CALLBACK_2(TwoScene::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(TwoScene::doStep));

	return true;
}
void TwoScene::doStep(float dt) {
	_fGameTime += dt;
	int velocityIterations = 8; // 速度迭代次數
								// 位置迭代次數，迭代次數一般設定為8~10 越高越真實但效率越差
	int positionIterations = 1;
	_b2World->Step(dt, velocityIterations, positionIterations);
	for (b2Body* body = _b2World->GetBodyList(); body; )
	{
		if (body->GetUserData() != NULL) {
			Sprite *ballData = (Sprite*)body->GetUserData();
			ballData->setPosition(body->GetPosition().x*PTM_RATIO, body->GetPosition().y*PTM_RATIO);
			ballData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
			if (_fGameTime >= 3 && !_bPlayerGo) {
				rectBody->ApplyLinearImpulse(b2Vec2(20, 0), rectBody->GetWorldCenter(), true);
				_bPlayerGo = true;
			}
		}
		// 跑出螢幕外面就讓物體從 b2World 中移除
		if (body->GetType() == b2BodyType::b2_dynamicBody && body != rectBody) {
			float x = body->GetPosition().x * PTM_RATIO;
			float y = body->GetPosition().y * PTM_RATIO;

			if (x > visibleSize.width || x < 0 || y >  visibleSize.height || y < 0) {
				if (body->GetUserData() != NULL) {
					Sprite* spriteData = (Sprite *)body->GetUserData();
					this->removeChild(spriteData);
				}
				_b2World->DestroyBody(body);
				body = NULL;
			}
			else body = body->GetNext(); //否則就繼續更新下一個Body
		}
		else body = body->GetNext(); //否則就繼續更新下一個Body
	}
}
void TwoScene::readSceneFile() {
	char tmp[20] = "";

	// 產生 EdgeShape 的 body
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // 設定這個 Body 為 靜態的
	bodyDef.userData = NULL;
	// 在 b2World 中產生該 Body, 並傳回產生的 b2Body 物件的指標
	// 產生一次，就可以讓後面所有的 Shape 使用
	b2Body *body = _b2World->CreateBody(&bodyDef);

	// 產生靜態邊界所需要的 EdgeShape
	b2EdgeShape edgeShape;
	b2FixtureDef fixtureDef; // 產生 Fixture
	fixtureDef.shape = &edgeShape;

	for (size_t i = 1; i <= 8; i++) {
		// 產生所需要的 Sprite file name int plist 
		// 此處取得的都是相對於 csbRoot 所在位置的相對座標
		// 在計算 edgeShape 的相對應座標時，必須進行轉換
		sprintf(tmp, "wall_%d", i);
		auto edgeSprite = (Sprite *)TwoBackground->getChildByName(tmp);
		Size ts = edgeSprite->getContentSize();
		Point loc = edgeSprite->getPosition();
		float angle = edgeSprite->getRotation();
		float scale = edgeSprite->getScaleX();	// 水平的線段圖示假設都只有對 X 軸放大

		Point lep1, lep2, wep1, wep2; // EdgeShape 的兩個端點
		lep1.y = 0; lep1.x = -(ts.width - 4) / 2.0f;
		lep2.y = 0; lep2.x = (ts.width - 4) / 2.0f;

		// 所有的線段圖示都是是本身的中心點為 (0,0)，
		// 根據縮放、旋轉產生所需要的矩陣
		// 根據寬度計算出兩個端點的座標，然後呈上開矩陣
		// 然後進行旋轉，
		// Step1: 先CHECK 有無旋轉，有旋轉則進行端點的計算
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scale;  // 先設定 X 軸的縮放
		cocos2d::Mat4::createRotationZ(angle*M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = PntLoc.x + loc.x; //設定 Translation，自己的加上父親的
		modelMatrix.m[7] = PntLoc.y + loc.y; //設定 Translation，自己的加上父親的

											 // 產生兩個端點
		wep1.x = lep1.x * modelMatrix.m[0] + lep1.y * modelMatrix.m[1] + modelMatrix.m[3];
		wep1.y = lep1.x * modelMatrix.m[4] + lep1.y * modelMatrix.m[5] + modelMatrix.m[7];
		wep2.x = lep2.x * modelMatrix.m[0] + lep2.y * modelMatrix.m[1] + modelMatrix.m[3];
		wep2.y = lep2.x * modelMatrix.m[4] + lep2.y * modelMatrix.m[5] + modelMatrix.m[7];

		// bottom edge
		edgeShape.Set(b2Vec2(wep1.x / PTM_RATIO, wep1.y / PTM_RATIO), b2Vec2(wep2.x / PTM_RATIO, wep2.y / PTM_RATIO));
		body->CreateFixture(&fixtureDef);
	}
}
void TwoScene::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
bool TwoScene::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) {
	Point touchLoc = pTouch->getLocation();
	BeginLoc = touchLoc;
	/*_ParticleControl._emitterPt = touchLoc;
	_ParticleControl.setEmitter(true);*/
	_bAirOpen = true;
	return true;
}
void TwoScene::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) {
	Point touchLoc = pTouch->getLocation();
	if (_bAirOpen) {
	}
}
void TwoScene::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) {
	Point touchLoc = pTouch->getLocation();
	if (_bAirOpen) {
		CreateAir(BeginLoc, touchLoc);
		_bAirOpen = false;
	}
}
void TwoScene::CreateAir(cocos2d::Point Bpos, cocos2d::Point Epos) {
	float dx = Epos.x - Bpos.x;
	float dy = Epos.y - Bpos.y;
	if (abs(dx) >= abs(dy)) {
		if (dx >= 0) {
			for (dx; dx >= 0; ) {
				auto AirSprite = Sprite::createWithSpriteFrameName("dount01.png");
				//auto AirSprite = Sprite::createWithSpriteFrameName("cloud.png");
				AirSprite->setScale(0.5f);
				this->addChild(AirSprite, 2);
				b2BodyDef bodyDef;
				bodyDef.type = b2_dynamicBody;
				bodyDef.userData = AirSprite;
				bodyDef.position.Set((Bpos.x + dx) / PTM_RATIO, (Bpos.y + dy) / PTM_RATIO);
				b2Body *AirBody = _b2World->CreateBody(&bodyDef);
				AirBody->SetGravityScale(-0.5f);
				b2CircleShape ballShape;
				Size ballsize = AirSprite->getContentSize();
				ballShape.m_radius = 0.5f * (ballsize.width - 4) *0.5f / PTM_RATIO;
				b2FixtureDef fixtureDef;
				fixtureDef.shape = &ballShape;
				fixtureDef.restitution = 0.75f;
				fixtureDef.density = 5.0f;
				fixtureDef.friction = 0.15f;
				AirBody->CreateFixture(&fixtureDef);
				dy = dy - 0.5f *(ballsize.width - 4)*(dy / dx);
				dx = dx - 0.5f *(ballsize.width - 4);
			}
		}
		else {
			for (dx; dx < 0; ) {
				auto AirSprite = Sprite::createWithSpriteFrameName("dount01.png");
				AirSprite->setScale(0.5f);
				this->addChild(AirSprite, 2);
				b2BodyDef bodyDef;
				bodyDef.type = b2_dynamicBody;
				bodyDef.userData = AirSprite;
				bodyDef.position.Set((Bpos.x + dx) / PTM_RATIO, (Bpos.y + dy) / PTM_RATIO);
				b2Body *AirBody = _b2World->CreateBody(&bodyDef);
				AirBody->SetGravityScale(-0.5f);
				b2CircleShape ballShape;
				Size ballsize = AirSprite->getContentSize();
				ballShape.m_radius = 0.5f * (ballsize.width - 4) *0.5f / PTM_RATIO;
				b2FixtureDef fixtureDef;
				fixtureDef.shape = &ballShape;
				fixtureDef.restitution = 0.75f;
				fixtureDef.density = 5.0f;
				fixtureDef.friction = 0.15f;
				AirBody->CreateFixture(&fixtureDef);
				dy = dy + 0.5f *(ballsize.width - 4)*(dy / dx);
				dx = dx + 0.5f *(ballsize.width - 4);
			}
		}
	}
	else {
		if (dy >= 0) {
			for (dy; dy >= 0; ) {
				auto AirSprite = Sprite::createWithSpriteFrameName("dount01.png");
				AirSprite->setScale(0.5f);
				this->addChild(AirSprite, 2);
				b2BodyDef bodyDef;
				bodyDef.type = b2_dynamicBody;
				bodyDef.userData = AirSprite;
				bodyDef.position.Set((Bpos.x + dx) / PTM_RATIO, (Bpos.y + dy) / PTM_RATIO);
				b2Body *AirBody = _b2World->CreateBody(&bodyDef);
				AirBody->SetGravityScale(-0.5f);
				b2CircleShape ballShape;
				Size ballsize = AirSprite->getContentSize();
				ballShape.m_radius = 0.5f * (ballsize.width - 4) *0.5f / PTM_RATIO;
				b2FixtureDef fixtureDef;
				fixtureDef.shape = &ballShape;
				fixtureDef.restitution = 0.75f;
				fixtureDef.density = 5.0f;
				fixtureDef.friction = 0.15f;
				AirBody->CreateFixture(&fixtureDef);
				dx = dx - 0.5f *(ballsize.width - 4)*(dx / dy);
				dy = dy - 0.5f *(ballsize.width - 4);
			}
		}
		else {
			for (dy; dy < 0; ) {
				auto AirSprite = Sprite::createWithSpriteFrameName("dount01.png");
				AirSprite->setScale(0.5f);
				this->addChild(AirSprite, 2);
				b2BodyDef bodyDef;
				bodyDef.type = b2_dynamicBody;
				bodyDef.userData = AirSprite;
				bodyDef.position.Set((Bpos.x + dx) / PTM_RATIO, (Bpos.y + dy) / PTM_RATIO);
				b2Body *AirBody = _b2World->CreateBody(&bodyDef);
				AirBody->SetGravityScale(-0.5f);
				b2CircleShape ballShape;
				Size ballsize = AirSprite->getContentSize();
				ballShape.m_radius = 0.5f * (ballsize.width - 4) *0.5f / PTM_RATIO;
				b2FixtureDef fixtureDef;
				fixtureDef.shape = &ballShape;
				fixtureDef.restitution = 0.75f;
				fixtureDef.density = 5.0f;
				fixtureDef.friction = 0.15f;
				AirBody->CreateFixture(&fixtureDef);
				dx = dx + 0.5f *(ballsize.width - 4)*(dx / dy);
				dy = dy + 0.5f *(ballsize.width - 4);
			}
		}
	}
}