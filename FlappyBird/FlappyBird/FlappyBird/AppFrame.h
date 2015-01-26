#pragma once
#include "Global.h"

// ��Ϸ���
class AppFrame :
	public f2dEngineEventListener,
	public fuiResProvider
{
protected:
	// f2d���
	fcyRefPointer<f2dEngine> m_pEngine;
	f2dWindow* m_pMainWindow;
	f2dFileSys* m_pFileSys;
	f2dRenderer* m_pRenderer;
	f2dRenderDevice* m_pDev;
	f2dSoundSys* m_pSoundSys;
	f2dVideoSys* m_pVideoSys;
	fcyRefPointer<f2dGraphics2D> m_pMainGraph2D;  // ����Ⱦ��

	// ��Դ�����
	std::unordered_map<std::wstring, fcyRefPointer<f2dTexture2D>> m_TexCache;
	std::unordered_map<std::pair<std::wstring, fcyVec2>, fcyRefPointer<f2dFontProvider>> m_FontCache;
	std::unordered_map<std::pair<std::wstring, std::wstring>, fcyRefPointer<f2dFontProvider>> m_SpriteFontCache;

	std::shared_ptr<ILayer> m_GameLayer;
protected:
	void InitEngine();
	void InitFileSystem();
	void SetWindowIcon();
public:
	f2dWindow* GetMainWindow()      { return m_pMainWindow; }
	f2dFileSys* GetFileSys()        { return m_pFileSys; }
	f2dRenderer* GetRenderer()      { return m_pRenderer; }
	f2dRenderDevice* GetRenderDev() { return m_pDev; }
	f2dSoundSys* GetSoundSys()      { return m_pSoundSys; }
	f2dGraphics2D* GetMainGraph2D() { return m_pMainGraph2D; }
	f2dVideoSys* GetVideoSys()      { return m_pVideoSys; }

	// ��ѯ��
	// ���� Path:�ļ������Դ����Ŀ¼��·��
	// ��·��������ʱ�׳�fcyException�쳣
	f2dStream* QueryStream(const std::wstring& Path);
	// ��ѯ����������
	// ���� ResSrc:�ļ������Դ����Ŀ¼��·��
	// ��·��������ʱ��QueryStream�׳�fcyException�쳣
	// ��ͼ�����ʧ��ʱ�׳�fcyException�쳣
	f2dTexture2D* QueryTexture(const std::wstring& ResSrc);
	// ��ѯ����������
	// ���� ResSrc:�ļ������Դ����Ŀ¼��·��
	// ���� Size:�����С
	// ��·��������ʱ��QueryStream�׳�fcyException�쳣
	// ���������ݼ���ʧ��ʱ�׳�fcyException�쳣
	f2dFontProvider* QueryFont(const std::wstring& ResSrc, const fcyVec2& Size);
	// ��ѯ��������������
	// ���� TexSrc:�ļ������Դ����Ŀ¼��·��
	// ���� DefineSrc:�����ļ������Դ����Ŀ¼��·��
	// ��·��������ʱ��QueryStream�׳�fcyException�쳣
	// ���������ݼ���ʧ��ʱ�׳�fcyException�쳣
	f2dFontProvider* QuerySpriteFont(const std::wstring& TexSrc, const std::wstring& DefineSrc);

	// �����������
	fcyRefPointer<f2dSprite> CreateSprite2D(const std::wstring& TexSrc);

	// ����UIPage
	fcyRefPointer<fuiPage> CreateUIPage(const std::wstring& StyleFile, const std::wstring& LayoutFile);

	// ����BGM��ogg��
	fcyRefPointer<f2dSoundBuffer> CreateBGM(const std::wstring& Path);

	// ������Ч��wav��
	fcyRefPointer<f2dSoundBuffer> CreateSoundEffect(const std::wstring& Path);

	// ������Ч
	fcyRefPointer<f2dSoundSprite> CreateSoundSprite(const std::wstring& Path);

	void Run();
	void Abort();
protected:
	fBool OnUpdate(fDouble ElapsedTime, f2dFPSController* pFPSController, f2dMsgPump* pMsgPump);
	fBool OnRender(fDouble ElapsedTime, f2dFPSController* pFPSController);
public:
	AppFrame();
	~AppFrame();
};
