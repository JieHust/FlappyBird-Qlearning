#include "GameLayer.h"

using namespace std;

GameLayer::GameLayer(AppFrame* pFrame)
	: m_pFrame(pFrame), m_Stage(pFrame, this), m_Timeline(TIMESTAMPTYPE_TIME),
	m_pRootUIPage(pFrame->CreateUIPage(L"UIStyle.xml", L"UILayout.xml")),
	m_Score(0), m_MaxScore(0), m_GameCount(0), m_CollisionTopPipe(0), m_CollisionBottomPipe(0),
	m_CollisionGround(0), m_MaxCollisionPipe(0), m_MaxCountScore(0)
{
	// 初始化UI更新器
	m_UIStateUpdater.SetPage(m_pRootUIPage);

	// 加载音效
	m_pSS_Point = pFrame->CreateSoundSprite(L"Sounds\\Point.wav");
	m_pSB_Swooshing = pFrame->CreateSoundEffect(L"Sounds\\Swooshing.wav");

	// 获得UI对象
	m_pScoreLabel = m_pRootUIPage->GetControl(L"ScoreLabel");
	m_pBoardScoreLabel = m_pRootUIPage->GetControl(L"ScoreBoard.Score");
	m_pBoardMaxScoreLabel = m_pRootUIPage->GetControl(L"ScoreBoard.MaxScore");
	m_pTotalGameCountLabel = m_pRootUIPage->GetControl(L"Statistics.TotalGameCount");
	m_pMaxCollisionPipeLabel = m_pRootUIPage->GetControl(L"Statistics.MaxCollisionPipe");
	m_pMaxScoreLabel = m_pRootUIPage->GetControl(L"Statistics.MaxScore");
	m_pTopPipeCollisionLabel = m_pRootUIPage->GetControl(L"Statistics.TopPipeCollision");
	m_pBottomPipeCollisionLabel = m_pRootUIPage->GetControl(L"Statistics.BottomPipeCollision");
	m_pGroundCollision = m_pRootUIPage->GetControl(L"Statistics.GroundCollision");
	m_ScoreBoardTop = m_pRootUIPage->GetControl(L"ScoreBoard")->GetTop();
	m_HintTop = m_pRootUIPage->GetControl(L"Hint")->GetTop();
	m_HintHeight = m_pRootUIPage->GetControl(L"Hint")->GetHeight();
	m_StatisticsLeft = m_pRootUIPage->GetControl(L"Statistics")->GetLeft();

	// 创建自定义图表控件
	m_pScoreGraphControl.DirectSet(new ScoreGraph(m_pRootUIPage, L"Statistics.ScoreGraph", pFrame, m_AllScorePoints));
	m_pScoreGraphControl->SetVisiable(false);
	m_pScoreGraphControl->SetTop(0); m_pScoreGraphControl->SetLeft(0);
	m_pScoreGraphControl->SetWidth(m_pRootUIPage->GetControl(L"Statistics.ScoreGraphContainer")->GetWidth());
	m_pScoreGraphControl->SetHeight(m_pRootUIPage->GetControl(L"Statistics.ScoreGraphContainer")->GetHeight());
	m_pScoreGraphControl->SetParent(m_pRootUIPage->GetControl(L"Statistics.ScoreGraphContainer"));

	// 绑定按钮事件
	m_pRootUIPage->GetControl(L"ScoreBoard.PlayAgain")->GetEvent(L"OnClick") += 
		[&](fuiControl*, fuiEventArgs*)
		{
			SetReceiveMsg(false);
			m_pRootUIPage->ReleaseMouseEventCatch();

			m_UIStateUpdater.BindUpdater(L"BlackMask", L"BlendColor", UIStateUpdater::MakeUpdater_AlphaFader(1.0f, 0, 255));

			m_Timeline.StartEditTimeline();
			m_Timeline.Clear();
			m_Timeline.AddEvent(1.f, [&](Timeline* pTimeline) {
					SetReceiveMsg(true);
					toReadyGame();
				});
			m_Timeline.EndEditTimeline();
			m_Timeline.Goto(0);
		};
	m_pRootUIPage->GetControl(L"ScoreBoard.ShowScore")->GetEvent(L"OnClick") += 
		[&](fuiControl*, fuiEventArgs*)
		{
			SetReceiveMsg(false);
			m_pRootUIPage->ReleaseMouseEventCatch();
			
			m_pScoreGraphControl->RefreshData();
			m_pScoreGraphControl->SetVisiable(true);

			m_UIStateUpdater.BindUpdater(L"Statistics", L"Left", 
				UIStateUpdater::MakeUpdater_ValueUpdater<float>(0.5f, 800, m_StatisticsLeft));

			m_UIStateUpdater.BindUpdater(L"Statistics", L"BlendColor", 
				UIStateUpdater::MakeUpdater_AlphaFader(0.3f, 0, 255));
			m_UIStateUpdater.BindUpdater(L"Statistics.TotalGameCount", L"FontColor", 
				UIStateUpdater::MakeUpdater_AlphaFader(0.3f, 0, 255));
			m_UIStateUpdater.BindUpdater(L"Statistics.MaxCollisionPipe", L"FontColor", 
				UIStateUpdater::MakeUpdater_AlphaFader(0.3f, 0, 255));
			m_UIStateUpdater.BindUpdater(L"Statistics.MaxScore", L"FontColor", 
				UIStateUpdater::MakeUpdater_AlphaFader(0.3f, 0, 255));
			m_UIStateUpdater.BindUpdater(L"Statistics.TopPipeCollision", L"FontColor", 
				UIStateUpdater::MakeUpdater_AlphaFader(0.3f, 0, 255));
			m_UIStateUpdater.BindUpdater(L"Statistics.BottomPipeCollision", L"FontColor", 
				UIStateUpdater::MakeUpdater_AlphaFader(0.3f, 0, 255));
			m_UIStateUpdater.BindUpdater(L"Statistics.GroundCollision", L"FontColor", 
				UIStateUpdater::MakeUpdater_AlphaFader(0.3f, 0, 255));

			m_pTotalGameCountLabel->RawSetProperty(L"Text", to_wstring(m_GameCount));
			m_pMaxCollisionPipeLabel->RawSetProperty(L"Text", to_wstring(m_MaxCollisionPipe));
			m_pMaxScoreLabel->RawSetProperty(L"Text", to_wstring(m_MaxCountScore));
			m_pTopPipeCollisionLabel->RawSetProperty(L"Text", to_wstring(m_CollisionTopPipe));
			m_pBottomPipeCollisionLabel->RawSetProperty(L"Text", to_wstring(m_CollisionBottomPipe));
			m_pGroundCollision->RawSetProperty(L"Text", to_wstring(m_CollisionGround));

			m_Timeline.StartEditTimeline();
			m_Timeline.Clear();
			m_Timeline.AddEvent(0.5f, [&](Timeline* pTimeline) {
					SetReceiveMsg(true);

					m_pRootUIPage->GetControl(L"Statistics")->SetMouseTrans(false);
				});
			m_Timeline.EndEditTimeline();
			m_Timeline.Goto(0);
		};
	m_pRootUIPage->GetControl(L"Statistics.MsgMask")->GetEvent(L"OnMouseLDown") += 
		[&](fuiControl*, fuiEventArgs*)
		{
			SetReceiveMsg(false);
			m_pRootUIPage->ReleaseMouseEventCatch();

			m_UIStateUpdater.BindUpdater(L"Statistics", L"Left", 
				UIStateUpdater::MakeUpdater_ValueUpdater<float>(0.5f, m_StatisticsLeft, 800));

			m_Timeline.StartEditTimeline();
			m_Timeline.Clear();
			m_Timeline.AddEvent(0.5f, [&](Timeline* pTimeline) {
					SetReceiveMsg(true);

					m_pScoreGraphControl->SetVisiable(false);

					m_pRootUIPage->GetControl(L"Statistics")->SetProperty<fcyColor>(L"BlendColor", 0x00000000);
					m_pRootUIPage->GetControl(L"Statistics")->SetMouseTrans(true);
				});
			m_Timeline.EndEditTimeline();
			m_Timeline.Goto(0);
		};

	// 读取分数信息
	readScoreData();

	toReadyGame();
}

