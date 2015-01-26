#include "GameStage.h"

#include <fstream>
#include "Collision.h"
#include <time.h>

// 度量参数
#define MS_GROUNDWIDTH      336
#define MS_GROUNDHEIGHT     112
#define MS_GROUNDDRAWCOUNT  (GAMESIZE_W / MS_GROUNDWIDTH + 1)
#define MS_PIPEWIDTH        52
#define MS_PIPEHEIGHT       420
#define MS_BIRDIMGSIZE_W    40
#define MS_BIRDIMGSIZE_H    40

#define MS_STAGEMOVESPEED 150   // 移动速度
#define MS_PIPECREATETIME 1.7   // 管子创建时间间隔
#define MS_PIPEGAPHEIGHT  100   // 管子间隙高度
#define MS_PIPECREATEYMINRESERVED  0.f
#define MS_PIPECREATEYMAXRESERVED  100.f
#define MS_PIPECREATEYMIN (GAMESIZE_H - MS_PIPEHEIGHT - (MS_PIPEGAPHEIGHT / 2.f) + MS_PIPECREATEYMINRESERVED) // 管子间隙中心最小Y值
#define MS_PIPECREATEYMAX (MS_PIPEHEIGHT + (MS_PIPEGAPHEIGHT / 2.f) - MS_PIPECREATEYMAXRESERVED)              // 管子间隙中心最大Y值

#define MS_BIRDCENTERX  200.f            // 鸟的水平位置
#define MS_BIRDBOUNDINGCIRCLESIZE 10.f   // 鸟的包围圆半径
#define MS_GRAVITY  800.f                // 重力加速度
#define MS_BIRDJUMPV 320.f               // 按下按键后鸟的速度
#define MS_BIRDMAXV  380.f               // 鸟的最大速度

#define MS_BIRDMAXROTATION (3.1415926f / 4.f + 3.1415926f / 8.f)  // 朝上的最大角度
#define MS_BIRDMINROTATION (MS_BIRDMAXROTATION)   // 朝下的最大角度
#define MS_BIRDROTATIONWEIGHT   1.25f        // 旋转的插值权重

#define MS_PATHLOGTIME          0.1f         // 路径记录时间间隔
#define MS_PATHFADEOUTTIME      2.5f         // 路径点消失时间
#define MS_PATHSTARTFADEOUTTIME 1.5f         // 路径点开始消失时间
#define MS_PATHCOLOR            0xFFE0E0E0   // 路径点颜色
#define MS_PATHSIZE             3.f          // 路径点大小

#define AI 0 // 1 means AI version, 0 means player mode.

using namespace std;

