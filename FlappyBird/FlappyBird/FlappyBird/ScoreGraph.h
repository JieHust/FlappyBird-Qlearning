#pragma once
#include "AppFrame.h"

// ����ͼ��ؼ�
class ScoreGraph :
	public fuiControl
{
public:
	typedef std::map<int, int> DataSource;
protected: // ��ͼ��Դ
	AppFrame* m_pFrame;
	fcyRefPointer<f2dGeometryRenderer> m_GRender;
	const DataSource& m_DataSource;

	int m_MinYValue;
	int m_MaxYValue;
	int m_MinXValue;
	int m_MaxXValue;
public:
	void RefreshData();  // ˢ������Դ
public: // ʵ�ֽӿ�
	void Update(fDouble ElapsedTime);
	void Render(fuiGraphics* pGraph);
public:
	ScoreGraph(fuiPage* pRootPage, const std::wstring& Name, AppFrame* pFrame, const DataSource& DataSource);
	~ScoreGraph();
};
