#pragma once
#include "Global.h"

// ʱ���������λ
enum TIMESTAMPTYPE
{
	TIMESTAMPTYPE_FRAME,  // ����֡����
	TIMESTAMPTYPE_TIME    // ����ʱ�䲽��(��)
};

// ʱ����
class Timeline
{
public:
	typedef std::function<void(Timeline*)> EventType;
	typedef std::pair<double, EventType>   Event;
	typedef std::vector<Event>             EventCollection;
private:
	// ʱ��������
	//   ʹ�����ü�����ʽʵ��
	//   Ĭ�����ü��� = 1
	class Data
	{
	private:
		int             m_cRefCount;      // ���ü���
		TIMESTAMPTYPE   m_TimestampType;  // ʱ�������
		EventCollection m_EventList;      // �¼��б�
	public:
		// ����ʱ��������
		TIMESTAMPTYPE GetType()const { return m_TimestampType; }
		// ��������
		EventCollection& GetCollection() { return m_EventList; }
		// �Ƿ���
		bool IsShared()const { return (m_cRefCount > 1); }
		// ���Ӽ���
		void AddRef()
		{
			++m_cRefCount;
		}
		// ���ټ���
		void Release()
		{
			--m_cRefCount;
			if(m_cRefCount <= 0)
				delete this;
		}
		// ��������
		//   ���ü��� = 1
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
	bool m_bPause;                  // �Ƿ���ͣ
	bool m_bEditEnable;             // �Ƿ���Ա༭
	double m_CurrentTimestamp;      // ��ǰʱ���
	int m_CurrentPos;               // ��ǰʱ����λ��
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

	// ����ʱ��
	//   ֡ģʽ�¶�Ӧ֡��
	void Goto(double tWhere);

	// ���ص�ǰʱ���
	double GetCurrentTimestamp()const { return m_CurrentTimestamp; }

	// ��ʼ�༭ʱ����
	void StartEditTimeline();

	// ����һ���¼�
	void AddEvent(double Timestamp, const EventType& EventObj);

	// ɾ��ʱ��Ƭ
	void RemoveEventAt(double Timestamp);

	// ���
	void Clear();

	// �����༭ʱ����
	void EndEditTimeline();
public:
	// ִ��
	void Update(double ElapsedTime);
public:
	Timeline& operator=(const Timeline& Right);
	Timeline();
	Timeline(TIMESTAMPTYPE Type, bool Paused = false);
	Timeline(const Timeline& Org);
	~Timeline();
};

