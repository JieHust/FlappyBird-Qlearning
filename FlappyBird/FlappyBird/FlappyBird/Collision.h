// 碰撞检测
// 摘自fancySTG
#pragma once
#include "Global.h"

/// @brief 碰撞检测功能函数
namespace Collision
{
	/// @brief 线段覆盖测试
	/// @note  需要保证lineA.x < lineA.y 和 lineB.x < lineB.y
	inline bool OverlapTest(const fcyVec2& lineA, const fcyVec2& lineB)
	{
		if(lineA.x < lineB.x)
			return lineA.y > lineB.x;
		else
			return lineB.y > lineA.x;
	};

	/// @brief 圆形相交测试
	inline bool CircleHitTest(const fcyVec2& P1, const float R1, const fcyVec2& P2, const float R2)
	{
		fcyVec2 tOffset = P1 - P2;
		float tRTotal = R1 + R2;

		return tOffset.Length2() < tRTotal * tRTotal;
	}

	/// @brief 无向矩形(AABB)相交测试
	/// @param[in] P1    矩形1的中心位置
	/// @param[in] Size1 矩形1的半边长
	/// @param[in] P2    矩形2的中心位置
	/// @param[in] Size2 矩形2的半边长
	inline bool AABBHitTest(const fcyVec2& P1, const fcyVec2& Size1, const fcyVec2& P2, const fcyVec2& Size2)
	{
		fcyRect tRect1(P1.x - Size1.x, P1.y - Size1.y, P1.x + Size1.x, P1.y + Size1.y);
		fcyRect tRect2(P2.x - Size2.x, P2.y - Size2.y, P2.x + Size2.x, P2.y + Size2.y);

		return tRect1.Intersect(tRect2, NULL);
	}

	/// @brief OBB有向矩形碰撞检测
	/// @param[in] P1     矩形1中心
	/// @param[in] Size1  矩形1半边长
	/// @param[in] Angle1 矩形1旋转
	/// @param[in] P2     矩形2中心
	/// @param[in] Size2  矩形2半边长
	/// @param[in] Angle2 矩形2旋转
	bool OBBHitTest(
		const fcyVec2& P1, const fcyVec2& Size1, const float Angle1,
		const fcyVec2& P2, const fcyVec2& Size2, const float Angle2);

	/// @brief OBB有向矩形与圆碰撞检测
	/// @param[in] P1    矩形中心
	/// @param[in] Size  矩形半边长
	/// @param[in] Angle 矩形旋转
	/// @param[in] P2    圆中心
	/// @param[in] R     圆半径
	bool OBBCircleHitTest(
		const fcyVec2& P1, const fcyVec2& Size, const float Angle,
		const fcyVec2& P2, const float R);

	/// @brief OBB有向矩形与AABB包围盒碰撞检测
	/// @param[in] P     矩形中心
	/// @param[in] Size  矩形半边长
	/// @param[in] Angle 矩形旋转
	/// @param[in] Rect  AABB位置
	bool OBBAABBHitTest(
		const fcyVec2& P, const fcyVec2& Size, const float Angle,
		const fcyRect& Rect);
};
