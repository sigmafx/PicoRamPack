#pragma once
#include <stdio.h>

class Button
{
	public:
		enum class State {
			Off = 0,
			Rising,
			On,
			Hold,
			Falling,
		};
		
	public:
		Button(uint pinBtn);
		Button::State Read();
		Button::State ReadRaw();

	private:
		const uint m_pinBtn;
		const uint32_t m_timeDebounce;
		const uint32_t m_timeHold;
		uint32_t m_timeLast;
		State m_readLast;
		bool m_risen;

		uint32_t Millis();
};
