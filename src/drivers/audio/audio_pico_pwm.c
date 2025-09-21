#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include "hardware/timer.h"

#include "picowalker-defs.h"

#define PW_SPEAKER_PIN 2

#define AUDIO_ALARM_NUM 1
#define AUDIO_SYSCLK 150e6
#define AUDIO_CLKDIV 4.0f
#define AUDIO_PWM_RANGE ((1<<16)-1)

#define AUDIO_SAMPLE_RATE 44100 // 44.1 kHz standard sampling

struct audio_queue_s {
    uint16_t pwm_values[127];
    uint16_t durations[127];
    size_t len;
    size_t head;
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


void audio_irq_callback() {
    // Ack this interrupt
    hw_clear_bits(&timer_hw->intr, 1u<<AUDIO_ALARM_NUM);

    // Check if we should continue
    if(audio_queue.head >= audio_queue.len) {
        audio_queue.len = 0;
        pwm_set_gpio_level(PW_SPEAKER_PIN, 0);
        return;
    }

    // Set note value
    pwm_set_wrap(pwm_gpio_to_slice_num(PW_SPEAKER_PIN), audio_queue.pwm_values[audio_queue.head]);
    pwm_set_gpio_level(PW_SPEAKER_PIN, audio_queue.pwm_values[audio_queue.head]/2);

    // Set timer for note duration and callback
    hw_set_bits(&timer_hw->inte, 1u<<AUDIO_ALARM_NUM);
    uint irq_num = timer_hardware_alarm_get_irq_num(timer_hw, AUDIO_ALARM_NUM);
    irq_set_exclusive_handler(irq_num, audio_irq_callback);
    irq_set_enabled(irq_num, true);

    uint64_t target = timer_hw->timerawl + (audio_queue.durations[audio_queue.head]*1000);
    timer_hw->alarm[AUDIO_ALARM_NUM] = (uint32_t)target;
    
    // Increment note counter
    audio_queue.head++;
}

/*
 * Helper functions
 */

void audio_play_queue() {
    /*
    for(size_t i = 0; i < audio_queue.len; i++) {

        // Set wrap to the value given by Pokewalker data
        pwm_set_wrap(pwm_gpio_to_slice_num(PW_SPEAKER_PIN), audio_queue.pwm_values[i]);
        // Set top to control volume
        // TODO: Change to 1/3 duty for medium volume
        pwm_set_gpio_level(PW_SPEAKER_PIN, audio_queue.pwm_values[i]/2);

        // Wait for note to finish
        sleep_ms(audio_queue.durations[i]);

        // Small delay after note finished to make it distinct
        pwm_set_gpio_level(PW_SPEAKER_PIN, 0);
        sleep_ms(10);

    }
        */

    // Lazy way of starting the queue
    audio_queue.head = 0;
    audio_irq_callback();

    // No more sound
    //pwm_set_gpio_level(PW_SPEAKER_PIN, 0);
    //audio_queue.len = 0;
    //audio_queue.head = 0;
}

/*
 * Functions needed by core
 */
void pw_audio_init() {
    gpio_init(PW_SPEAKER_PIN);
    gpio_set_function(PW_SPEAKER_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PW_SPEAKER_PIN);

    pwm_config config = pwm_get_default_config();
    // TODO: Modify the clockdiv to make 
    pwm_config_set_clkdiv(&config, 8.f);
    pwm_set_gpio_level(PW_SPEAKER_PIN, 0);
    pwm_init(slice_num, &config, true);
    // PWM is now running
}

void pw_audio_play_sound_data(const pw_sound_frame_t *sound_data, size_t sz) {
    if(pw_audio_is_playing_sound()) return;
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

        //printf("[Debug] Adding period %d, duration %d\n",
        //    audio_queue.pwm_values[audio_queue.len],
        //    audio_queue.durations[audio_queue.len]
        //);
        audio_queue.len++;

        audio_queue.pwm_values[audio_queue.len] = 0;
        audio_queue.durations[audio_queue.len] = 20;
        audio_queue.len++;

    }

    printf("[Debug] Playing audio queue of length %d\n", audio_queue.len);
    audio_play_queue();
    
}


bool pw_audio_is_playing_sound() {
    return audio_queue.len > 0;
}

void audio_test_program() {
    pw_audio_init();
    pw_eeprom_init();
    pw_button_init();

    gpio_init(18);
    gpio_set_dir(18, GPIO_OUT);

    pw_audio_volume = VOLUME_FULL;

    while(1) {
        pw_audio_play_sound(SOUND_DOWSING_FOUND_ITEM);
        while(pw_audio_is_playing_sound()) {
            gpio_put(18, 1);
            sleep_ms(1000);
            gpio_put(18, 0);
            sleep_ms(1000);
        }
        sleep_ms(5000);
    }

}

