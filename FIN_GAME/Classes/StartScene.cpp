#include "StartScene.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "OneScene.h"
#include "TwoScene.h"
#include "ThreeScene.h"
#include "FourScene.h"

USING_NS_CC;

using namespace cocostudio::timeline;

Scene* StartScene::createScene()
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = StartScene::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}
StartScene::~StartScene()
{

}

bool StartScene::init()
{
	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto origin = Director::getInstance()->getVisibleOrigin();

	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}

	auto rootNode = CSLoader::createNode("StartScene.csb");
	addChild(rootNode);

	OnePage = CButton::create();
	OnePage->setButtonInfo("dnarrow.png", "dnarrowon.png", "dnarrow.png", Point(visibleSize.width / 2.0f, 50.0f), true);
	OnePage->setScale(0.75f);
	this->addChild(OnePage, 10);

	_listener1 = EventListenerTouchOneByOne::create();	//創建一個一對一的事件聆聽器
	_listener1->onTouchBegan = CC_CALLBACK_2(StartScene::onTouchBegan, this);		//加入觸碰開始事件
	_listener1->onTouchMoved = CC_CALLBACK_2(StartScene::onTouchMoved, this);		//加入觸碰移動事件
	_listener1->onTouchEnded = CC_CALLBACK_2(StartScene::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(StartScene::doStep));

	return true;
}
void StartScene::doStep(float dt) {
}
bool StartScene::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent){
	Point touchLoc = pTouch->getLocation();
	if (OnePage->touchesBegin(touchLoc)) {
		auto scene = OneScene::createScene();
		//auto scene = TwoScene::createScene();
		//auto scene = ThreeScene::createScene();
		//auto scene = FourScene::createScene();
		Director::sharedDirector()->replaceScene(TransitionFade::create(0.5f, scene));
	}
	return true;
}
void StartScene::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent){
	Point touchLoc = pTouch->getLocation();
	if (OnePage->touchesMoved(touchLoc)) {
	}
}
void StartScene::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent){
	Point touchLoc = pTouch->getLocation();
	if (OnePage->touchesEnded(touchLoc)) {
	}
}