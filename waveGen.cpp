#include "waveGen.h"

void TCA0_init(void) {
    // Set waveform output on PORT A. This is for convenience.
    // Should be configurable for general use.
    PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTA_gc;

    // Set PIN1 of the configured port to output. This is
    // just because this pin was convenient. Should be
    // updated to a configurable pin for general use.
    PORTA.DIR |= PIN1_bm;
    
    // Not 100% sure if this is ncessary. Took it from some
    // AVR example code.
    TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm);

    // Using the 64x clock divider because it's the default
    // that Arduino uses (for compatibility with Uno) and
    // it works just fine for our usecase
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV64_gc
                      | TCA_SINGLE_ENABLE_bm;
}

void initializeWaveGen() {
    TCA0_init();
}

// Hardcoded math:
// 16 MHz / 64 (clock divider) = 250,000 ticks per second
// 250,000 ticks / 2 (two ticks per wave period) = 125,000 periods per second
// Max freq 125 KHz, way more than we need and good for nyquist
const uint32_t TICKS_PER_HALF_SECOND = 125000;

void startWave(uint16_t frequency) {
    uint16_t freq = TICKS_PER_HALF_SECOND / frequency;
    // CMP0 must be set because it controls the frequency
    TCA0.SINGLE.CMP0BUF = freq;
    // Using CMP1 for additional pin output because the pin
    // was convenient. Other pins were not.
    // Not 100% necessary to set this, but keeps the outputs
    // synced. Any other value would work except the first wave
    // would be slightly shifted. Would never hear the difference.
    TCA0.SINGLE.CMP1BUF = freq;
    // Enable CMP0 and CMP1. Set to waveform generation mode. This
    // mode causes the chip to automatically change a particular pin
    // (controlled by PORTMUX) to change from High to Low every time
    // the counter reaches the value in CMP0.
    // The pins for TCA0 in single mode (not split mode) are:
    //    CMP0 = Port X Pin 0
    //    CMP1 = Port X Pin 1
    //    CMP2 = Port X Pin 2
    // where X is the port set in PORTMUX.TCAROUTEA. Pins 3, 4, and 5
    // are only used in Split mode.
    TCA0.SINGLE.CTRLB = TCA_SINGLE_CMP0EN_bm
                      | TCA_SINGLE_CMP1EN_bm
                      | TCA_SINGLE_WGMODE_FRQ_gc;
}

void stopWave() {
    // The more obvious way to stop the wave would be to
    // disable the timer entirely in CTRLA. Unfortunately,
    // the 4809 uses the same register to enable TCA and TCB
    // and the Arduino API uses TCB for PWM and millis. So
    // we disable sound by changing the TCA mode.
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;
}