GameLayer::~GameLayer()
{
	writeScoreData();
}

void GameLayer::readScoreData()
{
	m_Score = 0;
	m_MaxScore = 0;
	m_GameCount = 0;
	m_CollisionTopPipe = 0;
	m_CollisionBottomPipe = 0;
	m_CollisionGround = 0;
	m_MaxCollisionPipe = 0;
	m_MaxCountScore = 0;

	m_AllCollisionPipes.clear();
	m_AllScorePoints.clear();

	try
	{
		fcyRefPointer<fcyFileStream> pFile;
		pFile.DirectSet(new fcyFileStream(L"Score.dat", false));
		fcyBinaryReader tReader(pFile);

		// 读取所有得分情况
		int tScorePoints = tReader.ReadInt32();
		for(int i = 0; i < tScorePoints; ++i)
		{
			int tScore = tReader.ReadInt32();
			int tCounts = tReader.ReadInt32();

			m_AllScorePoints[tScore] = tCounts;
			if(tScore > m_MaxScore)
				m_MaxScore = tScore;
			if(tCounts > m_AllScorePoints[m_MaxCountScore])
				m_MaxCountScore = tScore;
		}

		// 读取所有碰撞管子情况
		int tCollisionPipeCount = tReader.ReadInt32();
		for(int i = 0; i < tCollisionPipeCount; ++i)
		{
			int tWhere = tReader.ReadInt32();
			int tCounts = tReader.ReadInt32();

			m_AllCollisionPipes[tWhere] = tCounts;
			if(m_AllCollisionPipes[tWhere] > m_AllCollisionPipes[m_MaxCollisionPipe])
				m_MaxCollisionPipe = tWhere;
		}

		// 读取游戏次数
		m_GameCount = tReader.ReadInt32();

		// 读取碰撞到上方管子的次数
		m_CollisionTopPipe = tReader.ReadInt32();

		// 读取碰撞到下方管子的次数
		m_CollisionBottomPipe = tReader.ReadInt32();

		// 读取碰撞到地面的次数
		m_CollisionGround = tReader.ReadInt32();
	}
	catch(...)
	{
		m_Score = 0;
		m_MaxScore = 0;
		m_GameCount = 0;
		m_CollisionTopPipe = 0;
		m_CollisionBottomPipe = 0;
		m_CollisionGround = 0;
		m_MaxCollisionPipe = 0;
		m_MaxCountScore = 0;

		m_AllCollisionPipes.clear();
		m_AllScorePoints.clear();
	}
}

