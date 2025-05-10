#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <hardware/gpio.h>
#include <hardware/pwm.h>

#include "../picowalker-defs.h"

#define PW_SPEAKER_PIN 24

#define AUDIO_SYSCLK 150e6
#define AUDIO_CLKDIV 4.0f
#define AUDIO_PWM_RANGE ((1<<16)-1)

#define AUDIO_SAMPLE_RATE 44100 // 44.1 kHz standard sampling

struct audio_queue_s {
    uint16_t pwm_values[127];
    uint16_t durations[127];
    size_t len;
} audio_queue;


/**
 *
 * Notes on the audio
 *
 * Audio comes as (PWM period, duration) pairs, the units of which are unknown,
 * I also don't know the original clock rate but in theory, we can jsut take the 
 * period and duration and scale it to our PWM period and duration.
 *
 * CUrrently, the code is blocking which is awful.
 * Potentially use a timer interrupt to set an interrupt that sets the new period
 * and duration.
 * There needs to be a small gap (10ms seems ok, maybe a bit long) between notes
 * so potentually incorporate that into the timer code.
 *
 */


/*
 * Helper functions
 */

void audio_play_queue() {
    for(size_t i = 0; i < audio_queue.len; i++) {
        pwm_set_gpio_level(PW_SPEAKER_PIN, audio_queue.pwm_values[i]);
        sleep_ms(audio_queue.durations[i]);
        pwm_set_gpio_level(PW_SPEAKER_PIN, 0);
        sleep_ms(10);

    }

    pwm_set_gpio_level(PW_SPEAKER_PIN, 0);
    audio_queue.len = 0;
}

/*
 * Functions needed by core
 */
void pw_audio_init() {
    gpio_init(PW_SPEAKER_PIN);
    gpio_set_function(PW_SPEAKER_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PW_SPEAKER_PIN);

    // counter is allowed to wrap over its maximum range (0 to 2**16-1)
    // this means one wrap takes approx (65535/(sysclk/clkdiv) seconds
    // ~= 0.175ms or 572Hz max freq.
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 2.f);
    pwm_set_gpio_level(PW_SPEAKER_PIN, 0);
    pwm_init(slice_num, &config, true);
    // PWM is now running
}

void pw_audio_play_sound_data(const pw_sound_frame_t *sound_data, size_t sz) {
    /*
    for(size_t i = 0; i < sz; i++) {
        pw_sound_frame_t sf = sound_data[i];

        uint8_t sh = 0xff;
        if(sf.period_idx == 0x7b) {
            sh = sf.info;
            continue;
        }

        if(sf.period_idx == 0x7f) {
            break;
        }

        if(sf.period_idx == 0x7d) {
            printf("[Warn] Encountered `period_idx`=0x7d\n");
            continue;
        }

        uint16_t duration = 0;
        if(sf.period_idx > 0x80) {
            duration = (0x1400 * sf.info/sh);
        } else {
            duration = (0x1400 * sf.info/sh) - 0x14;
        }

        audio_queue.pwm_values[audio_queue.len] = PW_AUDIO_PERIODTAB[sf.period_idx & 0x7f] << 8;
        audio_queue.durations[audio_queue.len] = duration;

        printf("[Debug] Adding period %d, duration %d\n",
            audio_queue.pwm_values[audio_queue.len],
            audio_queue.durations[audio_queue.len]
        );
        audio_queue.len++;

    }

    printf("[Debug] Playing audio queue of length %d\n", audio_queue.len);
    audio_play_queue();
    // TODO: non-blocking
    */
    
}


bool pw_audio_is_playing_sound() {

}

void audio_test_program() {
    pw_audio_init();
    pw_eeprom_init();
    pw_button_init();

    pw_audio_volume = VOLUME_FULL;

    while(1) {
        pw_audio_play_sound(SOUND_DOWSING_FOUND_ITEM);
        sleep_ms(1000);
    }

}


