#include "noise-toy.h"

effect_t _nt_effects[NT_MAX_EFFECT_COUNT];  // Effects present in the chain
nt_ui_state_t _nt_ui_state;                 // Current state of the ui
uint32_t _nt_effect_frequency;              // Targeted output frequency in hz
uint32_t _nt_effect_duration;               // execution time of effects in us
uint32_t _nt_ui_duration;                   // execution time of ui in us

void noise_toy_init() {

    // Init UI GPIO
    bcm2835_gpio_fsel(NT_GPIO_UI_LEFT, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(NT_GPIO_UI_RIGHT, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(NT_GPIO_UI_SWITCH, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(NT_GPIO_UI_BYPASS, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(NT_GPIO_UI_LED, BCM2835_GPIO_FSEL_OUTP);

    // Init PWM Output
    bcm2835_gpio_fsel(NT_PWM_PWM0, BCM2835_GPIO_FSEL_ALT5 );
    bcm2835_gpio_fsel(NT_PWM_PWM1, BCM2835_GPIO_FSEL_ALT0 );
    bcm2835_pwm_set_clock(BCM2835_PWM_CLOCK_DIVIDER_2); // 9.6MHz PWM
    bcm2835_pwm_set_mode(0, 1, 1);
    bcm2835_pwm_set_range(0, 64);   // 6 Bit at 150KHz (=9.6MHz / 64)
    bcm2835_pwm_set_mode(1, 1, 1);
    bcm2835_pwm_set_range(1, 64);   // 6 Bit at 150KHz (=9.6MHz / 64)

    // Init SPI 
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);   
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);           
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);            
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW); 
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64);
    // NOTE: Div64 Means 4MHz wich is too high for 5V supply, yet alone 2.7V Operation maybe drop to 2MHz
    // TODO: Test with lower freq

}

void noise_toy_loop();
bool noise_toy_add_effect(effect_t effect);

/**
 * Read a single sample from the ADC. Execution time of this function includes
 * the adc conversion time and sample transport time.
 */
uint32_t _nt_read_sample() {
    uint8_t mosi[3] = {0x01, 0x00, 0x00};
    uint8_t miso[3] = {0x00};
    bcm2835_spi_transfernb(mosi, miso, 3);
    return miso[2] + ((miso[1] & 0x0F) << 8);
    // NOTE: ADC only returns 12Bit according to datasheet
    // TODO: Check output from ADC
}

/**
 * Output a 12 Bit sample via the pwm output. Execution time includes convertion
 * time and register update time. 
 */
void _nt_output_sample(uint32_t sample) {
    bcm2835_pwm_set_data(1, sample & 0x3F);
    bcm2835_pwm_set_data(0, sample >> 6);
}

/**
 * Update the state of the ui digital twin. Sets led state and reads button 
 * presses. Execution time includes i/o comm aswell as application logic. 
 */
void _nt_update_ui() {

    // update led state
    bcm2835_gpio_write(NT_GPIO_UI_LED, _nt_ui_state.led_on);

    // read inputs into tmp vars
    uint32_t leftDown  = bcm2835_gpio_lev(NT_GPIO_UI_LEFT);
    uint32_t rightDown = bcm2835_gpio_lev(NT_GPIO_UI_RIGHT);
    uint32_t switchOn  = bcm2835_gpio_lev(NT_GPIO_UI_SWITCH);
    uint32_t bypassOn  = bcm2835_gpio_lev(NT_GPIO_UI_BYPASS);

    // update logic state
    _nt_ui_state.was_left_released  = _nt_ui_state.is_left_down  && !leftDown;
    _nt_ui_state.was_right_released = _nt_ui_state.is_right_down && !rightDown;  
    _nt_ui_state.was_right_released = _nt_ui_state.is_switch_on != switchOn;
    _nt_ui_state.was_bypass_changed = _nt_ui_state.is_bypass_on != bypassOn;
    _nt_ui_state.is_left_down  = leftDown  && true;
    _nt_ui_state.is_right_down = rightDown && true; 
    _nt_ui_state.is_switch_on  = switchOn  && true;
    _nt_ui_state.is_bypass_on  = bypassOn  && true;

}