void GameLayer::writeScoreData()
{
	bool bOK = false;

	while(!bOK)
	{
		try
		{
			fcyRefPointer<fcyFileStream> pFile;
			pFile.DirectSet(new fcyFileStream(L"Score.dat", true));
			pFile->SetLength(0);  // 清空

			fcyBinaryWriter tWriter(pFile);

			// 写入所有得分情况
			tWriter.Write((int)m_AllScorePoints.size());
			for(auto i : m_AllScorePoints)
			{
				tWriter.Write((int)i.first);
				tWriter.Write((int)i.second);
			}
			
			// 写入所有碰撞管子的情况
			tWriter.Write((int)m_AllCollisionPipes.size());
			for(auto i : m_AllCollisionPipes)
			{
				tWriter.Write((int)i.first);
				tWriter.Write((int)i.second);
			}

			// 写入游戏次数
			tWriter.Write((int)m_GameCount);

			// 写入碰撞上方管子的次数
			tWriter.Write((int)m_CollisionTopPipe);

			// 写入碰撞下方管子的次数
			tWriter.Write((int)m_CollisionBottomPipe);

			// 写入碰撞地面的次数
			tWriter.Write((int)m_CollisionGround);

			bOK = true;
		}
		catch(...)
		{
//			if(IDCANCEL == MessageBox((HWND)m_pFrame->GetMainWindow()->GetHandle(), L"保存存档数据失败，请检查Score.dat是否可写。", L"错误", MB_RETRYCANCEL | MB_ICONERROR))
				return;
		}
	}
}

void GameLayer::toReadyGame()
{
	m_Stage.InitGame();

	static const wchar_t* spControlName[] = 
	{
		L"WhiteMask",
		L"ScoreBoard",
		L"ScoreBoard.NewMaxScore",
		L"ScoreBoard.PlayAgain",
		L"ScoreBoard.ShowScore"
	};

	// 将所有控件设为不可见
	for(int i = 0; i<_countof(spControlName); ++i)
	{
		m_pRootUIPage->GetControl(spControlName[i])->SetProperty<fcyColor>(L"BlendColor", 0x00000000);
	}
	m_pRootUIPage->GetControl(L"ScoreBoard.Score")->SetProperty<fcyColor>(L"FontColor", 0x00000000);
	m_pRootUIPage->GetControl(L"ScoreBoard.MaxScore")->SetProperty<fcyColor>(L"FontColor", 0x00000000);

	// 清空时间轴
	m_Timeline.StartEditTimeline();
	m_Timeline.Clear();
	m_Timeline.EndEditTimeline();
	m_Timeline.Goto(0);

	// 设置控件
	m_pRootUIPage->GetControl(L"GameHint")->SetProperty<fcyColor>(L"BlendColor", 0xFF000000);
	m_pRootUIPage->GetControl(L"Hint")->SetProperty<fcyColor>(L"BlendColor", 0xFF000000);
	m_pRootUIPage->GetControl(L"Hint")->SetTop(m_HintTop);
	m_pRootUIPage->GetControl(L"Hint")->RawSetProperty(L"Image", L"GetReadyImg");

	m_UIStateUpdater.BindUpdater(L"BlackMask", L"BlendColor", UIStateUpdater::MakeUpdater_AlphaFader(0.8f, 255, 0));
}

