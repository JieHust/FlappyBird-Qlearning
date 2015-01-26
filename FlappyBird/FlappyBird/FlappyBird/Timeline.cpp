#include "Timeline.h"

using namespace std;

Timeline::Timeline()
	: m_Data(NULL), m_bPause(true), m_bEditEnable(false), m_bTimeChanged(false),
	m_CurrentTimestamp(0), m_CurrentPos(0)
{}

Timeline::Timeline(TIMESTAMPTYPE Type, bool Paused)
	: m_Data(NULL), m_bPause(Paused), m_bEditEnable(false), m_bTimeChanged(false),
	m_CurrentTimestamp(0), m_CurrentPos(0)
{
	m_Data = new Data(Type);
}

Timeline::Timeline(const Timeline& Org)
	: m_Data(Org.m_Data), m_bPause(Org.m_bPause), m_bEditEnable(Org.m_bEditEnable), m_bTimeChanged(Org.m_bTimeChanged), 
	m_CurrentTimestamp(Org.m_CurrentTimestamp), m_CurrentPos(Org.m_CurrentPos)
{
	if(m_Data)
		m_Data->AddRef();
}

Timeline::~Timeline()
{
	if(m_Data)
		m_Data->Release();
}

Timeline& Timeline::operator=(const Timeline& Right)
{
	if(m_Data)
		m_Data->Release();
	m_Data = Right.m_Data;
	if(m_Data)
		m_Data->AddRef();

	m_bPause = Right.m_bPause;
	m_bEditEnable = Right.m_bEditEnable;
	m_CurrentTimestamp = Right.m_CurrentTimestamp;
	m_CurrentPos = Right.m_CurrentPos;

	return *this;
}

void Timeline::Goto(double tWhere)
{
	if(!m_Data)
		throw fcyException("Timeline::Goto", "Object not inited.");
	if(m_bEditEnable)
		throw fcyException("Timeline::Goto", "Edit enabled.");
	
	auto i = m_Data->GetCollection().begin();
	while(i != m_Data->GetCollection().end() && (*i).first < tWhere)
	{
		++i;
	}

	m_CurrentPos = i - m_Data->GetCollection().begin();
	m_CurrentTimestamp = tWhere;
	m_bTimeChanged = true;
}

void Timeline::StartEditTimeline()
{
	if(!m_Data)
		throw fcyException("Timeline::StartEditTimeline", "Object not inited.");

	m_bEditEnable = true;
}

void Timeline::AddEvent(double Timestamp, const EventType& EventObj)
{
	if(!m_bEditEnable)
		throw fcyException("Timeline::AddEvent", "Edit disabled.");
	
	if(m_Data->IsShared())
	{
		m_Data->Release();
		m_Data = m_Data->Clone();
	}

	m_Data->GetCollection().push_back(pair<double, EventType>(Timestamp, EventObj));
}

void Timeline::RemoveEventAt(double Timestamp)
{
	if(!m_bEditEnable)
		throw fcyException("Timeline::RemoveEventAt", "Edit disabled.");
	
	if(m_Data->IsShared())
	{
		m_Data->Release();
		m_Data = m_Data->Clone();
	}

	remove_if(m_Data->GetCollection().begin(), m_Data->GetCollection().end(),
		[=](const pair<double, EventType>& o)
		{
			return o.first == Timestamp;
		}
	);
}

void Timeline::Clear()
{
	if(!m_bEditEnable)
		throw fcyException("Timeline::Clear", "Edit disabled.");
	
	m_Data->GetCollection().clear();
}

void Timeline::EndEditTimeline()
{
	if(!m_bEditEnable)
		throw fcyException("Timeline::EndEditTimeline", "Edit disabled.");
	
	std::sort(m_Data->GetCollection().begin(), m_Data->GetCollection().end(), 
		[]( const pair<double, EventType>& a,
			const pair<double, EventType>& b)
		{
			return a.first < b.first;
		}
	);
	
	m_bEditEnable = false;

	Goto(m_CurrentTimestamp);
}

void Timeline::Update(double ElapsedTime)
{
	if(!m_Data)
		throw fcyException("Timeline::Update", "Object not inited.");
	if(m_bEditEnable)
		throw fcyException("Timeline::Update", "Edit enabled.");
	if(m_bPause)
		return;

	m_CurrentTimestamp += ElapsedTime;

	auto i = m_Data->GetCollection().begin() + m_CurrentPos;
	while(i != m_Data->GetCollection().end() && i->first <= m_CurrentTimestamp)
	{
		i->second(this);

		if(m_bTimeChanged)
		{
			m_bTimeChanged = false;
			return;
		}

		++i;
	}
	m_CurrentPos = i - m_Data->GetCollection().begin();
}
