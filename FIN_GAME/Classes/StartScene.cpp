#include "StartScene.h"
#include "cocostudio/CocoStudio.h"
#include "SimpleAudioEngine.h"
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
	this->removeAllChildren();
	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("mainscene.plist");
	Director::getInstance()->getTextureCache()->removeUnusedTextures();

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

	//Music
	auto bkmusic = (cocostudio::ComAudio *)rootNode->getChildByName("music_bg")->getComponent("music_bg");
	bkmusic->playBackgroundMusic();

	startbtn = (Sprite *)rootNode->getChildByName("main_right");
	auto _BtnSize = startbtn->getContentSize();
	auto _BtnLoc = startbtn->getPosition();
	_BtnRect.size = _BtnSize;
	_BtnRect.origin.x = _BtnLoc.x - _BtnSize.width*0.5f;
	_BtnRect.origin.y = _BtnLoc.y - _BtnSize.height*0.5f;

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
	if (_BtnRect.containsPoint(touchLoc)) {
		this->unschedule(schedule_selector(StartScene::doStep));
		removeAllChildren();
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("mainscene.plist");
		auto scene = OneScene::createScene(0);
		Director::sharedDirector()->replaceScene(TransitionFade::create(0.5f, scene));
		//SimpleAudioEngine::getInstance()->stopBackgroundMusic();
	}
	return true;
}
void StartScene::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent){
	Point touchLoc = pTouch->getLocation();
}
void StartScene::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent){
	Point touchLoc = pTouch->getLocation();
}