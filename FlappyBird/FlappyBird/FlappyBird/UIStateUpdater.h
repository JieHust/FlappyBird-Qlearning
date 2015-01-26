#pragma once
#include "Global.h"

// UI组件更新器
class UIStateUpdater
{
public:
	typedef std::function<bool(float, float&, fuiProperty*, fuiControl*)> UpdateFunc;
private:
	fcyRefPointer<fuiPage> m_RootUIPage;
	std::map<fuiControl*, std::map<fuiProperty*, std::pair<float, UpdateFunc>>> m_States;
public:
	// 线性插值器
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
	// 二次函数插值器
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
	// alpha插值器
	static UpdateFunc MakeUpdater_AlphaFader(float Duration, fByte From, fByte To);
public:
	// 向控件的某个属性绑定一个插值器
	void BindUpdater(const std::wstring& ControlName, const std::wstring& PropName, const UpdateFunc& Func);
public:
	// 返回当前的UIPage
	fcyRefPointer<fuiPage> GetPage() { return m_RootUIPage; }
	// 设置当前的UIPage
	void SetPage(const fcyRefPointer<fuiPage>& Page) { m_RootUIPage = Page; }
	// 更新所有插值器
	void Update(float ElapsedTime);
public:
	UIStateUpdater() {}
	UIStateUpdater(const fcyRefPointer<fuiPage>& Page)
		: m_RootUIPage(Page) {}
};
