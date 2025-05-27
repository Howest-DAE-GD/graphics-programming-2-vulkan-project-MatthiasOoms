#include "Timer.h"
#include <GLFW/glfw3.h>

Timer::Timer()
{
	m_SecondsPerCount = 1;
}

void Timer::Reset()
{
	double currentTime = glfwGetTime();

	m_BaseTime = currentTime;
	m_PreviousTime = currentTime;
	m_StopTime = 0;
	m_FPSTimer = 0;
	m_FPSCount = 0;
	m_IsStopped = false;

}

void Timer::Start()
{
	double startTime = glfwGetTime();

	if (m_IsStopped)
	{
		m_PausedTime += (startTime - m_StopTime);

		m_PreviousTime = startTime;
		m_StopTime = 0;
		m_IsStopped = false;
	}
}

void Timer::Update()
{
    if (m_IsStopped)
    {
        m_FPS = 0;
        m_ElapsedTime = 0.0f;
        m_TotalTime = static_cast<float>((m_StopTime - m_PausedTime) - m_BaseTime);
        return;
    }

    double currentTime = glfwGetTime();
    m_CurrentTime = currentTime;

    m_ElapsedTime = static_cast<float>(m_CurrentTime - m_PreviousTime);
    m_PreviousTime = m_CurrentTime;

    if (m_ElapsedTime < 0.0f)
        m_ElapsedTime = 0.0f;

    if (m_ForceElapsedUpperBound && m_ElapsedTime > m_ElapsedUpperBound)
    {
        m_ElapsedTime = m_ElapsedUpperBound;
    }

    m_TotalTime = static_cast<float>(m_CurrentTime - m_PausedTime - m_BaseTime);

    // FPS LOGIC
    m_FPSTimer += m_ElapsedTime;
    ++m_FPSCount;
    if (m_FPSTimer >= 1.0f)
    {
        m_dFPS = static_cast<float>(m_FPSCount) / m_FPSTimer;
        m_FPS = m_FPSCount;
        m_FPSCount = 0;
        m_FPSTimer = 0.0f;
    }
}

void Timer::Stop()
{
	if (!m_IsStopped)
    {
        m_StopTime = glfwGetTime();
        m_IsStopped = true;
    }
}