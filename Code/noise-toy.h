#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>

// UI Input / Output Pin Definitions
#define NT_GPIO_UI_LEFT     RPI_V2_GPIO_P1_38
#define NT_GPIO_UI_RIGHT    RPI_V2_GPIO_P1_10
#define NT_GPIO_UI_SWITCH   RPI_V2_GPIO_P1_32
#define NT_GPIO_UI_BYPASS   RPI_V2_GPIO_P1_08
#define NT_GPIO_UI_LED      RPI_V2_GPIO_P1_36
typedef struct {
    uint32_t led_on : 1;
    uint32_t is_left_down  : 1;
    uint32_t is_right_down : 1;
    uint32_t is_switch_on  : 1;
    uint32_t is_bypass_on  : 1;
    uint32_t was_left_released  : 1;
    uint32_t was_right_released : 1;
    uint32_t was_switch_changed : 1;
    uint32_t was_bypass_changed : 1;
} nt_ui_state_t;
extern nt_ui_state_t _nt_ui_state;
extern uint32_t _nt_ui_duration;

// PWM Output Definitions
#define NT_PWM_PWM0         RPI_V2_GPIO_P1_12
#define NT_PWM_PWM1         RPI_V2_GPIO_P1_33

// Effect Definitions
#define NT_MAX_EFFECT_COUNT 5
typedef uint32_t (*effect_t)(uint32_t);
extern effect_t _nt_effects[NT_MAX_EFFECT_COUNT];
extern uint32_t _nt_effect_frequency;
extern uint32_t _nt_effect_duration;

void noise_toy_init();
void noise_toy_loop();
void noise_toy_set_frequency(uint32_t freqInHz);
bool noise_toy_add_effect(effect_t effect);

uint32_t _nt_read_sample();
void _nt_output_sample(uint32_t sample);
void _nt_update_ui();
