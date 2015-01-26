#pragma once
#include "AppFrame.h"
#include "Timeline.h"
#include "CConsolePrinter.h"
#include <windows.h>

#define GAME_JUMPKEY  VK_SPACE  // ��Ծ��

// ��Ϸ״̬
enum GAMESTATE
{
	GAMESTATE_STOP,    // ֹͣһ���߼�
	GAMESTATE_READY,   // ׼��
	GAMESTATE_PLAYING, // ��Ϸ����
	GAMESTATE_DIYING,  // ����
	GAMESTATE_DIED     // ����
};

class GameStage;

// ��¼��ײĿ��
enum COLLISIONRESULT
{
	COLLISIONRESULT_NONE,         // ����ײ
	COLLISIONRESULT_TOP_PIPE,     // ���Ϸ�ˮ����ײ
	COLLISIONRESULT_BOTTOM_PIPE,  // ���·�ˮ����ײ
	COLLISIONRESULT_GROUND        // �������ײ
};

// �ص�
struct IStageListener
{
	virtual void OnStart(GameStage* pStage)=0;
	virtual void OnHit(GameStage* pStage, COLLISIONRESULT CollisionAt, int PipeNumber)=0;
	virtual void OnDied(GameStage* pStage)=0;
	virtual void OnPassPipe(GameStage* pStage)=0;
};

// ��Ϸ��̨
class GameStage :
	public ILayer
{
	struct PipeData
	{
		fcyVec2 Center;
		bool Passed;
		int Index;
	};
protected:
	AppFrame* m_pFrame;
	f2dGraphics2D* m_pGraph;
	GAMESTATE m_State;
	IStageListener* m_pListener;

	// ===== ��Ϸͼ����Դ =====
	fcyRefPointer<f2dSprite> m_pImg_Background;
	fcyRefPointer<f2dSprite> m_pImg_Ground;
	fcyRefPointer<f2dSprite> m_pImg_PipeTop;
	fcyRefPointer<f2dSprite> m_pImg_PipeBottom;
	fcyRefPointer<f2dSpriteAnimation> m_pImg_BirdAnimation;
	fcyRefPointer<f2dGeometryRenderer> m_GRenderer;
	
	// ===== ��Ϸ������Դ =====
	fcyRefPointer<f2dSoundBuffer> m_pSB_Died;
	fcyRefPointer<f2dSoundBuffer> m_pSB_Hit;
	fcyRefPointer<f2dSoundSprite> m_pSS_Wing;
	
	// ===== ����״̬ =====
	int m_PipeIndex;
	fcyRandomWELL512 m_Randomizer;
	f2dSpriteAnimationInstance m_BirdAnimation;
	Timeline m_GameTimeline;
	float m_GroundDrawOffset;     // �ذ�
	fcyVec2 m_BirdPos;            // ���λ��
	float m_BirdV;                // ��Ĵ�ֱ�ٶ�
	bool m_bJumpKeyPressed;       // ��Ծ���Ƿ��£���ֹ��������������Ϣ��
	float m_BirdRotation;         // ��ת����ֵ
	float m_BirdPathLogTimer;     // ·����¼������
	COLLISIONRESULT m_CollisionResult;             // �Ƿ���ײ
	std::vector<PipeData> m_Pipes;                 // ����
	std::vector<std::pair<float, fcyVec2>> m_Path; // ��¼�񾭹���λ��<��¼��ʱ��, ��¼��λ��>
private:
	COLLISIONRESULT checkCollision(int& PipeIndex);  // ������ײ���
	void checkScore();        // ����Ƿ�÷�
	void drawCollisionBox();  // (����)������ײ��

	void updateBird(float ElapsedTime);   // �������λ��
	void updatePipes(float ElapsedTime);  // ����ˮ�ܵ�λ��
	void updateGround(float ElapsedTime); // ���µذ��λ��
	void updatePathLogger(float ElapsedTime); // ����·����¼��

	void doRender(float ElapsedTime);   // ������Ⱦ
public:
	GAMESTATE GetState()const { return m_State; }

	void InitGame();   // ��ʼ����Ϸ
	void Start();      // ��ʼ��Ϸ

	void CreatePipe(); // ��������
protected: // �ӿ�ʵ��
	void OnMsg(const f2dMsg& Msg);
	void OnUpdate(float ElapsedTime);
	void OnRender(float ElapsedTime);
public:
	GameStage(AppFrame* pFrame, IStageListener* pListener);

public:
	
	HANDLE hPipe;   // Add by LJ

	int dieCount;
	CConsolePrinter *CPrint;
};
