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

	// 时间轴
	Timeline m_Timeline;

	// 声音资源	
	fcyRefPointer<f2dSoundSprite> m_pSS_Point;
	fcyRefPointer<f2dSoundBuffer> m_pSB_Swooshing;

	// UI对象
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

	// 舞台对象
	GameStage m_Stage;

	// 统计信息
	int m_Score;          // 得分
	int m_MaxScore;       // 最高分
	int m_MaxCountScore;  // 最多分数
	int m_GameCount;      // 游戏次数
	int m_CollisionTopPipe;     // 撞到上方水管
	int m_CollisionBottomPipe;  // 撞到下方水管
	int m_CollisionGround;      // 撞到地面
	int m_MaxCollisionPipe;     // 最常碰撞的管子
	std::map<int, int> m_AllCollisionPipes; // 所有碰撞的管子
	std::map<int, int> m_AllScorePoints;    // 之前所有的得分点
private:
	void readScoreData();    // 加载分数文件
	void writeScoreData();   // 写入分数文件

	void toReadyGame();   // 将所有状态切换至准备游戏状态
protected: // 接口实现
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
