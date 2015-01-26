#include "Collision.h"

using namespace std;

bool Collision::OBBHitTest(
	const fcyVec2& P1, const fcyVec2& Size1, const float Angle1,
	const fcyVec2& P2, const fcyVec2& Size2, const float Angle2)
{
	// 计算出矩形的4个顶点
	fcyVec2 tFinalPos[2][4] =
	{
		{
			fcyVec2(-Size1.x, -Size1.y),
			fcyVec2( Size1.x, -Size1.y),
			fcyVec2( Size1.x,  Size1.y),
			fcyVec2(-Size1.x,  Size1.y)
		},
		{
			fcyVec2(-Size2.x, -Size2.y),
			fcyVec2( Size2.x, -Size2.y),
			fcyVec2( Size2.x,  Size2.y),
			fcyVec2(-Size2.x,  Size2.y)
		}
	};

	float tSin, tCos;
	{
		tSin = sin(Angle1);
		tCos = cos(Angle1);

		for(int i = 0; i<4; i++)
		{
			tFinalPos[0][i].RotationSC(tSin, tCos);
			tFinalPos[0][i] += P1;
		}
	}
	{
		tSin = sin(Angle2);
		tCos = cos(Angle2);

		for(int i = 0; i<4; i++)
		{
			tFinalPos[1][i].RotationSC(tSin, tCos);
			tFinalPos[1][i] += P2;
		}
	}

	// 处理两个矩形的四条轴
	for(int i = 0; i<2; i++)
	{
		fcyVec2 tAxis[2] = 
		{
			tFinalPos[i][1] - tFinalPos[i][0],
			tFinalPos[i][2] - tFinalPos[i][1]
		};

		// 单位化轴向
		tAxis[0].Normalize();
		tAxis[1].Normalize();

		// 轴的投影线段
		fcyVec2 tAxisLine[2] = 
		{
			fcyVec2(tFinalPos[i][0] * tAxis[0], tFinalPos[i][1] * tAxis[0]),
			fcyVec2(tFinalPos[i][1] * tAxis[1], tFinalPos[i][2] * tAxis[1])					
		};

		// 对每一条轴向
		for(int j = 0; j<2; j++)
		{
			// 计算另一矩形在轴上的投影产生的线段
			fcyVec2 tProjLine(tFinalPos[1-i][0] * tAxis[j], tFinalPos[1-i][1] * tAxis[j]);
			if(tProjLine.y < tProjLine.x)
				std::swap(tProjLine.x, tProjLine.y);
			for(int k = 2; k<4; k++)
			{
				float v = tFinalPos[1-i][k] * tAxis[j];
				if(v < tProjLine.x)
					tProjLine.x = v;
				if(v > tProjLine.y)
					tProjLine.y = v;
			}

			// 进行覆盖测试
			if(!OverlapTest(tAxisLine[j], tProjLine))
			{
				// 分离轴定律，当有一条轴的投影不相交即无碰撞
				return false;
			}
		}
	}
	return true;
}

