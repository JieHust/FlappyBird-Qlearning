#pragma once
#include "Global.h"

// 时间轴计数单位
enum TIMESTAMPTYPE
{
	TIMESTAMPTYPE_FRAME,  // 基于帧步的
	TIMESTAMPTYPE_TIME    // 基于时间步的(秒)
};

// 时间轴
class Timeline
{
public:
	typedef std::function<void(Timeline*)> EventType;
	typedef std::pair<double, EventType>   Event;
	typedef std::vector<Event>             EventCollection;
private:
	// 时间轴数据
	//   使用引用计数方式实现
	//   默认引用计数 = 1
	class Data
	{
	private:
		int             m_cRefCount;      // 引用计数
		TIMESTAMPTYPE   m_TimestampType;  // 时间戳类型
		EventCollection m_EventList;      // 事件列表
	public:
		// 返回时间轴类型
		TIMESTAMPTYPE GetType()const { return m_TimestampType; }
		// 返回数据
		EventCollection& GetCollection() { return m_EventList; }
		// 是否共享
		bool IsShared()const { return (m_cRefCount > 1); }
		// 增加计数
		void AddRef()
		{
			++m_cRefCount;
		}
		// 减少计数
		void Release()
		{
			--m_cRefCount;
			if(m_cRefCount <= 0)
				delete this;
		}
		// 拷贝副本
		//   引用计数 = 1
		Data* Clone()
		{
			Data* tRet = new Data(m_TimestampType);
			tRet->GetCollection() = m_EventList;
			return tRet;
		}
	public:
		Data(TIMESTAMPTYPE Type)
			: m_cRefCount(1), m_TimestampType(Type) {}
	};
private:
	Data* m_Data;

	bool m_bTimeChanged;
	bool m_bPause;                  // 是否暂停
	bool m_bEditEnable;             // 是否可以编辑
	double m_CurrentTimestamp;      // 当前时间戳
	int m_CurrentPos;               // 当前时间轴位置
public:
	TIMESTAMPTYPE GetTimestampType()const
	{
		if(m_Data)
			return m_Data->GetType();
		else
			throw fcyException("Timeline::GetTimestampType", "Object not inited.");
	}

	bool IsPaused()const { return m_bPause; }
	void SetPause(bool v) { m_bPause = v; }

	// 跳到时间
	//   帧模式下对应帧步
	void Goto(double tWhere);

	// 返回当前时间点
	double GetCurrentTimestamp()const { return m_CurrentTimestamp; }

	// 开始编辑时间轴
	void StartEditTimeline();

	// 增加一个事件
	void AddEvent(double Timestamp, const EventType& EventObj);

	// 删除时间片
	void RemoveEventAt(double Timestamp);

	// 清空
	void Clear();

	// 结束编辑时间轴
	void EndEditTimeline();
public:
	// 执行
	void Update(double ElapsedTime);
public:
	Timeline& operator=(const Timeline& Right);
	Timeline();
	Timeline(TIMESTAMPTYPE Type, bool Paused = false);
	Timeline(const Timeline& Org);
	~Timeline();
};

