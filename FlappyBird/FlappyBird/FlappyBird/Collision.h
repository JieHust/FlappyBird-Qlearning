// ��ײ���
// ժ��fancySTG
#pragma once
#include "Global.h"

/// @brief ��ײ��⹦�ܺ���
namespace Collision
{
	/// @brief �߶θ��ǲ���
	/// @note  ��Ҫ��֤lineA.x < lineA.y �� lineB.x < lineB.y
	inline bool OverlapTest(const fcyVec2& lineA, const fcyVec2& lineB)
	{
		if(lineA.x < lineB.x)
			return lineA.y > lineB.x;
		else
			return lineB.y > lineA.x;
	};

	/// @brief Բ���ཻ����
	inline bool CircleHitTest(const fcyVec2& P1, const float R1, const fcyVec2& P2, const float R2)
	{
		fcyVec2 tOffset = P1 - P2;
		float tRTotal = R1 + R2;

		return tOffset.Length2() < tRTotal * tRTotal;
	}

	/// @brief �������(AABB)�ཻ����
	/// @param[in] P1    ����1������λ��
	/// @param[in] Size1 ����1�İ�߳�
	/// @param[in] P2    ����2������λ��
	/// @param[in] Size2 ����2�İ�߳�
	inline bool AABBHitTest(const fcyVec2& P1, const fcyVec2& Size1, const fcyVec2& P2, const fcyVec2& Size2)
	{
		fcyRect tRect1(P1.x - Size1.x, P1.y - Size1.y, P1.x + Size1.x, P1.y + Size1.y);
		fcyRect tRect2(P2.x - Size2.x, P2.y - Size2.y, P2.x + Size2.x, P2.y + Size2.y);

		return tRect1.Intersect(tRect2, NULL);
	}

	/// @brief OBB���������ײ���
	/// @param[in] P1     ����1����
	/// @param[in] Size1  ����1��߳�
	/// @param[in] Angle1 ����1��ת
	/// @param[in] P2     ����2����
	/// @param[in] Size2  ����2��߳�
	/// @param[in] Angle2 ����2��ת
	bool OBBHitTest(
		const fcyVec2& P1, const fcyVec2& Size1, const float Angle1,
		const fcyVec2& P2, const fcyVec2& Size2, const float Angle2);

	/// @brief OBB���������Բ��ײ���
	/// @param[in] P1    ��������
	/// @param[in] Size  ���ΰ�߳�
	/// @param[in] Angle ������ת
	/// @param[in] P2    Բ����
	/// @param[in] R     Բ�뾶
	bool OBBCircleHitTest(
		const fcyVec2& P1, const fcyVec2& Size, const float Angle,
		const fcyVec2& P2, const float R);

	/// @brief OBB���������AABB��Χ����ײ���
	/// @param[in] P     ��������
	/// @param[in] Size  ���ΰ�߳�
	/// @param[in] Angle ������ת
	/// @param[in] Rect  AABBλ��
	bool OBBAABBHitTest(
		const fcyVec2& P, const fcyVec2& Size, const float Angle,
		const fcyRect& Rect);
};
