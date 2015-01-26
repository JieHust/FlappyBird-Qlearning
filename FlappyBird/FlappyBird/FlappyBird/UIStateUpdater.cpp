#include "UIStateUpdater.h"

using namespace std;

UIStateUpdater::UpdateFunc UIStateUpdater::MakeUpdater_AlphaFader(float Duration, fByte From, fByte To)
{
	float K = ((float)(To - From)) / Duration;

	return 
		[=](float ElapsedTime, float& TotalTime, fuiProperty* pProp, fuiControl* pControl)
		{
			wstring tStr;
			fcyColor tColor;

			TotalTime += ElapsedTime;
			if(TotalTime >= Duration)
				tColor = fcyColor(To, 0, 0, 0);
			else
				tColor = fcyColor((fByte)(From + K * TotalTime), 0, 0, 0);
			
			fuiPropertyAccessorHelper<fcyColor>::DefaultGetter(tStr, &tColor);
			pProp->Set(tStr);

			return TotalTime < Duration;
		};
}

void UIStateUpdater::BindUpdater(const std::wstring& ControlName, const std::wstring& PropName, const UpdateFunc& Func)
{
	assert(m_RootUIPage != nullptr);

	fuiControl* pControl = m_RootUIPage->GetControl(ControlName);
	fuiProperty* pProp = pControl->QueryPropertyInterface(PropName);
	assert(pProp != nullptr);

	m_States[pControl][pProp] = std::pair<float, UpdateFunc>(0.f, Func);
}

void UIStateUpdater::Update(float ElapsedTime)
{
	auto i = m_States.begin();
	while(i != m_States.end())
	{
		auto j = i->second.begin();
		while(j != i->second.end())
		{
			if(false == j->second.second(ElapsedTime, j->second.first, j->first, i->first))
				j = i->second.erase(j);
			else
				++j;
		}

		++i;
	}
}
