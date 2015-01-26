#pragma once
#include "Global.h"

// UI���������
class UIStateUpdater
{
public:
	typedef std::function<bool(float, float&, fuiProperty*, fuiControl*)> UpdateFunc;
private:
	fcyRefPointer<fuiPage> m_RootUIPage;
	std::map<fuiControl*, std::map<fuiProperty*, std::pair<float, UpdateFunc>>> m_States;
public:
	// ���Բ�ֵ��
	template<class T>
	static UpdateFunc MakeUpdater_ValueUpdater(float Duration, T From, T To)
	{
		float K = ((float)(To - From)) / Duration;

		return 
			[=](float ElapsedTime, float& TotalTime, fuiProperty* pProp, fuiControl* pControl)
			{
				std::wstring tStr;
				T tValue;

				TotalTime += ElapsedTime;
				if(TotalTime >= Duration)
					tValue = To;
				else
					tValue = (T)(From + K * TotalTime);

				fuiPropertyAccessorHelper<T>::DefaultGetter(tStr, &tValue);
				pProp->Set(tStr);

				return TotalTime < Duration;
			};
	}
	// ���κ�����ֵ��
	template<class T>
	static UpdateFunc MakeUpdater_ValueUpdater_Pow(float Duration, T From, T To)
	{
		float K = ((float)(To - From));

		return 
			[=](float ElapsedTime, float& TotalTime, fuiProperty* pProp, fuiControl* pControl)
			{
				std::wstring tStr;
				T tValue;

				TotalTime += ElapsedTime;
				if(TotalTime >= Duration)
					tValue = To;
				else
				{
					float X = TotalTime / Duration;
					tValue = (T)(From + K * X * X);
				}

				fuiPropertyAccessorHelper<T>::DefaultGetter(tStr, &tValue);
				pProp->Set(tStr);

				return TotalTime < Duration;
			};
	}
	template<class T>
	static UpdateFunc MakeUpdater_ValueUpdater_PowInv(float Duration, T From, T To)
	{
		float K = ((float)(To - From));

		return 
			[=](float ElapsedTime, float& TotalTime, fuiProperty* pProp, fuiControl* pControl)
			{
				std::wstring tStr;
				T tValue;

				TotalTime += ElapsedTime;
				if(TotalTime >= Duration)
					tValue = To;
				else
				{
					float X = TotalTime / Duration;
					tValue = (T)(From + K * (2*X - X*X));
				}

				fuiPropertyAccessorHelper<T>::DefaultGetter(tStr, &tValue);
				pProp->Set(tStr);

				return TotalTime < Duration;
			};
	}
	// alpha��ֵ��
	static UpdateFunc MakeUpdater_AlphaFader(float Duration, fByte From, fByte To);
public:
	// ��ؼ���ĳ�����԰�һ����ֵ��
	void BindUpdater(const std::wstring& ControlName, const std::wstring& PropName, const UpdateFunc& Func);
public:
	// ���ص�ǰ��UIPage
	fcyRefPointer<fuiPage> GetPage() { return m_RootUIPage; }
	// ���õ�ǰ��UIPage
	void SetPage(const fcyRefPointer<fuiPage>& Page) { m_RootUIPage = Page; }
	// �������в�ֵ��
	void Update(float ElapsedTime);
public:
	UIStateUpdater() {}
	UIStateUpdater(const fcyRefPointer<fuiPage>& Page)
		: m_RootUIPage(Page) {}
};