GameStage::GameStage(AppFrame* pFrame, IStageListener* pListener)
	: m_pFrame(pFrame), m_GameTimeline(TIMESTAMPTYPE_TIME), m_State(GAMESTATE_STOP), m_pListener(pListener),
	m_GroundDrawOffset(0), m_CollisionResult(COLLISIONRESULT_NONE), m_BirdV(0.f), m_bJumpKeyPressed(false), m_BirdRotation(0.f),
	m_BirdPathLogTimer(0), m_PipeIndex(0)
{
	assert(pListener != nullptr);

	m_pGraph = pFrame->GetMainGraph2D();

	// 构造主图形渲染器
	pFrame->GetRenderer()->CreateGeometryRenderer(&m_GRenderer);

	// 加载图片
	m_pImg_Background = pFrame->CreateSprite2D(L"Images\\Background.png");
	m_pImg_Ground = pFrame->CreateSprite2D(L"Images\\Ground.png");
	m_pImg_PipeTop = pFrame->CreateSprite2D(L"Images\\Pipe_Top.png");
	m_pImg_PipeBottom = pFrame->CreateSprite2D(L"Images\\Pipe_Bottom.png");

	// 加载声音
	m_pSB_Died = pFrame->CreateSoundEffect(L"Sounds\\Die.wav");
	m_pSB_Hit = pFrame->CreateSoundEffect(L"Sounds\\Hit.wav");
	m_pSS_Wing = pFrame->CreateSoundSprite(L"Sounds\\Wing.wav");

	// 加载动画
	pFrame->GetRenderer()->CreateSpriteAnimation(&m_pImg_BirdAnimation);
	m_pImg_BirdAnimation->SplitAndAppend(
		3, pFrame->QueryTexture(L"Images\\Birds.png"), 
		fcyRect(0.f, 0.f, MS_BIRDIMGSIZE_W * 3.f, MS_BIRDIMGSIZE_H), 3);
	m_pImg_BirdAnimation->SetLoopStart(0);
	m_pImg_BirdAnimation->SetLoopEnd(-1);
	m_pImg_BirdAnimation->SetLoop(true);

	if (AI)
	{
		if (WaitNamedPipe("\\\\.\\Pipe\\Test", NMPWAIT_WAIT_FOREVER) == FALSE)
		{
			assert(0);
		}
		else
		{
			hPipe = CreateFile("\\\\.\\Pipe\\Test", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
			if (hPipe == INVALID_HANDLE_VALUE)
			{
				assert(0);
			} 
		}
	}	

	InitGame();
	m_State = GAMESTATE_STOP;
}

COLLISIONRESULT GameStage::checkCollision(int& PipeIndex)
{
	PipeIndex = -1;

	// 检查与水管的碰撞
 	for(auto i = m_Pipes.begin(); i != m_Pipes.end(); ++i)
	{
		// 计算上水管绘制位置
		fcyVec2 tPipeTopCenter(i->Center.x, i->Center.y - MS_PIPEGAPHEIGHT / 2.f - MS_PIPEHEIGHT / 2.f);

		// 计算下水管绘制位置
		fcyVec2 tPipeBottomCenter(i->Center.x, i->Center.y + MS_PIPEGAPHEIGHT / 2.f + MS_PIPEHEIGHT / 2.f);

		// 进行碰撞检测
		if(Collision::OBBCircleHitTest(
			tPipeTopCenter, 
			fcyVec2(MS_PIPEWIDTH / 2.f, MS_PIPEHEIGHT / 2.f),
			0.f,
			m_BirdPos,
			MS_BIRDBOUNDINGCIRCLESIZE
			))
		{
			PipeIndex = i->Index;
			return COLLISIONRESULT_TOP_PIPE;
		}
		else if(Collision::OBBCircleHitTest(
			tPipeBottomCenter, 
			fcyVec2(MS_PIPEWIDTH / 2.f, MS_PIPEHEIGHT / 2.f),
			0.f,
			m_BirdPos,
			MS_BIRDBOUNDINGCIRCLESIZE
			))
		{
			PipeIndex = i->Index;
			return COLLISIONRESULT_BOTTOM_PIPE;
		}
	}

	// 检查与地面的碰撞
	if(Collision::OBBCircleHitTest(
		fcyVec2(GAMESIZE_W / 2.f, GAMESIZE_H - MS_GROUNDHEIGHT / 2.f),
		fcyVec2(GAMESIZE_W / 2.f, MS_GROUNDHEIGHT / 2.f),
		0.f,
		m_BirdPos,
		MS_BIRDBOUNDINGCIRCLESIZE
		))
		return COLLISIONRESULT_GROUND;

	return COLLISIONRESULT_NONE;
}

void GameStage::checkScore()
{
	for(auto i = m_Pipes.begin(); i != m_Pipes.end(); ++i)
	{
		if(m_BirdPos.x > i->Center.x + MS_PIPEWIDTH/2.0f + MS_BIRDBOUNDINGCIRCLESIZE && i->Passed == false)
		{
			i->Passed = true;

			m_pListener->OnPassPipe(this);
		}
	}
}

void GameStage::drawCollisionBox()
{
	m_GRenderer->SetPenSize(2.f);

	if(m_CollisionResult != COLLISIONRESULT_NONE)
	{
		m_GRenderer->SetPenColor(0, 0xFFFFFFFF);
		m_GRenderer->SetPenColor(1, 0xFFFFFFFF);
		m_GRenderer->SetPenColor(2, 0xFFFFFFFF);
	}
	else
	{
		m_GRenderer->SetPenColor(0, 0xFFFF0000);
		m_GRenderer->SetPenColor(1, 0xFFFF0000);
		m_GRenderer->SetPenColor(2, 0xFFFF0000);
	}

	// 绘制鸟的碰撞盒
	m_GRenderer->DrawCircle(m_pGraph, m_BirdPos, MS_BIRDBOUNDINGCIRCLESIZE, 25);

	m_GRenderer->SetPenColor(0, 0xFFFFFF00);
	m_GRenderer->SetPenColor(1, 0xFFFFFF00);
	m_GRenderer->SetPenColor(2, 0xFFFFFF00);

	// 绘制水管的碰撞盒
	for(auto i = m_Pipes.begin(); i != m_Pipes.end(); ++i)
	{
		// 计算上水管绘制位置
		fcyVec2 tPipeTopCenter(i->Center.x, i->Center.y - MS_PIPEGAPHEIGHT / 2.f - MS_PIPEHEIGHT / 2.f);

		// 计算下水管绘制位置
		fcyVec2 tPipeBottomCenter(i->Center.x, i->Center.y + MS_PIPEGAPHEIGHT / 2.f + MS_PIPEHEIGHT / 2.f);

		// 绘制上下水管
		m_GRenderer->DrawRectangle(m_pGraph, 
			fcyRect(tPipeTopCenter.x - MS_PIPEWIDTH / 2.f, 
					tPipeTopCenter.y - MS_PIPEHEIGHT / 2.f, 
					tPipeTopCenter.x + MS_PIPEWIDTH / 2.f, 
					tPipeTopCenter.y + MS_PIPEHEIGHT / 2.f));
		m_GRenderer->DrawRectangle(m_pGraph, 
			fcyRect(tPipeBottomCenter.x - MS_PIPEWIDTH / 2.f, 
					tPipeBottomCenter.y - MS_PIPEHEIGHT / 2.f, 
					tPipeBottomCenter.x + MS_PIPEWIDTH / 2.f, 
					tPipeBottomCenter.y + MS_PIPEHEIGHT / 2.f));
	}

	m_GRenderer->SetPenColor(0, 0xFF0000FF);
	m_GRenderer->SetPenColor(1, 0xFF0000FF);
	m_GRenderer->SetPenColor(2, 0xFF0000FF);

	// 绘制地面碰撞位置
	m_GRenderer->DrawRectangle(m_pGraph, 
		fcyRect(0.f, GAMESIZE_H - MS_GROUNDHEIGHT, GAMESIZE_W, GAMESIZE_H));
}

void GameStage::updateBird(float ElapsedTime)
{
	// 改变鸟的垂直位置
	m_BirdPos.y += m_BirdV * ElapsedTime;
	if(m_BirdPos.y - MS_BIRDBOUNDINGCIRCLESIZE < 0.f) // 禁止越界
		m_BirdPos.y = MS_BIRDBOUNDINGCIRCLESIZE;
	m_BirdV += MS_GRAVITY * ElapsedTime;
	if(m_BirdV >= MS_BIRDMAXV)  // 最大速度
		m_BirdV = MS_BIRDMAXV;

	// 计算鸟的旋转
	float tBirdTargetRotation = 0.f;
	if(m_BirdV < 0.f)
		tBirdTargetRotation = m_BirdV / MS_BIRDJUMPV * MS_BIRDMAXROTATION;
	else if(m_BirdV > 0.f)
		tBirdTargetRotation = m_BirdV / MS_BIRDMAXV * MS_BIRDMINROTATION;

	// 计算过渡旋转值(ease-out)
	m_BirdRotation = (m_BirdRotation * (MS_BIRDROTATIONWEIGHT - 1.f) + tBirdTargetRotation) / MS_BIRDROTATIONWEIGHT;
}

void GameStage::updatePipes(float ElapsedTime)
{
	// 移动管子
	auto i = m_Pipes.begin();
	while(i != m_Pipes.end())
	{
		// 计算当前管子位置
		i->Center.x -= ElapsedTime * MS_STAGEMOVESPEED;

		// 移除越界管子
		if(i->Center.x + MS_PIPEWIDTH / 2.f < 0.f)
			i = m_Pipes.erase(i);
		else
			++i;
	}
}

void GameStage::updateGround(float ElapsedTime)
{
	// 移动地板
	m_GroundDrawOffset = (float)fmod(m_GroundDrawOffset + ElapsedTime * MS_STAGEMOVESPEED, MS_GROUNDWIDTH);
}

void GameStage::updatePathLogger(float ElapsedTime)
{
	// 记录鸟的位置
	m_BirdPathLogTimer += ElapsedTime;
	if(m_BirdPathLogTimer > MS_PATHLOGTIME)
	{
		m_Path.push_back(pair<float, fcyVec2>(0.f, m_BirdPos));

		m_BirdPathLogTimer = 0.f;
	}

	// 更新鸟的路径点
	auto i = m_Path.begin();
	while(i != m_Path.end())
	{
		i->first += ElapsedTime;
		i->second.x -= MS_STAGEMOVESPEED * ElapsedTime;

		if(i->first > MS_PATHFADEOUTTIME)
			i = m_Path.erase(i);
		else
			++i;
	}
}

void GameStage::doRender(float ElapsedTime)
{
	m_pGraph->Begin();

	// 绘制背景
	m_pImg_Background->Draw(m_pGraph, fcyVec2(GAMESIZE_W / 2.f, GAMESIZE_H / 2.f));

	// 绘制管子
	for(auto i = m_Pipes.begin(); i != m_Pipes.end(); ++i)
	{
		// 计算上水管绘制位置
		fcyVec2 tPipeTopCenter(i->Center.x, i->Center.y - MS_PIPEGAPHEIGHT / 2.f - MS_PIPEHEIGHT / 2.f);

		// 计算下水管绘制位置
		fcyVec2 tPipeBottomCenter(i->Center.x, i->Center.y + MS_PIPEGAPHEIGHT / 2.f + MS_PIPEHEIGHT / 2.f);

		// 绘制上下水管
		m_pImg_PipeTop->Draw(m_pGraph, tPipeTopCenter);
		m_pImg_PipeBottom->Draw(m_pGraph, tPipeBottomCenter);
	}

	// 绘制地板
	for(int i = 0; i <= MS_GROUNDDRAWCOUNT; ++i)
	{
		m_pImg_Ground->Draw(m_pGraph, fcyVec2(MS_GROUNDWIDTH * i + MS_GROUNDWIDTH / 2.f - m_GroundDrawOffset, GAMESIZE_H - MS_GROUNDHEIGHT / 2.f));
	}

	// 绘制鸟的路径
	for(auto i = m_Path.begin(); i != m_Path.end(); ++i)
	{
		fcyColor tPathColor = MS_PATHCOLOR;

		if(i->first > MS_PATHSTARTFADEOUTTIME)
			tPathColor.a = 255 - (fByte)(255 * ((i->first - MS_PATHSTARTFADEOUTTIME) / (MS_PATHFADEOUTTIME - MS_PATHSTARTFADEOUTTIME)));

		m_GRenderer->FillCircle(m_pGraph, i->second, MS_PATHSIZE, tPathColor, tPathColor, 16);
	}

	// 绘制鸟
	m_pImg_BirdAnimation->DrawInstance(m_BirdAnimation, m_pGraph, m_BirdPos, fcyVec2(1.f, 1.f), m_BirdRotation);

	/*
	// 调试绘制
#ifdef _DEBUG
	drawCollisionBox();
#endif
	*/

	m_pGraph->End();
}

void GameStage::InitGame()
{
	m_State = GAMESTATE_READY;

	m_GroundDrawOffset = 0.f;
	m_CollisionResult = COLLISIONRESULT_NONE;
	m_bJumpKeyPressed = false;
	m_BirdPos = fcyVec2(MS_BIRDCENTERX, (GAMESIZE_H - MS_GROUNDHEIGHT) / 2.f);
	m_BirdV = 0.f;
	m_BirdRotation = 0.f;

	m_PipeIndex = 0;
	m_BirdPathLogTimer = 0.f;
	m_Path.clear();

	m_Pipes.clear();

	// 初始化动画
	m_pImg_BirdAnimation->InitInstance(m_BirdAnimation);

	// 初始化时间轴
	{
		m_GameTimeline.StartEditTimeline();
		m_GameTimeline.Clear();

		// 到指定时间创建水管
		m_GameTimeline.AddEvent(MS_PIPECREATETIME, [=](Timeline* pTimeline)
			{
				this->CreatePipe();

				// 循环事件
				m_GameTimeline.Goto(0);
			});

		m_GameTimeline.EndEditTimeline();

		m_GameTimeline.Goto(0);
	}
}

void GameStage::Start()
{
	m_State = GAMESTATE_PLAYING;

	m_pListener->OnStart(this);
}

void GameStage::CreatePipe()
{
	PipeData tData;

	// 计算创建位置(空隙中心)
	++m_PipeIndex;
	tData.Center = fcyVec2(GAMESIZE_W + MS_PIPEWIDTH / 2.f, m_Randomizer.GetRandFloat(MS_PIPECREATEYMIN, MS_PIPECREATEYMAX));
	tData.Index = m_PipeIndex;
	tData.Passed = false;

	m_Pipes.push_back(tData);
}

void GameStage::OnMsg(const f2dMsg& Msg)
{
	switch(m_State)
	{
	case GAMESTATE_STOP:
		break;
	case GAMESTATE_READY:
		break;
	case GAMESTATE_PLAYING:
		switch(Msg.Type)
		{
		case F2DMSG_WINDOW_ONMOUSELDOWN:
			m_BirdV = -MS_BIRDJUMPV;
			m_pSS_Wing->PlayNewSound();
			break;
		case F2DMSG_WINDOW_ONKEYDOWN:
			if(Msg.Param1 == GAME_JUMPKEY)
			{
				if(!m_bJumpKeyPressed)
				{
					m_BirdV = -MS_BIRDJUMPV;
					m_bJumpKeyPressed = true;

					m_pSS_Wing->PlayNewSound();
				}
			}
			break;
		case F2DMSG_WINDOW_ONKEYUP:
			if(Msg.Param1 == GAME_JUMPKEY)
			{
				m_bJumpKeyPressed = false;
			}
			break;
		}
		break;
	}
}

void GameStage::OnUpdate(float ElapsedTime)
{
	// Add by LJ,
	float h_dis = 9999, v_dis = 9999, vg_dis = 9999;
	if (AI)
	{
		for(auto i = m_Pipes.begin(); i != m_Pipes.end(); ++i)
		{
			if (i->Passed == 1)
				continue;

			h_dis = i->Center.x-m_BirdPos.x;
			v_dis = i->Center.y-m_BirdPos.y;
			vg_dis  = GAMESIZE_H - MS_GROUNDHEIGHT - m_BirdPos.y;
			break;
		}
	}	
	// Add ends


	switch(m_State)
	{
	case GAMESTATE_READY:
		updateGround(ElapsedTime);

		// 更新鸟的动画
		m_pImg_BirdAnimation->StepInstance(m_BirdAnimation);
		break;
	case GAMESTATE_PLAYING:
		{
					
			// 更新时间轴
			m_GameTimeline.Update(ElapsedTime);
			
			// Add by LJ, Keep bird at the same position before the distance less then pipe gap
			if (AI)
			{
				if(h_dis < MS_STAGEMOVESPEED*MS_PIPECREATETIME)
				{
					updateBird(ElapsedTime);
				}
			}else
			{
				updateBird(ElapsedTime);
			}
			// Add ends
			
		//	updateBird(ElapsedTime);
			updatePipes(ElapsedTime);
			updateGround(ElapsedTime);
			updatePathLogger(ElapsedTime);

			// 碰撞检测
			int tPipeIndex;
			m_CollisionResult = checkCollision(tPipeIndex);
			
			// Add by LJ, Get my data
			if (AI)
			{
// 			static time_t start_t,end_t;
// 			char debugOut[100];
// 			
// 			start_t = clock();
// 			sprintf_s(debugOut,"%f   %lf\n",ElapsedTime,double(start_t -end_t)*1000/CLOCKS_PER_SEC);
// 			end_t = start_t;
// 			CPrint->Instance()->print(debugOut);
				for(auto i = m_Pipes.begin(); i != m_Pipes.end(); ++i)
				{
					if (i->Passed == 1)
						continue;

					h_dis = i->Center.x-m_BirdPos.x;
					v_dis = i->Center.y-m_BirdPos.y;
					vg_dis  = GAMESIZE_H - MS_GROUNDHEIGHT - m_BirdPos.y;
					break;
				}

				dieCount = 0;		
				static float timegap=0;
				timegap += ElapsedTime;
				if (m_CollisionResult)
				{
					DWORD WriteNum;
					int birdstate = 0;
					WriteFile(hPipe, &h_dis, sizeof(float), &WriteNum, NULL);
					WriteFile(hPipe, &v_dis, sizeof(float), &WriteNum, NULL);
					WriteFile(hPipe, &vg_dis, sizeof(float), &WriteNum, NULL);
					WriteFile(hPipe, &birdstate, sizeof(float), &WriteNum, NULL);
				}			
				else if(timegap > 0.03 && h_dis < MS_STAGEMOVESPEED*MS_PIPECREATETIME)
		//		else if(h_dis < MS_STAGEMOVESPEED*MS_PIPECREATETIME)
				{
					timegap = 0.f;
					int birdstate = 1;	
					DWORD WriteNum;
					WriteFile(hPipe, &h_dis, sizeof(float), &WriteNum, NULL);
					WriteFile(hPipe, &v_dis, sizeof(float), &WriteNum, NULL);
					WriteFile(hPipe, &vg_dis, sizeof(float), &WriteNum, NULL);
					WriteFile(hPipe, &birdstate, sizeof(float), &WriteNum, NULL);
				
				}
			}
			// Add ends


			switch(m_CollisionResult)
			{
			case COLLISIONRESULT_TOP_PIPE:
			case COLLISIONRESULT_BOTTOM_PIPE:
				m_State = GAMESTATE_DIYING;
				m_pListener->OnHit(this, m_CollisionResult, tPipeIndex);
				m_pSB_Hit->SetTime(0);
				m_pSB_Hit->Play();
				m_pSB_Died->SetTime(0);
				m_pSB_Died->Play();
				break;
			case COLLISIONRESULT_GROUND:
				m_State = GAMESTATE_DIYING;
				m_pListener->OnHit(this, m_CollisionResult, -1);
				m_pSB_Hit->SetTime(0);
				m_pSB_Hit->Play();
				break;
			}

			// 检查分数
			checkScore();

			// 更新鸟的动画
			m_pImg_BirdAnimation->StepInstance(m_BirdAnimation);
		}
		break;
	case GAMESTATE_DIYING:
		{
			// 检测地面碰撞
			if(!Collision::OBBCircleHitTest(
				fcyVec2(GAMESIZE_W / 2.f, GAMESIZE_H - MS_GROUNDHEIGHT / 2.f),
				fcyVec2(GAMESIZE_W / 2.f, MS_GROUNDHEIGHT / 2.f),
				0.f,
				m_BirdPos,
				MS_BIRDBOUNDINGCIRCLESIZE
				))
			{
				updateBird(ElapsedTime);
			}
			else
			{
				m_State = GAMESTATE_DIED;
				m_pListener->OnDied(this);
			}
		}
		break;
	case GAMESTATE_DIED:	
		break;
	}
}

void GameStage::OnRender(float ElapsedTime)
{
	switch(m_State)
	{
	case GAMESTATE_READY:
	case GAMESTATE_PLAYING:
	case GAMESTATE_DIYING:
	case GAMESTATE_DIED:
		doRender(ElapsedTime);
		break;
	case GAMESTATE_STOP:
		break;
	}
}
