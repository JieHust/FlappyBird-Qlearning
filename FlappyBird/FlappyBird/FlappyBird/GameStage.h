#pragma once
#include "AppFrame.h"
#include "Timeline.h"
#include "CConsolePrinter.h"
#include <windows.h>

#define GAME_JUMPKEY  VK_SPACE  // 跳跃键

// 游戏状态
enum GAMESTATE
{
	GAMESTATE_STOP,    // 停止一切逻辑
	GAMESTATE_READY,   // 准备
	GAMESTATE_PLAYING, // 游戏进行
	GAMESTATE_DIYING,  // 垂死
	GAMESTATE_DIED     // 死亡
};

class GameStage;

// 记录碰撞目标
enum COLLISIONRESULT
{
	COLLISIONRESULT_NONE,         // 无碰撞
	COLLISIONRESULT_TOP_PIPE,     // 与上方水管碰撞
	COLLISIONRESULT_BOTTOM_PIPE,  // 与下方水管碰撞
	COLLISIONRESULT_GROUND        // 与地面碰撞
};

// 回调
struct IStageListener
{
	virtual void OnStart(GameStage* pStage)=0;
	virtual void OnHit(GameStage* pStage, COLLISIONRESULT CollisionAt, int PipeNumber)=0;
	virtual void OnDied(GameStage* pStage)=0;
	virtual void OnPassPipe(GameStage* pStage)=0;
};

// 游戏舞台
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

	// ===== 游戏图像资源 =====
	fcyRefPointer<f2dSprite> m_pImg_Background;
	fcyRefPointer<f2dSprite> m_pImg_Ground;
	fcyRefPointer<f2dSprite> m_pImg_PipeTop;
	fcyRefPointer<f2dSprite> m_pImg_PipeBottom;
	fcyRefPointer<f2dSpriteAnimation> m_pImg_BirdAnimation;
	fcyRefPointer<f2dGeometryRenderer> m_GRenderer;
	
	// ===== 游戏声音资源 =====
	fcyRefPointer<f2dSoundBuffer> m_pSB_Died;
	fcyRefPointer<f2dSoundBuffer> m_pSB_Hit;
	fcyRefPointer<f2dSoundSprite> m_pSS_Wing;
	
	// ===== 场景状态 =====
	int m_PipeIndex;
	fcyRandomWELL512 m_Randomizer;
	f2dSpriteAnimationInstance m_BirdAnimation;
	Timeline m_GameTimeline;
	float m_GroundDrawOffset;     // 地板
	fcyVec2 m_BirdPos;            // 鸟的位置
	float m_BirdV;                // 鸟的垂直速度
	bool m_bJumpKeyPressed;       // 跳跃键是否按下（防止连续触发按下消息）
	float m_BirdRotation;         // 旋转过渡值
	float m_BirdPathLogTimer;     // 路径记录计数器
	COLLISIONRESULT m_CollisionResult;             // 是否碰撞
	std::vector<PipeData> m_Pipes;                 // 管子
	std::vector<std::pair<float, fcyVec2>> m_Path; // 记录鸟经过的位置<记录点时间, 记录点位置>
private:
	COLLISIONRESULT checkCollision(int& PipeIndex);  // 进行碰撞检测
	void checkScore();        // 检查是否得分
	void drawCollisionBox();  // (调试)绘制碰撞盒

	void updateBird(float ElapsedTime);   // 更新鸟的位置
	void updatePipes(float ElapsedTime);  // 更新水管的位置
	void updateGround(float ElapsedTime); // 更新地板的位置
	void updatePathLogger(float ElapsedTime); // 更新路径记录器

	void doRender(float ElapsedTime);   // 进行渲染
public:
	GAMESTATE GetState()const { return m_State; }

	void InitGame();   // 初始化游戏
	void Start();      // 开始游戏

	void CreatePipe(); // 创建管子
protected: // 接口实现
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
