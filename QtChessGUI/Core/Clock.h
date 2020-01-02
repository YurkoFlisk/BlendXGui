#pragma once

#include <QTimer>

class Clock
	: public QObject
{
	Q_OBJECT

	using milliseconds = std::chrono::milliseconds; 
public:
	Clock(milliseconds start = std::chrono::minutes(5),
		milliseconds inc = milliseconds::zero());
	
	// Whether there is timeout
	bool isTimeout() const;
	// Pauses the clock
	void pause();
	// Stop the clock
	void stop();
	// Resumes (or starts) the clock
	void resume();
	// Get time remaining (correct, includes time spent for current turn)
	milliseconds remainingTime() const;
signals:
	void timeout();
private:
	// Timer. Remaining clock time is held THERE as interval or
	// remaining time (depending on whether clock is active)
	QTimer m_timer;
	// Time increment
	milliseconds m_inc;
	// Whether the clock is active, i.e. ticks now
	bool active;
};
