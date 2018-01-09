#ifndef __CPARTICLE_H__
#define __CPARTICLE_H__

#define STAY_FOR_TWOSECONDS 0	// �b����ƹL���a�貣�� particle
#define RANDOMS_FALLING     1	// �b����ƹL���a�詹�U���� particle
#define EMITTER_DEFAULT 99

#define Sprite_0 "flare.png"
#define Sprite_1 "bubble.png"
#define Sprite_2 "circle.png"
#define Sprite_3 "cloud.png"
#define Sprite_4 "comet.png"
#define Sprite_5 "raindrop.png"
#define Sprite_6 "spark.png"

#include "cocos2d.h"

class CParticle
{
private:
	cocos2d::Sprite *_Particle;	// ���l����
	cocos2d::Point  _OldPos;		// ���l�e�@�Ӧ�m
	cocos2d::Point  _Direction;	// ���l�ثe���B�ʤ�V�A���V�q

	float _fVelocity;	   // ���l���t��
	float _fLifeTime;	   // �ͩR�g��
	float _fIntensity;     // ���l��q���̤j�ȡA�q�`�Ω���ܤ��l���C��
	float _fOpacity;	   // �ثe���l���z����
	float _fSpin;		   // ���l������q
	float _fSize;		   // ���l���j�p
	float _fGravity;	
	float _fWindDirection; //������V
	float _fWind;          //�����j��   
	cocos2d::Color3B _color;	// ���l���C��
	// �ɶ�
	float _fElapsedTime; // ���l�q��ܨ�ثe����ɶ�
	float _fDelayTime;	 // ���l��ܫe������ɶ�
	// �欰�Ҧ�
	int _iType;
	// ��ܻP�_
	bool _bVisible;
public:
	cocos2d::Point  _Pos;			// ���l�ثe����m
	CParticle();
	void setParticle(const char *pngName, cocos2d::Layer &inlayer);

	int _iTypeNumber;
	bool _bTypeEnd = false;
	float _bTypeSize = 1.0f;

	bool doStep(float dt);
	void setPosition(const cocos2d::Point &inPos);
	void setColor(cocos2d::Color3B &color) { _color = color;}
	void setBehavior(int iType); // �]�w���l���ͪ��_�l�欰�Ҧ�
	void setVisible();
	void setGravity(const float fGravity);
	void setSpin(const float fSpin);
	void setOpacity(const float fOpacity);
	void setVelocity(const float v) { _fVelocity = v; }
	void setLifetime(const float lt);
	void setDirection(const cocos2d::Point pt) { _Direction = pt; }
	void setSize(float s) { _fSize = s; _Particle->setScale(_fSize); }
	void setSprite(const char *pngName);
	void setWind(const float wind, const float windDir);

};

#endif