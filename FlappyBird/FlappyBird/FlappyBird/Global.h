#pragma once

#include <cassert>

// STL
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <deque>
#include <stack>
#include <map>
#include <algorithm>
#include <memory>
#include <set>

// fancy2D
#include <fcyIO/fcyStream.h>
#include <fcyIO/fcyBinaryHelper.h>
#include <fcyMisc/fcyStopWatch.h>
#include <fcyMisc/fcyRandom.h>
#include <fcyMisc/fcyStringHelper.h>
#include <fcyParser/fcyXml.h>
#include <fcyUIBase/fuiRes.h>
#include <fcyUIBase/fuiControl.h>
#include <fcyUIBase/fuiPage.h>
#include <fcyUIBase/fuiStyle.h>
#include <fcyUIControl/fuiButton.h>
#include <fcyUIControl/fuiCursor.h>
#include <fcyUIControl/fuiEditBox.h>
#include <fcyUIControl/fuiImageBox.h>
#include <fcyUIControl/fuiIME.h>
#include <fcyUIControl/fuiParticlePool.h>
#include <fcyUIControl/fuiParticleEmitter.h>
#include <fcyUIControl/fuiLabel.h>
#include <fcyUIControl/fuiLabelEx.h>
#include <f2d.h>

#define GAMESIZE_W    800  // 游戏宽度
#define GAMESIZE_H    600  // 游戏高度
#define GAMEFRAMERATE 60   // 帧率

// 层结构
class ILayer
{
private:
	bool m_bReceiveMsg;
public:
	bool IsReceiveMsg()const { return m_bReceiveMsg; }
	void SetReceiveMsg(bool bValue) { m_bReceiveMsg = bValue; }
protected:
	virtual void OnMsg(const f2dMsg& Msg) {}
	virtual void OnUpdate(float ElapsedTime) {}
	virtual void OnRender(float ElapsedTime) {}
public:
	void DoMsg(const f2dMsg& Msg)
	{
		if(m_bReceiveMsg)
			OnMsg(Msg);
	}
	void Update(float ElapsedTime)
	{
		OnUpdate(ElapsedTime);
	}
	void Render(float ElapsedTime)
	{
		OnRender(ElapsedTime);
	}
public:
	ILayer()
		: m_bReceiveMsg(true) {}
	virtual ~ILayer() {}
};
