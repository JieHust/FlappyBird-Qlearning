#include "AppFrame.h"

#include "GameLayer.h"

#include "resource.h"

using namespace std;

AppFrame::AppFrame()
{
	InitEngine();
	InitFileSystem();
	SetWindowIcon();

	m_GameLayer.reset(new GameLayer(this));
}

AppFrame::~AppFrame()
{
}

void AppFrame::InitEngine()
{
	// ��ʼ���������
	struct : public f2dInitialErrListener
	{
		void OnErr(fuInt TimeTick, fcStr Src, fcStr Desc)
		{
			// ��DLL�������׳��쳣
			// ABI�������⣿
			throw fcyException(Src, Desc);
		}
	} tErrListener;

	// ��ʼ������
	CreateF2DEngineAndInit(
		F2DVERSION, 
		fcyRect(0.f, 0.f, (float)GAMESIZE_W, (float)GAMESIZE_H),
		L"Flappy Bird",
		true,
		true,
		F2DAALEVEL_2,  // Ĭ������2������ݣ�����֧���Զ��ر�
		this,
		&m_pEngine,
		&tErrListener
		);
	assert(*m_pEngine != NULL);

	// ����������
	m_pMainWindow = m_pEngine->GetMainWindow();
	m_pFileSys = m_pEngine->GetFileSys();
	m_pRenderer = m_pEngine->GetRenderer();
	m_pDev = m_pRenderer->GetDevice();
	m_pSoundSys = m_pEngine->GetSoundSys();
	m_pVideoSys = m_pEngine->GetVideoSys();

	// ������Ⱦ��
	if(FCYFAILED(m_pDev->CreateGraphics2D(0,0,&m_pMainGraph2D)))
		throw fcyException("AppFrame::InitEngine", "f2dRenderDevice::CreateGraphics2D failed.");
}

void AppFrame::InitFileSystem()
{
	bool bSucceed = false;
	
	// ������Դ��
	// ����Դ���ҽӵ�ResĿ¼��
	// �ò�������ʧ��
	if(FCYOK(m_pFileSys->LoadResPackage(L"Res", L"Data.frp")))
		bSucceed = true;

	// ����Ŀ¼
	// ������Դ��ͬ����Ŀ¼�ҽӵ�PatchĿ¼��
	// �ò�������ʧ�ܣ�������Դ���Ͳ���Ŀ¼����Ҫ�ҽ�һ��
	// �����߾����ڣ��Բ���Ŀ¼����
	if(FCYOK(m_pFileSys->LoadRealPath(L"Patch", L"Data")))
		bSucceed = true;

	// ���ҽ�ʧ�ܣ��׳��쳣
	if(!bSucceed)
		throw fcyException(
				"AppFrame::InitFileSystem",
				"Load filesystem failed, resource package '%s' not found.",
				fcyStringHelper::WideCharToMultiByte(L"Data.frp").c_str()
			);
}

