#include "StopScene.h"
#include "OneScene.h"
#include "TwoScene.h"
#include "ThreeScene.h"
#include "FourScene.h"
#include "StartScene.h"

#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

using namespace cocostudio::timeline;
using namespace ui;

StopScene::~StopScene() {
	removeAllChildren();
}
Scene* StopScene::createScene(RenderTexture * sc, const int score, const int number)
{
	auto scene = Scene::create();
	auto layer = StopScene::create();
	layer->score = score;
	layer->number = number;
	Size visibleSize = Director::getInstance()->getVisibleSize();
	Sprite * bg = Sprite::createWithTexture(sc->getSprite()->getTexture());
	bg->setPosition(Point(visibleSize.width / 2, visibleSize.height / 2));
	bg->setFlipY(true);
	bg->setColor(Color3B(115, 115, 115));
	layer->addChild(bg);
	scene->addChild(layer);
	return scene;
}
bool StopScene::init()
{
	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	if (!Layer::init())
	{
		return false;
	}

	PlayBtn = CButton::create();
	PlayBtn->setButtonInfo("new_play.png", "new_play_on.png", "new_play.png", Point(445, 360), true);
	PlayBtn->setScale(1.5f);
	this->addChild(PlayBtn, 10);
	ReplayBtn = CButton::create();
	ReplayBtn->setButtonInfo("new_replay.png", "new_replay_on.png", "new_replay.png", Point(640, 360), true);
	ReplayBtn->setScale(1.5f);
	this->addChild(ReplayBtn, 10);
	HomeBtn = CButton::create();
	HomeBtn->setButtonInfo("new_home.png", "new_home_on.png", "new_home.png", Point(835, 360), true);
	HomeBtn->setScale(1.5f);
	this->addChild(HomeBtn, 10);

	//Touch
	_listener1 = EventListenerTouchOneByOne::create();
	_listener1->onTouchBegan = CC_CALLBACK_2(StopScene::onTouchBegan, this);
	_listener1->onTouchMoved = CC_CALLBACK_2(StopScene::onTouchMoved, this);
	_listener1->onTouchEnded = CC_CALLBACK_2(StopScene::onTouchEnded, this);

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//加入剛創建的事件聆聽器

	return true;
}
bool StopScene::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)//觸碰開始事件
{
	Point touchLoc = pTouch->getLocation();
	if (PlayBtn->touchesBegin(touchLoc)) {}
	if (ReplayBtn->touchesBegin(touchLoc)) {}
	if (HomeBtn->touchesBegin(touchLoc)) {}
	return true;
}
void StopScene::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //觸碰移動事件
{
	Point touchLoc = pTouch->getLocation();
}
void StopScene::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //觸碰結束事件 
{
	Point touchLoc = pTouch->getLocation();
	if (PlayBtn->touchesEnded(touchLoc)) {
		Director::sharedDirector()->popScene();
	}
	if (ReplayBtn->touchesEnded(touchLoc)) {
		TransitionFade *pageTurn;
		if (number == 1)
			pageTurn = TransitionFade::create(1.0f, OneScene::createScene(score + 1));
		else if (number == 2)
			pageTurn = TransitionFade::create(1.0f, TwoScene::createScene(score + 1));
		else if (number == 3)
			pageTurn = TransitionFade::create(1.0f, ThreeScene::createScene(score + 1));
		else if (number == 4)
			pageTurn = TransitionFade::create(1.0f, FourScene::createScene(score + 1));
		Director::getInstance()->replaceScene(pageTurn);
	}
	if (HomeBtn->touchesEnded(touchLoc)) {
		TransitionFade *pageTurn = TransitionFade::create(1.0f, StartScene::createScene());
		Director::getInstance()->replaceScene(pageTurn);
	}
}