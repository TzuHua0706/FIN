#include "CParticleSystem.h"

#define NUMBER_PARTICLES 1000 // �w�]�@�����o 1000 �� Particles

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
	_pParticles = new CParticle[NUMBER_PARTICLES]; // ���o�һݭn�� particle �Ŷ�
	 // Ū�J�x�s�h�i�Ϥ��� plist ��
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
	if (_bEmitterOn) { // �ھ� Emitter �]�w�������ѼơA���ͬ۹��������l
		// ���p��b�֥[
		int n = (int)(_fElpasedTime * _iNumParticles); // ��ثe�������Ӳ��ͪ����l�Ӽ�
		if (n > _iGenParticles) {  // ���ͪ����l�ӼƤ����A���ͨ� n �Ӥ��l
			for (int i = 0; i < n - _iGenParticles; i++) {
				// �ھ� Emitter �������ѼơA�]�w�Ҳ��ͤ��l���Ѽ�
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

					// �ھ� _fSpread �P _vDir ���ͤ�V
					float t = (rand() % 1001) / 1000.0f; // ���ͤ��� 0 �� 1 ������
					t = _fSpread - t * _fSpread * 2; //  ���ͪ����סA�ন����
					t = ( _fDir + t )* M_PI / 180.0f;
					Vec2 vdir(cosf(t), sinf(t));
					get->setDirection(vdir);
					_FreeList.pop_front();
					_InUsedList.push_front(get);
					_iFree--; _iInUsed++;
				}
			}
			_iGenParticles = n; // �ثe�w�g���� n �Ӥ��l
			
		}
		if (_fElpasedTime >= 1.0f) {
			_fElpasedTime -= 1.0f;
			if (_iGenParticles >= _iNumParticles) _iGenParticles -= _iNumParticles;
			else _iGenParticles = 0;
		}
		_fElpasedTime += dt;
	}

	if (_iInUsed != 0) { // �����l�ݭn��s��
		for (it = _InUsedList.begin(); it != _InUsedList.end(); ) {
			if ((*it)->doStep(dt)) { // ���l�ͩR�g���w�g��F
									 // �N�ثe�o�@�Ӹ`�I�����e��^ _FreeList
				_FreeList.push_front((*it));
				it = _InUsedList.erase(it); // �����ثe�o�@��, 
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
	// �]�w�ثe particle �O�b inused �� gravity
	_fGravity = fGravity;
	list <CParticle *>::iterator it;
	if (_iInUsed != 0) { // �����l�ݭn��s��
		for (it = _InUsedList.begin(); it != _InUsedList.end(); it++) {
			(*it)->setGravity(_fGravity);
		}
	}
}

void CParticleSystem::setSprite(char * pngName) {
	list <CParticle *>::iterator it;
	if (_iInUsed != 0) { // �����l�ݭn��s��
		for (it = _InUsedList.begin(); it != _InUsedList.end(); it++) {
			(*it)->setSprite(pngName);
		}
	}
}


void CParticleSystem::setWindDirection(float fWindDir) {
	_fWindDirection = fWindDir;
	list <CParticle *>::iterator it;
	if (_iInUsed != 0) { // �����l�ݭn��s��
		for (it = _InUsedList.begin(); it != _InUsedList.end(); it++) {
			(*it)->setWind(_fWind, _fWindDirection);
		}
	}
}

void CParticleSystem::setWind(float fWind) {
	_fWind = fWind;
	list <CParticle *>::iterator it;
	if (_iInUsed != 0) { // �����l�ݭn��s��
		for (it = _InUsedList.begin(); it != _InUsedList.end(); it++) {
			(*it)->setWind(_fWind, _fWindDirection);
		}
	}
}

void CParticleSystem::setFlower(float alltime, float time, cocos2d::Point loc, float speed) {
	_fTime = time;
	list <CParticle *>::iterator it;
	if (_iInUsed != 0) { // �����l�ݭn��s��
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
	if (_iInUsed != 0) _InUsedList.clear(); // �����Ҧ��� NODE
	if (_iFree != 0) _FreeList.clear();
	delete[] _pParticles; // ����Ҧ����o�귽
	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("particletexture.plist");
}

void CParticleSystem::onTouchesBegan(const cocos2d::CCPoint &touchPoint)
{
	CParticle *get;
	switch (_iType)
	{
	case STAY_FOR_TWOSECONDS:
		// �q _FreeList ���o�@�Ӥ��l����� _InUsed
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
		else return;// �S�����l, �ҥH�N������
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
		else return;// �S�����l, �ҥH�N������
		break;
	}
}

void CParticleSystem::onTouchesMoved(const cocos2d::CCPoint &touchPoint)
{
	CParticle *get;
	switch (_iType)
	{
	case STAY_FOR_TWOSECONDS:
		// �q _FreeList ���o�@�Ӥ��l����� _InUsed
		if (_iFree != 0) {
			get = _FreeList.front();
			get->setBehavior(STAY_FOR_TWOSECONDS);
			get->setPosition(touchPoint);
			get->setGravity(_fGravity);
			_FreeList.pop_front();
			_InUsedList.push_front(get);
			_iFree--; _iInUsed++;
		}
		else return;// �S�����l, �ҥH�N������
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
		else return;// �S�����l, �ҥH�N������
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