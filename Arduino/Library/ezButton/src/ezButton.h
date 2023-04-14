/*
 * Copyright (c) 2019, ArduinoGetStarted.com. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of the ArduinoGetStarted.com nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ARDUINOGETSTARTED.COM "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ARDUINOGETSTARTED.COM BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ezButton_h
#define ezButton_h

#include <Arduino.h>

#define COUNT_FALLING 0
#define COUNT_RISING  1
#define COUNT_BOTH    2

class ezButton
{
	private:
		int btnPin;
		unsigned long debounceTime;
		unsigned long count;
		int countMode;

		int previousSteadyState;  // the previous steady state from the input pin, used to detect pressed and released event
		int lastSteadyState;      // the last steady state from the input pin
		int lastFlickerableState; // the last flickerable state from the input pin

		unsigned long lastDebounceTime; // the last time the output pin was toggled

	public:
		ezButton(int pin);
		ezButton(int pin, int mode);
		void setDebounceTime(unsigned long time);
		int getState(void);
		int getStateRaw(void);
		bool isPressed(void);
		bool isReleased(void);
		void setCountMode(int mode);
		unsigned long getCount(void);
		void resetCount(void);
		void loop(void);
};

#endif
