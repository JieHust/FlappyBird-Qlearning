#pragma once
#include "Global.h"

// 游戏框架
class AppFrame :
	public f2dEngineEventListener,
	public fuiResProvider
{
protected:
	// f2d组件
	fcyRefPointer<f2dEngine> m_pEngine;
	f2dWindow* m_pMainWindow;
	f2dFileSys* m_pFileSys;
	f2dRenderer* m_pRenderer;
	f2dRenderDevice* m_pDev;
	f2dSoundSys* m_pSoundSys;
	f2dVideoSys* m_pVideoSys;
	fcyRefPointer<f2dGraphics2D> m_pMainGraph2D;  // 主渲染器

	// 资源对象池
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

	// 查询流
	// 参数 Path:文件相对资源包根目录的路径
	// 当路径不存在时抛出fcyException异常
	f2dStream* QueryStream(const std::wstring& Path);
	// 查询并构造纹理
	// 参数 ResSrc:文件相对资源包根目录的路径
	// 当路径不存在时由QueryStream抛出fcyException异常
	// 当图像加载失败时抛出fcyException异常
	f2dTexture2D* QueryTexture(const std::wstring& ResSrc);
	// 查询并构造字体
	// 参数 ResSrc:文件相对资源包根目录的路径
	// 参数 Size:字体大小
	// 当路径不存在时由QueryStream抛出fcyException异常
	// 当字体数据加载失败时抛出fcyException异常
	f2dFontProvider* QueryFont(const std::wstring& ResSrc, const fcyVec2& Size);
	// 查询并构造纹理字体
	// 参数 TexSrc:文件相对资源包根目录的路径
	// 参数 DefineSrc:定义文件相对资源包根目录的路径
	// 当路径不存在时由QueryStream抛出fcyException异常
	// 当字体数据加载失败时抛出fcyException异常
	f2dFontProvider* QuerySpriteFont(const std::wstring& TexSrc, const std::wstring& DefineSrc);

	// 创建精灵对象
	fcyRefPointer<f2dSprite> CreateSprite2D(const std::wstring& TexSrc);

	// 创建UIPage
	fcyRefPointer<fuiPage> CreateUIPage(const std::wstring& StyleFile, const std::wstring& LayoutFile);

	// 创建BGM（ogg）
	fcyRefPointer<f2dSoundBuffer> CreateBGM(const std::wstring& Path);

	// 创建音效（wav）
	fcyRefPointer<f2dSoundBuffer> CreateSoundEffect(const std::wstring& Path);

	// 创建音效
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
