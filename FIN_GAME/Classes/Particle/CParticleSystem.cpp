#include "CParticleSystem.h"

#define NUMBER_PARTICLES 1000 // 預設一次取得 1000 個 Particles

USING_NS_CC;

CParticleSystem::CParticleSystem()
{
	_fGravity = 0;
	_bEmitterOn = false;
	_fSize = 0.125f;
}

void CParticleSystem::setEmitter(bool bEm)
{
	_bEmitterOn = bEm;
}

void CParticleSystem::init(cocos2d::Layer &inlayer)
{
	_iFree = NUMBER_PARTICLES;
	_iInUsed = 0;
	_pParticles = new CParticle[NUMBER_PARTICLES]; // 取得所需要的 particle 空間
	 // 讀入儲存多張圖片的 plist 檔
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("particletexture.plist");
	for (int i = 0; i < NUMBER_PARTICLES; i++) {
		_pParticles[i].setParticle(_cSprite, inlayer);
		_FreeList.push_front(&_pParticles[i]);
	}
}

void CParticleSystem::doStep(float dt)
{
	CParticle *get;
	list <CParticle *>::iterator it;	
	if (_bEmitterOn) { // 根據 Emitter 設定的相關參數，產生相對應的分子
		// 先計算在累加
		int n = (int)(_fElpasedTime * _iNumParticles); // 到目前為止應該產生的分子個數
		if (n > _iGenParticles) {  // 產生的分子個數不足，產生到 n 個分子
			for (int i = 0; i < n - _iGenParticles; i++) {
				// 根據 Emitter 的相關參數，設定所產生分子的參數
				if (_iFree != 0) {
					get = _FreeList.front();
					get->setBehavior(EMITTER_DEFAULT);
					get->setVelocity(_fVelocity);
					get->setLifetime(_fLifeTime);
					get->setGravity(_fGravity);
					get->setSpin(_fSpin);
					get->setOpacity(_iOpacity);
					get->setPosition(_emitterPt);
					//get->setSize(0.125f);
					get->setSize(_fSize);
					get->setColor(Color3B(_iRed, _iGreen, _iBlue));
					get->setSprite(_cSprite);
					get->setWind(_fWind, _fWindDirection);

					if (_bFlowerOn) {
						//get->setOpacity(255 - sinf(_fTime / (180 / M_PI)) * 100);
					}

					// 根據 _fSpread 與 _vDir 產生方向
					float t = (rand() % 1001) / 1000.0f; // 產生介於 0 到 1 間的數
					t = _fSpread - t * _fSpread * 2; //  產生的角度，轉成弧度
					t = ( _fDir + t )* M_PI / 180.0f;
					Vec2 vdir(cosf(t), sinf(t));
					get->setDirection(vdir);
					_FreeList.pop_front();
					_InUsedList.push_front(get);
					_iFree--; _iInUsed++;
				}
			}
			_iGenParticles = n; // 目前已經產生 n 個分子
			
		}
		if (_fElpasedTime >= 1.0f) {
			_fElpasedTime -= 1.0f;
			if (_iGenParticles >= _iNumParticles) _iGenParticles -= _iNumParticles;
			else _iGenParticles = 0;
		}
		_fElpasedTime += dt;
	}

	if (_iInUsed != 0) { // 有分子需要更新時
		for (it = _InUsedList.begin(); it != _InUsedList.end(); ) {
			if ((*it)->doStep(dt)) { // 分子生命週期已經到達
									 // 將目前這一個節點的內容放回 _FreeList
				_FreeList.push_front((*it));
				it = _InUsedList.erase(it); // 移除目前這一個, 
				_iFree++; _iInUsed--;
			}
			else it++;
		}
	}
	if (_bType3) {
		onTouchesBegan(_TypePos); _fTypeTime += 600 * dt;
	}
}

void CParticleSystem::setGravity(float fGravity)
{
	// 設定目前 particle 是在 inused 的 gravity
	_fGravity = fGravity;
	list <CParticle *>::iterator it;
	if (_iInUsed != 0) { // 有分子需要更新時
		for (it = _InUsedList.begin(); it != _InUsedList.end(); it++) {
			(*it)->setGravity(_fGravity);
		}
	}
}

void CParticleSystem::setSprite(char * pngName) {
	list <CParticle *>::iterator it;
	if (_iInUsed != 0) { // 有分子需要更新時
		for (it = _InUsedList.begin(); it != _InUsedList.end(); it++) {
			(*it)->setSprite(pngName);
		}
	}
}


