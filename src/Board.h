/*
Copyright (C) 2019 Bengt Martensson.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see http://www.gnu.org/licenses/.
 */

/**
 * This class serves as an HAL (Hardware Abstraction Layer).
 * All access to the hardware should go through this class.
 * (In particular, using digital[Read,Write] and ::pinMode is prohibited
 * (exception: code that runs exclusively on the host).
 *
 * It is a singleton class (since there is only one board), instantiated
 * automatically to one of its subclasses.
 */

#pragma once

#include "IrSignal.h"

class Board {
protected:
    Board() {};

public:
    static void delayMicroseconds(microseconds_t);

    virtual void writeLow();
    virtual void writeHigh();

    virtual void writeLow(pin_t pin) { digitalWrite(pin, LOW); };
    virtual void writeHigh(pin_t pin) { digitalWrite(pin, HIGH); };

    void setPinMode(pin_t pin, uint8_t mode) { pinMode(pin, mode); };
    bool readDigital(pin_t pin) { return digitalRead(pin) == HIGH; };

    virtual pin_t getPwmPin() const;

    static Board* getInstance() {
        return instance;
    };

    static const unsigned long microsPerTick = 50UL; // was USECPERTICK

    virtual void checkValidSendPin(pin_t pin __attribute__((unused))) {/* TODO */};

    /**
     * Constant indicating no or invalid pin.
     */
    static constexpr pin_t NO_PIN = 255U;

    /**
     * Default duty cycle to use. Do not confuse with IrSignal::defaultDutyCycle.
     */
    static constexpr dutycycle_t defaultDutyCycle = 40;

    /**
     * Start the periodic ISR sampler routine. Called from IrReceiveSampler.
     */
    virtual void enableSampler(pin_t pin __attribute__((unused))) {
        TIMER_CONFIG_NORMAL();
        TIMER_ENABLE_INTR();
        TIMER_RESET();
    }

    /**
     * Turn off sampler routine.
     */
    virtual void disableSampler() {
        TIMER_DISABLE_INTR();
    }

    /**
     * Start PWM, making output active.
     * @param pin
     * @param frequency
     * @param dutyCycle
     */
    void enablePwm(pin_t pin, frequency_t frequency, dutycycle_t dutyCycle) {
        checkValidSendPin(pin);
        TIMER_DISABLE_INTR();
        //pinMode(getPin(), OUTPUT);
        //writeLow();
        TIMER_CONFIG_HZ(frequency, dutyCycle);
    }

    /**
     * Turn off PWM.
     */
    void disablePwm() {
    }

    void sendPwmMark(microseconds_t time) {
        TIMER_ENABLE_PWM(); // supposed to turn on
        delayMicroseconds(time);
        TIMER_DISABLE_PWM();
    }

    /**
     * TODO
     */
    virtual void TIMER_RESET() {};

protected:
    /**
     * Start periodic sampling routine.
     */
    virtual void TIMER_ENABLE_INTR() = 0;

    /**
     * Turn off periodic interrupts.
     * @return
     */
    virtual void TIMER_DISABLE_INTR() = 0;

    /**
     * Configure hardware PWM, but do not enable it.
     * @return
     */
    virtual void TIMER_CONFIG_HZ(frequency_t hz, dutycycle_t dutyCycle = defaultDutyCycle) = 0;

    /**
     * Disables the PWM configuration.
     * @return
     */
    virtual void TIMER_CONFIG_NORMAL() = 0;

    /**
     * Start PWM output.
     * @return
     */
    virtual void TIMER_ENABLE_PWM() = 0;

    /**
     * Turn off PWM output.
     * @return
     */
    virtual void TIMER_DISABLE_PWM() = 0;

public:
    static constexpr pin_t defaultPwmPin();

    static constexpr bool hasHardwarePwm();

private:
    static Board* instance;
};

///////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
// CPU Frequency
//
#ifndef F_CPU
#error F_CPU not defined. This must be fixed.
#endif

