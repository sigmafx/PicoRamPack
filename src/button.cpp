#include "button.h"

#include <pico/stdlib.h>

Button::Button(uint pinBtn) :
	m_pinBtn(pinBtn),
	m_timeDebounce(10),
	m_timeHold(1000)
{
	m_risen = false;
	m_readLast = State::Off;

	// Set the button pin to input pull up
	// Button is expected to be grounded to be ON
	gpio_init(m_pinBtn);
	gpio_pull_up(m_pinBtn);
}

Button::State Button::ReadRaw()
{
	return gpio_get(m_pinBtn) ? State::Off : State::On;
}

Button::State Button::Read()
{
	uint32_t timeNow = Millis();
	State ret = State::Off;

	if(ReadRaw() == State::On)
	{
		if(m_readLast == State::Off)
		{
			// Transition from OFF to ON, not debounced
			m_timeLast = Millis();
		}
		else
		{
			if((timeNow - m_timeLast) > m_timeDebounce)
			{
				// Debounced
				if(m_risen)
				{
					if((timeNow - m_timeLast) > m_timeHold)
					{
						// Button ON and HOLD
						ret = State::Hold;
					}
					else
					{
						// Button ON but not HOLD
						ret = State::On;
					}
				}
				else
				{
					// Btn RISING
					ret = State::Rising;
					m_risen = true;
				}
			}
		}

		// Keep track of where we are
		m_readLast = State::On;
	}
	else
	{
		// Button not pressed, reset everything
		m_timeLast = 0;		
		m_risen = false;

		// Keep track of where we are
		m_readLast = State::Off;
	}

	return ret;
}

uint32_t Button::Millis()
{
	return time_us_32() / 1000;
}