void GameLayer::OnStart(GameStage* pStage)
{
	++m_GameCount;

	m_Score = 0;
	m_pScoreLabel->RawSetProperty(L"Text", L"0");

	m_pScoreLabel->SetVisiable(true);
}

void GameLayer::OnHit(GameStage* pStage, COLLISIONRESULT CollisionAt, int PipeNumber)
{
	switch(CollisionAt)
	{
	case COLLISIONRESULT_TOP_PIPE:
		++m_CollisionTopPipe;

		m_AllCollisionPipes[PipeNumber]++;
		if(m_AllCollisionPipes[m_MaxCollisionPipe] < m_AllCollisionPipes[PipeNumber])
			m_MaxCollisionPipe = PipeNumber;
		break;
	case COLLISIONRESULT_BOTTOM_PIPE:
		++m_CollisionBottomPipe;

		m_AllCollisionPipes[PipeNumber]++;
		if(m_AllCollisionPipes[m_MaxCollisionPipe] < m_AllCollisionPipes[PipeNumber])
			m_MaxCollisionPipe = PipeNumber;
		break;
	case COLLISIONRESULT_GROUND:
		++m_CollisionGround;
		break;
	}

	m_UIStateUpdater.BindUpdater(L"WhiteMask", L"BlendColor", 
		[=](float ElapsedTime, float& TotalTime, fuiProperty* pProp, fuiControl* pControl)
		{
			wstring tStr;
			fcyColor tColor;

			TotalTime += ElapsedTime;
			if(TotalTime < 0.15f)
				tColor = fcyColor((fByte)(0 + 255 * (TotalTime / 0.15f)), 0, 0, 0);
			else if(TotalTime < 0.3f)
				tColor = fcyColor((fByte)(255 - 255 * ((TotalTime - 0.15f) / 0.15f)), 0, 0, 0);
			else
				tColor = fcyColor(0, 0, 0, 0);

			fuiPropertyAccessorHelper<fcyColor>::DefaultGetter(tStr, &tColor);
			pProp->Set(tStr);

			return TotalTime < 0.4f;
		});
}

