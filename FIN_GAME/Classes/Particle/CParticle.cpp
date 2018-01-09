#include <cmath>
#include "CParticle.h"

// 根據重力與時間計算 delta time 之間的位移差量 再轉成螢幕的位移
// 假設 2.5 秒讓一個分子從螢幕最上方移動到最下方, 也就是移動 720 PIXEL
// 720 PIXEL = 0.5*9.8*2.5*2.5 m => 1M = 23.5102 PIXEL
// 因為Y 軸往下為負, 所以重力要加上負號, 代表方向

#define PTM_RATIO 32.0f

#define FALLING_TIME 2.5f
#define MAX_HEIGHT 720.0f
#define PIXEL_PERM (2.0f*MAX_HEIGHT/(9.8f*FALLING_TIME*FALLING_TIME))
#define GRAVITY_Y(t,dt,g) ((g)*(t+0.5f*(dt)))  //已經經過 t 秒後，再經過 dt 

#define WIND_X(t,dt,wind,dir) (wind*((cosf(dir/(180/M_PI))*(dt))))
#define WIND_Y(t,dt,wind,dir) (wind*((sinf(dir/(180/M_PI))*(dt))))

#define LIFE_NOISE(f) ((f)*(1.0f-(rand()%2001/1000.0f)))
//#define INTENSITY(f)  ( (f) >= 255 ? 255 : (f) )

inline int INTENSITY(float f) {
	if (f >= 255) return(255);
	else if (f <= 0) return(0);
	else return((int)f);
}

USING_NS_CC;

CParticle::CParticle()
{
	_fGravity = 0;
	_fWindDirection = 90;
	_fWind = 0;
}

bool CParticle::doStep(float dt)
{
	float cost, sint;
	switch (_iType) {
	case STAY_FOR_TWOSECONDS:
		if (!_bVisible && _fElapsedTime >= _fDelayTime ) {
			_fElapsedTime = _fElapsedTime - _fDelayTime; // 重新開始計時
			_bVisible = true;
			_Particle->setVisible(_bVisible);
			_Particle->setColor(_color);
			_Particle->setPosition(_Pos);
		}
		else if (_fElapsedTime> _fLifeTime) {
			_bVisible = false;
			_Particle->setVisible(_bVisible);
			return true; // 分子生命週期已經結束
		}
		else {
			sint = sinf(M_PI*_fElapsedTime / _fLifeTime);
			cost = cosf(M_PI_2*_fElapsedTime / _fLifeTime);
			_Particle->setScale(1 + sint * 2);
			_Particle->setOpacity(_fOpacity * cost);
			_Particle->setColor(Color3B(INTENSITY((_color.r + sint * 64)*(1 + sint)), 
				INTENSITY((_color.g - cost * 32)*(1 + sint)), INTENSITY((_color.b - sint * 64)*(1 + sint))));
			float tt = GRAVITY_Y(_fElapsedTime, dt, _fGravity);
			_Pos.y += (_Direction.y  * _fVelocity + tt) * dt * PIXEL_PERM;
			_Particle->setPosition(_Pos);
		}
		break;
	case RANDOMS_FALLING:
		if (!_bVisible && _fElapsedTime >= _fDelayTime) {
			_fElapsedTime = _fElapsedTime - _fDelayTime; // 重新開始計時
			_bVisible = true;
			_Particle->setVisible(_bVisible);
			_Particle->setColor(_color);
			_Particle->setPosition(_Pos);
		}
		else if (_fElapsedTime > _fLifeTime) {
			_bVisible = false;
			_Particle->setVisible(_bVisible);
			return true; // 分子生命週期已經結束
		}
		else {
			sint = sinf(M_PI*_fElapsedTime / _fLifeTime);
			cost = cosf(M_PI_2*_fElapsedTime / _fLifeTime);
			_Particle->setScale(1 + sint * 1.25f);
			_Particle->setOpacity(_fOpacity * cost);
			_Particle->setColor(_color);
			_Pos.x += _Direction.x * _fVelocity * dt * PIXEL_PERM;
			float tt = GRAVITY_Y(_fElapsedTime, dt, _fGravity);
			_Pos.y += (_Direction.y * _fVelocity + tt)* dt * PIXEL_PERM;
			_Particle->setPosition(_Pos);
		}
		break;
	case EMITTER_DEFAULT:
		if (!_bVisible && _fElapsedTime >= _fDelayTime) {
			_fElapsedTime = _fElapsedTime - _fDelayTime; // 重新開始計時
			_bVisible = true;
			_Particle->setVisible(_bVisible);
			_Particle->setColor(_color);
			_Particle->setPosition(_Pos);
		}
		else if (_fElapsedTime > _fLifeTime) {
			_bVisible = false;
			_Particle->setVisible(_bVisible);
			return true; // 分子生命週期已經結束
		}
		else {
			sint = sinf(M_PI*_fElapsedTime / _fLifeTime);
			cost = cosf(M_PI_2*_fElapsedTime / _fLifeTime);
			_Particle->setScale(_fSize + sint * 1.5f );
			_Particle->setOpacity(_fOpacity * cost);
			_Particle->setColor(Color3B(INTENSITY(_color.r*(1 + sint)), INTENSITY(_color.g*(1 + sint)), INTENSITY(_color.b*(1 + sint))));
//			_Particle->setColor(_color);

			float wind_x = WIND_X(_fElapsedTime, dt, _fWind, _fWindDirection);
			_Pos.x += (_Direction.x * _fVelocity + wind_x) * dt * PIXEL_PERM;
			float tt = GRAVITY_Y(_fElapsedTime, dt, _fGravity);
			float wind_y = WIND_Y(_fElapsedTime, dt, _fWind, _fWindDirection);
			_Pos.y += (_Direction.y * _fVelocity + tt + wind_y)* dt * PIXEL_PERM;
			_Particle->setPosition(_Pos);
			
			float degree = _fSpin * _fElapsedTime;
			_Particle->setRotation(degree);
		}
		break;
	}
	// 累加時間
	_fElapsedTime += dt;
	return false;
}