void AppFrame::SetWindowIcon()
{
	HWND hWindow = (HWND)m_pMainWindow->GetHandle();
	HICON hIcon = LoadIcon((HINSTANCE)GetWindowLong(hWindow, GWL_HINSTANCE), MAKEINTRESOURCE(IDI_MAINICON));
	SendMessage(hWindow, WM_SETICON, ICON_BIG,  (LPARAM)hIcon);
	SendMessage(hWindow, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
}

f2dStream* AppFrame::QueryStream(const std::wstring& Path)
{
	f2dStream* pStream = m_pFileSys->GetStream((L"Patch\\" + Path).c_str());
	if(!pStream)
		pStream = m_pFileSys->GetStream((L"Res\\" + Path).c_str());

	if(!pStream)
		throw fcyException(
				"AppFrame::QueryStream",
				"Path '%s' not found.",
				fcyStringHelper::WideCharToMultiByte(Path).c_str()
			);

	return pStream;
}

f2dTexture2D* AppFrame::QueryTexture(const std::wstring& ResSrc)
{
	unordered_map<std::wstring, fcyRefPointer<f2dTexture2D>>::iterator i = m_TexCache.find(ResSrc);

	if(i == m_TexCache.end())
	{
		// ��ȡ��
		f2dStream* pStream = QueryStream(ResSrc.c_str());
		pStream->SetPosition(FCYSEEKORIGIN_BEG, 0);

		// ��������
		f2dTexture2D* pTex = NULL;
		if(FCYFAILED(m_pRenderer->GetDevice()->CreateTextureFromStream(pStream, 0, 0, false, true, &pTex)))
		{
			throw fcyException("AppFrame::QueryTexture", "f2dRenderer::CreateTextureFromStream failed.");
		}

		m_TexCache[ResSrc].DirectSet(pTex);

		return pTex;
	}
	else
		return i->second;
}

f2dFontProvider* AppFrame::QueryFont(const std::wstring& ResSrc, const fcyVec2& Size)
{
	unordered_map<std::pair<std::wstring, fcyVec2>, fcyRefPointer<f2dFontProvider>>::iterator i = m_FontCache.find(pair<wstring, fcyVec2>(ResSrc, Size));

	if(i == m_FontCache.end())
	{
		// ��ȡ��
		f2dStream* pStream = QueryStream(ResSrc.c_str());
		pStream->SetPosition(FCYSEEKORIGIN_BEG, 0);

		// ��������
		f2dFontProvider* pFont = NULL;
		if(FCYFAILED(m_pRenderer->CreateFontFromFile(pStream, 0, Size, F2DFONTFLAG_NONE, &pFont)))
		{
			throw fcyException("AppFrame::QueryFont", "f2dRenderer::CreateFontFromFile failed.");
		}

		m_FontCache[pair<std::wstring, fcyVec2>(ResSrc, Size)].DirectSet(pFont);
		
		return pFont;
	}
	else
		return i->second;
}

f2dFontProvider* AppFrame::QuerySpriteFont(const std::wstring& TexSrc, const std::wstring& DefineSrc)
{
	unordered_map<std::pair<std::wstring, std::wstring>, fcyRefPointer<f2dFontProvider>>::iterator i = m_SpriteFontCache.find(pair<wstring, wstring>(TexSrc, DefineSrc));

	if(i == m_SpriteFontCache.end())
	{
		// ��ѯ����
		f2dTexture2D* pTex = QueryTexture(TexSrc);

		// ��ȡ��
		f2dStream* pStream = QueryStream(DefineSrc.c_str());
		pStream->SetPosition(FCYSEEKORIGIN_BEG, 0);

		// ��������
		f2dFontProvider* pFont = NULL;
		if(FCYFAILED(m_pRenderer->CreateFontFromTex(pStream, pTex, &pFont)))
		{
			throw fcyException("AppFrame::QuerySpriteFont", "f2dRenderer::CreateFontFromTex failed.");
		}

		m_SpriteFontCache[pair<std::wstring, std::wstring>(TexSrc, DefineSrc)].DirectSet(pFont);
		
		return pFont;
	}
	else
		return i->second;
}

fcyRefPointer<f2dSprite> AppFrame::CreateSprite2D(const std::wstring& TexSrc)
{
	fcyRefPointer<f2dSprite> tRet;
	f2dTexture2D* pTex = QueryTexture(TexSrc);

	if(FCYFAILED(GetRenderer()->CreateSprite2D(pTex, &tRet)))
		throw fcyException("AppFrame::CreateSprite2D", "f2dRenderer::CreateSprite2D failed.");

	return tRet;
}

fcyRefPointer<fuiPage> AppFrame::CreateUIPage(const std::wstring& StyleFile, const std::wstring& LayoutFile)
{
	fcyRefPointer<fuiPage> tRet;
	tRet.DirectSet(new fuiPage(L"Root", GetRenderer(), GetMainGraph2D()));

	tRet->GetControlStyle()->LoadResFromFile(QueryStream(StyleFile), this);
	tRet->LoadLayoutFromFile(QueryStream(LayoutFile));

	return tRet;
}

fcyRefPointer<f2dSoundBuffer> AppFrame::CreateBGM(const std::wstring& Path)
{
	fcyRefPointer<f2dSoundBuffer> tRet;
	fcyRefPointer<f2dSoundDecoder> tDecoder;

	if(FCYFAILED(m_pSoundSys->CreateOGGVorbisDecoder(QueryStream(Path), &tDecoder)))
		throw fcyException(
			"AppFrame::CreateBGM", 
			"CreateOGGVorbisDecoder failed on file '%s'.",
			fcyStringHelper::WideCharToMultiByte(Path).c_str()
			);
	m_pSoundSys->CreateDynamicBuffer(tDecoder, true, &tRet);

	return tRet;
}

fcyRefPointer<f2dSoundBuffer> AppFrame::CreateSoundEffect(const std::wstring& Path)
{
	fcyRefPointer<f2dSoundBuffer> tRet;
	fcyRefPointer<f2dSoundDecoder> tDecoder;

	if(FCYFAILED(m_pSoundSys->CreateWaveDecoder(QueryStream(Path), &tDecoder)))
		throw fcyException(
			"AppFrame::CreateSoundEffect", 
			"CreateWaveDecoder failed on file '%s'.",
			fcyStringHelper::WideCharToMultiByte(Path).c_str()
			);
	m_pSoundSys->CreateStaticBuffer(tDecoder, true, &tRet);

	return tRet;
}

fcyRefPointer<f2dSoundSprite> AppFrame::CreateSoundSprite(const std::wstring& Path)
{
	fcyRefPointer<f2dSoundSprite> tRet;
	fcyRefPointer<f2dSoundDecoder> tDecoder;

	if(FCYFAILED(m_pSoundSys->CreateWaveDecoder(QueryStream(Path), &tDecoder)))
		throw fcyException(
			"AppFrame::CreateSoundSprite", 
			"CreateWaveDecoder failed on file '%s'.",
			fcyStringHelper::WideCharToMultiByte(Path).c_str()
			);
	m_pSoundSys->CreateSoundSprite(tDecoder, true, 5, &tRet);

	return tRet;
}

void AppFrame::Run()
{
	m_pMainWindow->MoveToCenter();
	m_pMainWindow->SetVisiable(true);
	m_pEngine->Run(F2DENGTHREADMODE_MULTITHREAD, GAMEFRAMERATE);
}

void AppFrame::Abort()
{
	m_pEngine->Abort();
	m_pMainWindow->SetVisiable(false);
}

fBool AppFrame::OnUpdate(fDouble ElapsedTime, f2dFPSController* pFPSController, f2dMsgPump* pMsgPump)
{
	f2dMsg tMsg;
	while(FCYOK(pMsgPump->GetMsg(&tMsg)))
	{
		switch(tMsg.Type)
		{
		case F2DMSG_WINDOW_ONCLOSE:
			return false;
		default:
			m_GameLayer->DoMsg(tMsg);
			break;
		}
	}

	m_GameLayer->Update((float)ElapsedTime);

	return true;
}

fBool AppFrame::OnRender(fDouble ElapsedTime, f2dFPSController* pFPSController)
{
	GetRenderDev()->Clear();
	
	m_GameLayer->Render((float)ElapsedTime);

	return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	// _CrtSetBreakAlloc(542);
#endif

	try
	{
		AppFrame tFrame;
		tFrame.Run();
	}
	catch(const fcyException& e)
	{
		fChar tBuf[1024];
		sprintf_s(tBuf, "��Ϸ����δ���������ʱ����\n\n����������%s\n���ԣ�%s", e.GetDesc(), e.GetSrc());

		MessageBoxA(0, tBuf, "����", MB_ICONERROR);
		return -1;
	}

	return 0;
}