void GameLayer::OnDied(GameStage* pStage)
{
	// 记录分数
	m_AllScorePoints[m_Score]++;
	if(m_AllScorePoints[m_Score] > m_AllScorePoints[m_MaxCountScore])
		m_MaxCountScore = m_Score;

	this->SetReceiveMsg(false);

	m_pBoardScoreLabel->RawSetProperty(L"Text", L"0");
	m_pBoardMaxScoreLabel->RawSetProperty(L"Text", to_wstring(m_MaxScore));

	m_pSB_Swooshing->SetTime(0);
	m_pSB_Swooshing->Play();

	m_pScoreLabel->SetVisiable(false);
	m_pRootUIPage->GetControl(L"Hint")->RawSetProperty(L"Image", L"GameOverImg");
	m_UIStateUpdater.BindUpdater(L"Hint", L"BlendColor", 
		UIStateUpdater::MakeUpdater_AlphaFader(0.3f, 0, 255));
	m_UIStateUpdater.BindUpdater(L"Hint", L"Top",
		UIStateUpdater::MakeUpdater_ValueUpdater<float>(0.5f, -m_HintHeight, m_HintTop));

	m_Timeline.StartEditTimeline();
	m_Timeline.Clear();

	m_Timeline.AddEvent(0.8, [&](Timeline* pTimeline) {
			m_pSB_Swooshing->SetTime(0);
			m_pSB_Swooshing->Play();

			m_UIStateUpdater.BindUpdater(L"ScoreBoard", L"BlendColor", 
				UIStateUpdater::MakeUpdater_AlphaFader(0.3f, 0, 255));
			m_UIStateUpdater.BindUpdater(L"ScoreBoard.Score", L"FontColor", 
				UIStateUpdater::MakeUpdater_AlphaFader(0.3f, 0, 255));
			m_UIStateUpdater.BindUpdater(L"ScoreBoard.MaxScore", L"FontColor", 
				UIStateUpdater::MakeUpdater_AlphaFader(0.3f, 0, 255));
			m_UIStateUpdater.BindUpdater(L"ScoreBoard.PlayAgain", L"BlendColor", 
				UIStateUpdater::MakeUpdater_AlphaFader(0.3f, 0, 255));
			m_UIStateUpdater.BindUpdater(L"ScoreBoard.ShowScore", L"BlendColor", 
				UIStateUpdater::MakeUpdater_AlphaFader(0.3f, 0, 255));
			m_UIStateUpdater.BindUpdater(L"ScoreBoard", L"Top",
				UIStateUpdater::MakeUpdater_ValueUpdater<float>(0.5f, 600, m_ScoreBoardTop));

			if(m_MaxScore < m_Score)
			{
				m_UIStateUpdater.BindUpdater(L"ScoreBoard.NewMaxScore", L"BlendColor", 
					UIStateUpdater::MakeUpdater_AlphaFader(0.3f, 0, 255));
			}
		});
	m_Timeline.AddEvent(1.3, [&](Timeline* pTimeline) {
			m_UIStateUpdater.BindUpdater(L"ScoreBoard.Score", L"Text",
				UIStateUpdater::MakeUpdater_ValueUpdater<int>(m_Score * 0.05f, 0, m_Score));

			if(m_MaxScore < m_Score)
			{
				m_UIStateUpdater.BindUpdater(L"ScoreBoard.MaxScore", L"Text",
					UIStateUpdater::MakeUpdater_ValueUpdater<int>((m_Score - m_MaxScore) * 0.05f, m_MaxScore, m_Score));

				m_MaxScore = m_Score;
			}

			this->SetReceiveMsg(true);
		});

	m_Timeline.EndEditTimeline();
	m_Timeline.Goto(0);

	// Add by LJ
// 	{
// 		SetReceiveMsg(false);
// 		m_pRootUIPage->ReleaseMouseEventCatch();
// 
// 		m_UIStateUpdater.BindUpdater(L"BlackMask", L"BlendColor", UIStateUpdater::MakeUpdater_AlphaFader(1.0f, 0, 255));
// 
// 		m_Timeline.StartEditTimeline();
// 		m_Timeline.Clear();
// 		m_Timeline.AddEvent(1.f, [&](Timeline* pTimeline) {
// 			SetReceiveMsg(true);
// 			toReadyGame();
// 		});
// 		m_Timeline.EndEditTimeline();
// 		m_Timeline.Goto(0);
// 	}
}

void GameLayer::OnPassPipe(GameStage* pStage)
{
	m_Score++;

	m_pSS_Point->PlayNewSound();
	m_pScoreLabel->RawSetProperty(L"Text", to_wstring(m_Score));
}

void GameLayer::OnMsg(const f2dMsg& Msg)
{
	switch(m_Stage.GetState())
	{
	case GAMESTATE_READY:
		{
			if(Msg.Type == F2DMSG_WINDOW_ONMOUSELDOWN || (Msg.Type == F2DMSG_WINDOW_ONKEYDOWN && Msg.Param1 == GAME_JUMPKEY))
			{
				m_Stage.Start();

				m_UIStateUpdater.BindUpdater(L"GameHint", L"BlendColor", 
					UIStateUpdater::MakeUpdater_AlphaFader(1.0f, 255, 0));
				m_UIStateUpdater.BindUpdater(L"Hint", L"BlendColor", 
					UIStateUpdater::MakeUpdater_AlphaFader(0.8f, 255, 0));

				m_Stage.DoMsg(Msg); // 重新发送消息，以便鸟进行跳跃
			}
		}
		break;
	case GAMESTATE_PLAYING:
	case GAMESTATE_DIYING:
		m_Stage.DoMsg(Msg);
		break;
	default:
		m_pRootUIPage->DealF2DMsg(Msg);
	}	
}

void GameLayer::OnUpdate(float ElapsedTime)
{
	m_Stage.Update(ElapsedTime);
	m_Timeline.Update(ElapsedTime);
	m_UIStateUpdater.Update(ElapsedTime);
	m_pRootUIPage->Update(ElapsedTime);
}

void GameLayer::OnRender(float ElapsedTime)
{
	m_Stage.Render(ElapsedTime);

	m_pRootUIPage->Render();
}
