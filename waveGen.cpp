#include "waveGen.h"

void TCA0_init(void) {
    /* set waveform output on PORT A */
    PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTA_gc;
    
    /* disable event counting */
    TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm);

    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV64_gc
                      | TCA_SINGLE_ENABLE_bm;
}

void PORT_init(void) {
    PORTA.DIR |= PIN1_bm;
}

void initializeWaveGen() {
    PORT_init();
    TCA0_init();
}

const uint32_t TICKS_PER_HALF_SECOND = 125000;

void startWave(uint16_t frequency) {
    uint16_t freq = TICKS_PER_HALF_SECOND / frequency;
    TCA0.SINGLE.CMP0BUF = freq;
    TCA0.SINGLE.CMP1BUF = freq;
    TCA0.SINGLE.CTRLB = TCA_SINGLE_CMP0EN_bm
                      | TCA_SINGLE_CMP1EN_bm
                      | TCA_SINGLE_WGMODE_FRQ_gc;
}

void stopWave() {
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;
}