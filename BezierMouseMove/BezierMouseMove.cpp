#include <windows.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <thread>

/****************************************************
* Copyright (C)	: Liv
* @file			: BezierMouseMove.cpp
* @author		: Liv
* @email		: 1319923129@qq.com / livkaze@gmail.com
* @version		: 1.0
* @date			: 2024/6/22 22:00
****************************************************/

// 随机引擎
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(0.0, 1.0);
std::uniform_real_distribution<> dis2(-1.0, 1.0);

// 移动热键
const DWORD HotKey = VK_SHIFT;

// 贝塞尔曲线计算
POINT bezierCurve(const std::vector<POINT>& ControlPoints, double t) 
{
    int n = ControlPoints.size() - 1;
    POINT Result = { 0, 0 };
    for (int i = 0; i <= n; ++i) 
    {
        double binomialCoeff = 1;
        for (int j = 0; j < i; ++j) 
        {
            binomialCoeff *= (n - j) / (j + 1.0);
        }
        double factor = binomialCoeff * pow(t, i) * pow(1 - t, n - i);
        Result.x += static_cast<LONG>(factor * ControlPoints[i].x);
        Result.y += static_cast<LONG>(factor * ControlPoints[i].y);
    }
    return Result;
}

// 鼠标移动
void MouseMove(const POINT& CurrentPos, const POINT& TargetPos)
{
    // 相对移动
    LONG dx = TargetPos.x - CurrentPos.x;
    LONG dy = TargetPos.y - CurrentPos.y;
    mouse_event(MOUSEEVENTF_MOVE, dx, dy, 0, 0);
}

// 模拟曲线鼠标移动
void SimulateMouseMove(const POINT& StartPos, const POINT& TargetPos)
{
    // 正在移动Flag
    static bool OnMoving = false;
    // 控制点
    static std::vector<POINT> ControlPoints;
    // 总步数
    const int Steps = 100;
    // 当前步数
    static int CurrentStep = 0;
    // 上次移动时间戳
    static auto LastTime = std::chrono::steady_clock::now();
    // 移动间隔时间（ms） & 速度（移动时随机）
    const int DefaultInterval = 7;
    static auto Interval = std::chrono::milliseconds(DefaultInterval);
    // 控制点随机范围
    const float ControlRange = 100;

    if (GetAsyncKeyState(HotKey) & 0x8000)
    {
        // 随机移动时间间隔
        Interval = std::chrono::milliseconds((int)(DefaultInterval + 2 * dis2(gen)));

        // 瞄准死区，即鼠标与目标点距离在指定范围内不启动移动（可选用）
        auto DistanceToCenter = sqrtf(powf(StartPos.x - TargetPos.x, 2) + powf(StartPos.y - TargetPos.y, 2));
        if (DistanceToCenter < 15.f)
        {
            OnMoving = false;
            return;
        }

        if (!OnMoving)
        {
            OnMoving = true;

            // 设置随机控制点
            ControlPoints = { StartPos,
                POINT{LONG(StartPos.x + dis(gen) * ControlRange), LONG(StartPos.y + dis(gen) * ControlRange)},
                POINT{LONG(TargetPos.x - dis(gen) * ControlRange), LONG(TargetPos.y - dis(gen) * ControlRange)},
                TargetPos };

            CurrentStep = 0;

            LastTime = std::chrono::steady_clock::now();
        }
        else
        {
            auto Now = std::chrono::steady_clock::now();
            if (Now - LastTime >= Interval)
            {
                // 利用总步数和当前步数计算t进度
                double t = CurrentStep / static_cast<double>(Steps);
                // 计算下个移动坐标进行间隔计算，用于鼠标相对移动
                auto NextPos = bezierCurve(ControlPoints, t);

                MouseMove(StartPos, NextPos);

                CurrentStep++;
                LastTime = Now;
            }

            if (CurrentStep > Steps)
            {
                // 重置随机控制点
                ControlPoints = { ControlPoints[3],
                    POINT{LONG(ControlPoints[3].x + dis(gen) * ControlRange),LONG(ControlPoints[3].y + dis(gen) * ControlRange)},
                    POINT{LONG(TargetPos.x - dis(gen) * ControlRange),  LONG(TargetPos.y - dis(gen) * ControlRange)},
                    TargetPos };

                CurrentStep = 0;
            }
        }
    }
    else
    {
        if (OnMoving)
        {
            // 取消移动Flag
            OnMoving = false;
        }
    }
}

int main() 
{
    /*
        NOTICE:
        1. 此项目特点是在主线程非阻塞进行鼠标模拟移动，不影响线程中其他代码运行
        2. 使用贝塞尔曲线进行仿真模拟轨迹
        3. 可以自由调整参数数据进行适配
    */

    // 若应用到自瞄上，此坐标直接赋值为游戏窗口中心坐标
    POINT CurrentPos;

    // 模拟主线程
    while (true)
    {
        // 其他运行代码...

        // 若应用到自瞄上，请注释此行
        GetCursorPos(&CurrentPos);

        SimulateMouseMove(CurrentPos, { 500,500 });

        // 其他运行代码...
    }

    return 0;
}