#if ! defined(ARDUINO)

// Assume that we compile a test version, to be executed on the host, not on a board.
#include "boards/NoBoard.h"

#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
/////////////////// Arduino Uno, Nano etc (previously default clause)
//#define IR_USE_TIMER1   // tx = pin 9
#define IR_USE_TIMER2     // tx = pin 3

#include "boards/ATmega328P.h"

#elif defined(__AVR_ATmega2560__)
/////////////////// Mega 2560 etc //////////////////////////////////
//#define IR_USE_TIMER1   // tx = pin 11
#define IR_USE_TIMER2     // tx = pin 9
//#define IR_USE_TIMER3   // tx = pin 5
//#define IR_USE_TIMER4   // tx = pin 6
//#define IR_USE_TIMER5   // tx = pin 46

#include "boards/ATmega2560.h"

#elif defined(__AVR_ATmega32U4__)
// Can be either Leonardo, Micro, or Teensy 2.
#if defined(ARDUINO_AVR_LEONARDO) || defined(ARDUINO_AVR_MICRO)
#define IR_USE_TIMER1     // tx = 9
//#define IR_USE_TIMER2   // tx = 3, currently broken

#include "boards/ATmega32U4.h"

#else
//// Teensy 2.0 ?
#error Unsupported board. Please fix boarddefs.h.
#endif

#elif defined(__AVR_ATmega4809__)
// ATMega4809, like Uno WiFi Rev 2, Nano Every
#define IR_USE_TIMER1     //  tx = pin 6
//#define IR_USE_TIMER2     // Not yet implemented tx = ?

#include "boards/ATmega4809.h"

#elif defined(__SAM3X8E__) || defined(__SAM3X8H__)
// Arduino Due

  //#define IR_USE_PWM0   // tx = pin 34
  //#define IR_USE_PWM1   // tx = pin 36
  //#define IR_USE_PWM2   // tx = pin 38
  //#define IR_USE_PWM3   // tx = pin 40
  //#define IR_USE_PWM4   // tx = pin 9
  //#define IR_USE_PWM5   // tx = pin 8
  #define IR_USE_PWM6   // tx = pin 7
  //#define IR_USE_PWM7   // tx = pin 6

  #define IR_USE_TC3    // Use timer clock 3.
  //#define IR_USE_TC4    // Use timer clock 4.
  //#define IR_USE_TC5    // Use timer clock 5.

//  #define IR_USE_SAM // Used to correct code where needed to be compatible with the Due.
//  #define IR_USE_DUE // Used to correctly map pins. (The idea being there might be more than one Arduino model based on SAM cores.)

#include "boards/due.h"

#elif defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__)
// Teensy 3.0 / Teensy 3.1 / 3.2

#include "boards/teensy3x.h"

#elif defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)

#include "boards/Sam.h"

///////////////////// ESP32
#elif defined(ESP32)

#include "boards/Esp32.h"

///////////////////// ESP8266
#elif defined(ESP8266)

#include "boards/Esp8266.h"

#else

#error Your board is currently not supported. Please add it to boarddefs.h.

#endif

#ifdef HAS_HARDWARE_PWM

inline constexpr bool Board::hasHardwarePwm() {
    return true;
}
inline constexpr pin_t Board::defaultPwmPin() { return PWM_PIN; };
inline pin_t Board::getPwmPin() const { return PWM_PIN; };
inline void Board::writeLow() { digitalWrite(getPwmPin(), LOW); };
inline void Board::writeHigh() { digitalWrite(getPwmPin(), HIGH); };

#else

inline constexpr bool Board::hasHardwarePwm() { return false; };
inline constexpr pin_t Board::defaultPwmPin() { return NO_PIN; };
;inline pin_t Board::getPwmPin() const { return PWM_PIN; };
inline void Board::writeLow() { digitalWrite(getPwmPin(), LOW); };
inline void Board::writeHigh() { digitalWrite(getPwmPin(), HIGH); };

#endif