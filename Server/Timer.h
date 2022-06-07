#pragma once

class CTimer
{
private:
	bool			  m_IsStopped{};
					  
	LARGE_INTEGER	  m_PerformanceFrequency{};
	LARGE_INTEGER	  m_LastPerformanceCounter{};
	LARGE_INTEGER	  m_CurrentPerformanceCounter{};
					  
	float			  m_ElapsedTime{};
	float			  m_FPSElapsedTime{};
					  
	static const UINT m_MaxSampleCount{ 60 };
	UINT			  m_SampleCount{};
					  
	float			  m_FrameTime[m_MaxSampleCount]{};
	UINT			  m_FrameRate{};
	UINT			  m_FrameCount{};

public:
	CTimer();
	~CTimer() = default;

	void Start();
	void Stop();
	void Reset();
	void Tick(float LockFPS);

	float GetElapsedTime() const;
	UINT GetFrameRate(LPTSTR Title, UINT Characters) const;
};