bool Collision::OBBCircleHitTest(
	const fcyVec2& P1, const fcyVec2& Size, const float Angle,
	const fcyVec2& P2, const float R)
{
	// 计算出矩形的4个顶点
	fcyVec2 tFinalPos[4] = 
	{
		fcyVec2(-Size.x, -Size.y),
		fcyVec2( Size.x, -Size.y),
		fcyVec2( Size.x,  Size.y),
		fcyVec2(-Size.x,  Size.y)
	};

	float tSin = sin(Angle), tCos = cos(Angle);

	// 变换
	for(int i = 0; i<4; i++)
	{
		tFinalPos[i].RotationSC(tSin, tCos);
		tFinalPos[i] += P1;
	}

	// 计算两条轴向
	fcyVec2 tAxis[2] = 
	{
		tFinalPos[1] - tFinalPos[0],
		tFinalPos[2] - tFinalPos[1]
	};

	// 轴长度
	float tAxisLen[2] = 
	{
		tAxis[0].Length(),
		tAxis[1].Length(),
	};

	// 单位化轴向
	if(tAxisLen[0] != 0.f)
		tAxis[0] /= tAxisLen[0];
	if(tAxisLen[1] != 0.f)
		tAxis[1] /= tAxisLen[1];

	// 计算各边中心参考点在轴向上的投影
	float tProjValue[2] = 
	{
		((tFinalPos[1]+tFinalPos[0])/2.f) * tAxis[0],
		((tFinalPos[2]+tFinalPos[1])/2.f) * tAxis[1]
	};

	// 计算圆在轴上的投影
	float tCircleCenterProjValue[2] = 
	{
		P2 * tAxis[0],
		P2 * tAxis[1]
	};

	// 检查边碰撞
	if(fabs(tCircleCenterProjValue[0] - tProjValue[0]) < tAxisLen[0] / 2.f)
	{
		if(fabs(tCircleCenterProjValue[1] - tProjValue[1]) < tAxisLen[1] / 2.f + R)
			return true;
		else
			return false;
	}
	else if(fabs(tCircleCenterProjValue[1] - tProjValue[1]) < tAxisLen[1] / 2.f)
	{
		if(fabs(tCircleCenterProjValue[0] - tProjValue[0]) < tAxisLen[0] / 2.f + R)
			return true;
		else
			return false;
	}

	// 检查四个角
	float tDist2 = R;
	tDist2 *= tDist2;
	for(int i = 0; i<4; i++)
	{
		if((tFinalPos[i] - P2).Length2() < tDist2)
			return true;
	}

	return false;
}

bool Collision::OBBAABBHitTest(
	const fcyVec2& P, const fcyVec2& Size, const float Angle,
	const fcyRect& Rect)
{
	// 计算出OBB矩形的4个顶点
	fcyVec2 tFinalPos[2][4] =
	{
		{
			fcyVec2(-Size.x, -Size.y),
			fcyVec2( Size.x, -Size.y),
			fcyVec2( Size.x,  Size.y),
			fcyVec2(-Size.x,  Size.y)
		},
		{
			Rect.a,
			fcyVec2(Rect.b.x, Rect.a.y),
			Rect.b,
			fcyVec2(Rect.a.x, Rect.b.y)
		}
	};

	float tSin, tCos;
	{
		tSin = sin(Angle);
		tCos = cos(Angle);

		for(int i = 0; i<4; i++)
		{
			tFinalPos[0][i].RotationSC(tSin, tCos);
			tFinalPos[0][i] += P;
		}
	}

	// 处理两个矩形的四条轴
	for(int i = 0; i<2; i++)
	{
		fcyVec2 tAxis[2] = 
		{
			tFinalPos[i][1] - tFinalPos[i][0],
			tFinalPos[i][2] - tFinalPos[i][1]
		};

		// 单位化轴向
		tAxis[0].Normalize();
		tAxis[1].Normalize();

		// 轴的投影线段
		fcyVec2 tAxisLine[2] = 
		{
			fcyVec2(tFinalPos[i][0] * tAxis[0], tFinalPos[i][1] * tAxis[0]),
			fcyVec2(tFinalPos[i][1] * tAxis[1], tFinalPos[i][2] * tAxis[1])					
		};

		// 对每一条轴向
		for(int j = 0; j<2; j++)
		{
			// 计算另一矩形在轴上的投影产生的线段
			fcyVec2 tProjLine(tFinalPos[1-i][0] * tAxis[j], tFinalPos[1-i][1] * tAxis[j]);
			if(tProjLine.y < tProjLine.x)
				std::swap(tProjLine.x, tProjLine.y);
			for(int k = 2; k<4; k++)
			{
				float v = tFinalPos[1-i][k] * tAxis[j];
				if(v < tProjLine.x)
					tProjLine.x = v;
				if(v > tProjLine.y)
					tProjLine.y = v;
			}

			// 进行覆盖测试
			if(!OverlapTest(tAxisLine[j], tProjLine))
			{
				// 分离轴定律，当有一条轴的投影不相交即无碰撞
				return false;
			}
		}
	}
	return true;
}
