#pragma once
#include "AppFrame.h"

// 分数图表控件
class ScoreGraph :
	public fuiControl
{
public:
	typedef std::map<int, int> DataSource;
protected: // 绘图资源
	AppFrame* m_pFrame;
	fcyRefPointer<f2dGeometryRenderer> m_GRender;
	const DataSource& m_DataSource;

	int m_MinYValue;
	int m_MaxYValue;
	int m_MinXValue;
	int m_MaxXValue;
public:
	void RefreshData();  // 刷新数据源
public: // 实现接口
	void Update(fDouble ElapsedTime);
	void Render(fuiGraphics* pGraph);
public:
	ScoreGraph(fuiPage* pRootPage, const std::wstring& Name, AppFrame* pFrame, const DataSource& DataSource);
	~ScoreGraph();
};
