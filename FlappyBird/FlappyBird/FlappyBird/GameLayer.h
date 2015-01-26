#pragma once
#include "AppFrame.h"

#include "GameStage.h"
#include "Timeline.h"
#include "UIStateUpdater.h"

#include "ScoreGraph.h"

class GameLayer :
	public ILayer,
	public IStageListener
{
protected:
	AppFrame* m_pFrame;

	// ʱ����
	Timeline m_Timeline;

	// ������Դ	
	fcyRefPointer<f2dSoundSprite> m_pSS_Point;
	fcyRefPointer<f2dSoundBuffer> m_pSB_Swooshing;

	// UI����
	fcyRefPointer<fuiPage> m_pRootUIPage;
	UIStateUpdater m_UIStateUpdater;
	fuiControl* m_pScoreLabel;
	fuiControl* m_pBoardScoreLabel;
	fuiControl* m_pBoardMaxScoreLabel;
	fuiControl* m_pTotalGameCountLabel;
	fuiControl* m_pMaxCollisionPipeLabel;
	fuiControl* m_pMaxScoreLabel;
	fuiControl* m_pTopPipeCollisionLabel;
	fuiControl* m_pBottomPipeCollisionLabel;
	fuiControl* m_pGroundCollision;
	float m_HintTop;
	float m_HintHeight;
	float m_ScoreBoardTop;
	float m_StatisticsLeft;
	
	fcyRefPointer<ScoreGraph> m_pScoreGraphControl;

	// ��̨����
	GameStage m_Stage;

	// ͳ����Ϣ
	int m_Score;          // �÷�
	int m_MaxScore;       // ��߷�
	int m_MaxCountScore;  // ������
	int m_GameCount;      // ��Ϸ����
	int m_CollisionTopPipe;     // ײ���Ϸ�ˮ��
	int m_CollisionBottomPipe;  // ײ���·�ˮ��
	int m_CollisionGround;      // ײ������
	int m_MaxCollisionPipe;     // ���ײ�Ĺ���
	std::map<int, int> m_AllCollisionPipes; // ������ײ�Ĺ���
	std::map<int, int> m_AllScorePoints;    // ֮ǰ���еĵ÷ֵ�
private:
	void readScoreData();    // ���ط����ļ�
	void writeScoreData();   // д������ļ�

	void toReadyGame();   // ������״̬�л���׼����Ϸ״̬
protected: // �ӿ�ʵ��
	void OnStart(GameStage* pStage);
	void OnHit(GameStage* pStage, COLLISIONRESULT CollisionAt, int PipeNumber);
	void OnDied(GameStage* pStage);
	void OnPassPipe(GameStage* pStage);

	void OnMsg(const f2dMsg& Msg);
	void OnUpdate(float ElapsedTime);
	void OnRender(float ElapsedTime);
public:
	GameLayer(AppFrame* pFrame);
	~GameLayer();
};
