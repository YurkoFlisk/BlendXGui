#include "Clock.h"

using namespace std::chrono;

Clock::Clock(milliseconds start, milliseconds inc)
	: m_inc(inc), active(false)
{
	m_timer.setInterval(start);
	m_timer.setSingleShot(true);
	m_timer.setTimerType(Qt::PreciseTimer);
	connect(&m_timer, &QTimer::timeout, [this]{
		stop();
		emit timeout();
	});
}

bool Clock::isTimeout() const
{
	return remainingTime() == milliseconds::zero();
}

void Clock::pause()
{
	if (!active)
		return;
	auto newTime = m_timer.remainingTimeAsDuration() + m_inc;
	m_timer.stop();
	m_timer.setInterval(newTime);
	active = false;
}

void Clock::stop()
{
	m_timer.stop();
	active = false;
}

void Clock::resume()
{
	if (active || isTimeout()) // Do not activate in this case
		return;
	m_timer.start();
	active = true;
}

milliseconds Clock::remainingTime() const
{
	if (active)
		return m_timer.remainingTimeAsDuration();
	else
		return m_timer.intervalAsDuration();
}