void CParticleSystem::setWindDirection(float fWindDir) {
	_fWindDirection = fWindDir;
	list <CParticle *>::iterator it;
	if (_iInUsed != 0) { // 有分子需要更新時
		for (it = _InUsedList.begin(); it != _InUsedList.end(); it++) {
			(*it)->setWind(_fWind, _fWindDirection);
		}
	}
}

void CParticleSystem::setWind(float fWind) {
	_fWind = fWind;
	list <CParticle *>::iterator it;
	if (_iInUsed != 0) { // 有分子需要更新時
		for (it = _InUsedList.begin(); it != _InUsedList.end(); it++) {
			(*it)->setWind(_fWind, _fWindDirection);
		}
	}
}

void CParticleSystem::setFlower(float alltime, float time, cocos2d::Point loc, float speed) {
	_fTime = time;
	list <CParticle *>::iterator it;
	if (_iInUsed != 0) { // 有分子需要更新時
		for (it = _InUsedList.begin(); it != _InUsedList.end(); it++) {
			auto dx_0 = (loc.x - (*it)->_Pos.x);
			auto dy_0 = (loc.y - (*it)->_Pos.y);
			float dx, dy;
			if (dx_0 < 0)dx = dx_0 * -1; else dx = dx_0;
			if (dy_0 < 0)dy = dy_0 * -1; else dy = dy_0;
			_fAngle = atan(dy_0 / dx_0) * (180 / M_PI);
			if ((dx_0 >= 0) && (dy_0 >= 0))
				_fAngle = _fAngle - 180;
			else if ((dx_0 < 0) && (dy_0 >= 0))
				_fAngle = _fAngle;
			else if ((dx_0 < 0) && (dy_0 < 0))
				_fAngle = _fAngle - 360;
			else if ((dx_0 >= 0) && (dy_0 < 0))
				_fAngle = 180 + _fAngle;
			if (alltime >= (dx / speed)) {
				(*it)->setWind(1 * sinf((time / 2 + 90) / (180 / M_PI)), _fAngle);
				_fSpread = 2 * sinf(time / (180 / M_PI));
			}
		}
	}
}

CParticleSystem::~CParticleSystem()
{
	if (_iInUsed != 0) _InUsedList.clear(); // 移除所有的 NODE
	if (_iFree != 0) _FreeList.clear();
	delete[] _pParticles; // 釋放所有取得資源
	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("particletexture.plist");
}

void CParticleSystem::onTouchesBegan(const cocos2d::CCPoint &touchPoint)
{
	CParticle *get;
	switch (_iType)
	{
	case STAY_FOR_TWOSECONDS:
		// 從 _FreeList 取得一個分子給放到 _InUsed
		if (_iFree != 0) {
			get = _FreeList.front();
			get->setSprite(_cSprite);
			get->setBehavior(STAY_FOR_TWOSECONDS);
			get->setPosition(touchPoint);
			get->setGravity(_fGravity);
			_FreeList.pop_front();
			_InUsedList.push_front(get);
			_iFree--; _iInUsed++;
		}
		else return;// 沒有分子, 所以就不提供
		break;
	case RANDOMS_FALLING :
		if (_iFree != 0) {
			get = _FreeList.front();
			get->setSprite(_cSprite);
			get->setBehavior(RANDOMS_FALLING);
			get->setPosition(touchPoint);
			get->setGravity(_fGravity);
			_FreeList.pop_front();
			_InUsedList.push_front(get);
			_iFree--; _iInUsed++;
		}
		else return;// 沒有分子, 所以就不提供
		break;
	}
}

void CParticleSystem::onTouchesMoved(const cocos2d::CCPoint &touchPoint)
{
	CParticle *get;
	switch (_iType)
	{
	case STAY_FOR_TWOSECONDS:
		// 從 _FreeList 取得一個分子給放到 _InUsed
		if (_iFree != 0) {
			get = _FreeList.front();
			get->setBehavior(STAY_FOR_TWOSECONDS);
			get->setPosition(touchPoint);
			get->setGravity(_fGravity);
			_FreeList.pop_front();
			_InUsedList.push_front(get);
			_iFree--; _iInUsed++;
		}
		else return;// 沒有分子, 所以就不提供
		break;
	case RANDOMS_FALLING:
		if (_iFree != 0) {
			get = _FreeList.front();
			get->setBehavior(RANDOMS_FALLING);
			get->setPosition(touchPoint);
			get->setGravity(_fGravity);
			_FreeList.pop_front();
			_InUsedList.push_front(get);
			_iFree--; _iInUsed++;
		}
		else return;// 沒有分子, 所以就不提供
		break;
	}
}
void CParticleSystem::onTouchesEnded(const cocos2d::CCPoint &touchPoint)
{
	CParticle *get;
	switch (_iType)
	{
	}
}