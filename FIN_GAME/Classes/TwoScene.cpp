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
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);	//���O��V
	bool AllowSleep = true;					//���\�ε�
	_b2World = new b2World(Gravity);		//�Ыإ@��
	_b2World->SetAllowSleeping(AllowSleep);	//�]�w���󤹳\�ε�

	readSceneFile();

	// ���إ� ballSprite �� Sprite �å[�J������
	PlayerSprite = (Sprite *)TwoBackground->getChildByName("player");
	PlayerSprite->setScale(0.75f);
	// �]�w�ϥܪ���m�A�y�ᥲ���ε{���X�p���۰ʺA������ܦ�m
	Point player_loc = PlayerSprite->getPosition();
	// �إߤ@��²�檺�ʺA�y��
	b2BodyDef bodyDef;// ���H���c b2BodyDef �ŧi�@�� Body ���ܼ�
	bodyDef.type = b2_dynamicBody; // �]�w���ʺA����
	bodyDef.userData = PlayerSprite;// �]�w Sprite ���ʺA���骺��ܹϥ�
	bodyDef.position.Set(player_loc.x / PTM_RATIO, player_loc.y / PTM_RATIO);
	// �H bodyDef �b b2World ���إ߹���öǦ^�ӹ��骺����
	rectBody = _b2World->CreateBody(&bodyDef);
	// �]�w�Ӫ��骺�~��
	// �ھ� Sprite �ϧΪ��j�p�ӳ]�w��Ϊ��b�|
	b2PolygonShape rectShape;
	Size ts = PlayerSprite->getContentSize();
	float scaleX = PlayerSprite->getScaleX();	// Ū���x�εe�ئ��� X �b�Y��
	float scaleY = PlayerSprite->getScaleY();	// Ū���x�εe�ئ��� Y �b�Y��
	float bw = (ts.width - 4)* scaleX;
	float bh = (ts.height - 4)* scaleY;
	// �]�w���骺�d��O�@�� BOX �]�i�H�Y�񦨯x�Ρ^
	rectShape.SetAsBox(bw*0.5f / PTM_RATIO, bh*0.5f / PTM_RATIO);
	// �H b2FixtureDef  ���c�ŧi���鵲�c�ܼơA�ó]�w���骺�������z�Y��
	b2FixtureDef fixtureDef;
	//fixtureDef.shape = &ballShape;// ���w���骺�~�������
	fixtureDef.shape = &rectShape;
	fixtureDef.restitution = 0.5f;// �]�w�u�ʫY��
	fixtureDef.density = 1.0f;// �]�w�K��
	fixtureDef.friction = 0.15f;// �]�w�����Y��
	rectBody->CreateFixture(&fixtureDef);// �b Body �W���ͳo�ӭ��骺�]�w


	if (BOX2D_DEBUG) {
		//DebugDrawInit
		_DebugDraw = nullptr;
		_DebugDraw = new GLESDebugDraw(PTM_RATIO);
		//�]�wDebugDraw
		_b2World->SetDebugDraw(_DebugDraw);
		//���ø�s���O
		uint32 flags = 0;
		flags += GLESDebugDraw::e_shapeBit;						//ø�s�Ϊ�
		flags += GLESDebugDraw::e_pairBit;
		flags += GLESDebugDraw::e_jointBit;
		flags += GLESDebugDraw::e_centerOfMassBit;
		flags += GLESDebugDraw::e_aabbBit;
		//�]�wø�s����
		_DebugDraw->SetFlags(flags);
	}

	_listener1 = EventListenerTouchOneByOne::create();	//�Ыؤ@�Ӥ@��@���ƥ��ť��
	_listener1->onTouchBegan = CC_CALLBACK_2(TwoScene::onTouchBegan, this);		//�[�JĲ�I�}�l�ƥ�
	_listener1->onTouchMoved = CC_CALLBACK_2(TwoScene::onTouchMoved, this);		//�[�JĲ�I���ʨƥ�
	_listener1->onTouchEnded = CC_CALLBACK_2(TwoScene::onTouchEnded, this);		//�[�JĲ�I���}�ƥ�

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//�[�J��Ыت��ƥ��ť��
	this->schedule(CC_SCHEDULE_SELECTOR(TwoScene::doStep));

	return true;
}
void TwoScene::doStep(float dt) {
	_fGameTime += dt;
	int velocityIterations = 8; // �t�׭��N����
								// ��m���N���ơA���N���Ƥ@��]�w��8~10 �V���V�u����Ĳv�V�t
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
		// �]�X�ù��~���N������q b2World ������
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
			else body = body->GetNext(); //�_�h�N�~���s�U�@��Body
		}
		else body = body->GetNext(); //�_�h�N�~���s�U�@��Body
	}
}
void TwoScene::readSceneFile() {
	char tmp[20] = "";

	// ���� EdgeShape �� body
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // �]�w�o�� Body �� �R�A��
	bodyDef.userData = NULL;
	// �b b2World �����͸� Body, �öǦ^���ͪ� b2Body ���󪺫���
	// ���ͤ@���A�N�i�H���᭱�Ҧ��� Shape �ϥ�
	b2Body *body = _b2World->CreateBody(&bodyDef);

	// �����R�A��ɩһݭn�� EdgeShape
	b2EdgeShape edgeShape;
	b2FixtureDef fixtureDef; // ���� Fixture
	fixtureDef.shape = &edgeShape;

	for (size_t i = 1; i <= 8; i++) {
		// ���ͩһݭn�� Sprite file name int plist 
		// ���B���o�����O�۹�� csbRoot �Ҧb��m���۹�y��
		// �b�p�� edgeShape ���۹����y�ЮɡA�����i���ഫ
		sprintf(tmp, "wall_%d", i);
		auto edgeSprite = (Sprite *)TwoBackground->getChildByName(tmp);
		Size ts = edgeSprite->getContentSize();
		Point loc = edgeSprite->getPosition();
		float angle = edgeSprite->getRotation();
		float scale = edgeSprite->getScaleX();	// �������u�q�ϥܰ��]���u���� X �b��j

		Point lep1, lep2, wep1, wep2; // EdgeShape ����Ӻ��I
		lep1.y = 0; lep1.x = -(ts.width - 4) / 2.0f;
		lep2.y = 0; lep2.x = (ts.width - 4) / 2.0f;

		// �Ҧ����u�q�ϥܳ��O�O�����������I�� (0,0)�A
		// �ھ��Y��B���ಣ�ͩһݭn���x�}
		// �ھڼe�׭p��X��Ӻ��I���y�СA�M��e�W�}�x�}
		// �M��i�����A
		// Step1: ��CHECK ���L����A������h�i����I���p��
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scale;  // ���]�w X �b���Y��
		cocos2d::Mat4::createRotationZ(angle*M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = PntLoc.x + loc.x; //�]�w Translation�A�ۤv���[�W���˪�
		modelMatrix.m[7] = PntLoc.y + loc.y; //�]�w Translation�A�ۤv���[�W���˪�

											 // ���ͨ�Ӻ��I
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