void CParticle::setBehavior(int iType)
{
	float t;
	_iType = iType;
	switch (_iType) {
	case STAY_FOR_TWOSECONDS:
		_fVelocity = 0;
		_fLifeTime = 2.5f + LIFE_NOISE(0.15f);
		_fIntensity = 1;
		_fOpacity = 255;
		_fSpin = 0;
		_fSize = 1;
		_color = Color3B(64 + rand() % 128, 64 + rand() % 128, 64 + rand() % 128);
		_fElapsedTime = 0;
		_fDelayTime = 0;
		_fGravity = 0;
		_Particle->setOpacity(255);
		_Particle->setScale(_fSize);
		break;
	case RANDOMS_FALLING:
		_fVelocity = 5.0f + rand() % 10 / 10.0f; // 單位 M/Sec
		_Direction.x = 0;
		_Direction.y = -1;
		_fLifeTime = 3.0f + LIFE_NOISE(0.15f);
		_fIntensity = 1;
		_fOpacity = 255;
		_fSpin = 0;
		_fSize = 1;
		_color = Color3B(128 + rand() % 128, 128 + rand() % 128, 128 + rand() % 128);
		_fElapsedTime = 0;
		_fDelayTime = 0;
		_fGravity = 0;
		break;
	case EMITTER_DEFAULT:
		_fIntensity = 1;
		_fOpacity = 255;
		_fSize = 1;
		_color = Color3B(rand() % 128, rand() % 128, 128 + rand() % 128);
		_fElapsedTime = 0;
		_fDelayTime = 0;
		_Particle->setScale(_fSize);
		break;
	}
}
void CParticle::setLifetime(const float lt) {
	_fLifeTime = lt + LIFE_NOISE(0.15f);;
}


void CParticle::setParticle(const char *pngName, cocos2d::Layer &inlayer)
{
	_Particle = Sprite::createWithSpriteFrameName(pngName);
	_Particle->setPosition(Point(rand() % 1024, rand() % 768));
	_Particle->setOpacity(255);
	_Particle->setColor(Color3B::WHITE);
	_bVisible = false;
	_Particle->setVisible(false);
	_iType = 0;
	//BlendFunc blendfunc = {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA};  
	BlendFunc blendfunc = { GL_SRC_ALPHA, GL_ONE };
	_Particle->setBlendFunc(blendfunc);
	inlayer.addChild(_Particle, 1);
}

void CParticle::setVisible()
{
	_bVisible = true;
	_Particle->setVisible(_bVisible);
}

void CParticle::setPosition(const cocos2d::Point &inPos) {
	_Pos = inPos;
};

void CParticle::setGravity(const float fGravity)
{
	_fGravity = fGravity;
}

void CParticle::setSpin(const float fSpin)
{
	_fSpin = fSpin;
}

void CParticle::setOpacity(const float fOpacity)
{
	_fOpacity = fOpacity;
}

void CParticle::setSprite(const char *pngName)
{
	_Particle->setSpriteFrame(pngName);
}

void CParticle::setWind(const float wind, const float windDir)
{
	_fWindDirection = windDir;
	_fWind = wind * 50.0f;
}
