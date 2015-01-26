#include "ScoreGraph.h"

using namespace std;

ScoreGraph::ScoreGraph(fuiPage* pRootPage, const std::wstring& Name, AppFrame* pFrame, const DataSource& DataSource)
	: fuiControl(pRootPage, Name), m_pFrame(pFrame), m_DataSource(DataSource)
{
	if(FCYFAILED(pFrame->GetRenderer()->CreateGeometryRenderer(&m_GRender)))
		throw fcyException("ScoreGraph::ScoreGraph", "f2dRenderer::CreateGeometryRenderer failed.");

	RefreshData();
}

ScoreGraph::~ScoreGraph()
{}

void ScoreGraph::RefreshData()
{
	m_MinXValue = 0;
	m_MaxXValue = 0;
	m_MinYValue = 0;
	m_MaxYValue = 0;
	for(auto i : m_DataSource)
	{
		if(i.first > m_MaxXValue)
			m_MaxXValue = i.first;
		if(i.second > m_MaxYValue)
			m_MaxYValue = i.second;
	}
}

void ScoreGraph::Update(fDouble ElapsedTime)
{
}

void ScoreGraph::Render(fuiGraphics* pGraph)
{
	fcyVec2 tControlSize(GetWidth(), GetHeight());

	// »æÖÆ±³¾°
	fcyColor tColorArr[] = {
		0x80FFFFFF,
		0x80FFFFFF,
		0x80FFFFFF,
		0x80FFFFFF
	};
	m_GRender->FillRectangle(pGraph->GetGraphics(), fcyRect(fcyVec2(), tControlSize), tColorArr);

	// »æÖÆÇúÏßÍ¼
	m_GRender->SetPenColor(0, 0xfffc7858);
	m_GRender->SetPenColor(1, 0xfffc7858);
	m_GRender->SetPenColor(2, 0xfffc7858);
	m_GRender->SetPenSize(2.f);
	fcyVec2 tLastPoint;
	for(auto i = m_DataSource.begin(); i != m_DataSource.end(); ++i)
	{
		fcyVec2 tCurPoint(
			((float)i->first - m_MinXValue) / ((float)m_MaxXValue - m_MinXValue),
			((float)i->second - m_MinYValue) / ((float)m_MaxYValue - m_MinYValue));
		tCurPoint.x *= tControlSize.x;
		tCurPoint.y = (1.f - tCurPoint.y) * tControlSize.y;

		if(i != m_DataSource.begin())
		{
			m_GRender->DrawLine(pGraph->GetGraphics(), tLastPoint, tCurPoint);
		}

		tLastPoint = tCurPoint;
	}
}
