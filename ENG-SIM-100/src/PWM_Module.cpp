#include <Arduino.h>
#include "../include/PWM_Module.h"

void PWM_Module::Stop() {
}

const int chPin1 = 19; 
const int chPin2 = 23;
const int chPin3 = 25; 


const int freq = 5; /* 5 KHz */
const int resolution = 12;
const int MAX_DUTY_CYCLE = (int)(pow(2, resolution) - 1);

const int pwmChannel1 = 0;
const int pwmChannel2 = 2;
const int pwmChannel3 = 4;

#define AVERAGE_SAMPLES 16

uint8_t idx = 0;

uint16_t avg_chan1[AVERAGE_SAMPLES];
uint16_t avg_chan2[AVERAGE_SAMPLES];
uint16_t avg_chan3[AVERAGE_SAMPLES];

const int ADC_RESOLUTION = 4095; /* 12-bit */

void  PWM_Module::Setup() {
    pinMode(35, ANALOG);
    pinMode(33, ANALOG);
    pinMode(34, ANALOG);

    ledcSetup(pwmChannel1, freq, resolution);    
    ledcSetup(pwmChannel2, freq, resolution);
    ledcSetup(pwmChannel3, freq, resolution);

    ledcAttachPin(chPin1, pwmChannel1);    
    ledcAttachPin(chPin2, pwmChannel2);
    ledcAttachPin(chPin3, pwmChannel3); 
}

void PWM_Module::Loop() {    

    uint32_t ch1Acc = 0;
    uint32_t ch2Acc = 0;
    uint32_t ch3Acc = 0;

    for(uint8_t idx = 0; idx < AVERAGE_SAMPLES; ++idx) {
        ch1Acc += analogRead(35) / 4;
        ch2Acc += analogRead(33) / 4;
        ch3Acc += analogRead(34) / 4;
    }

    uint16_t freq1 = ch1Acc / AVERAGE_SAMPLES;
    uint16_t freq2 = ch2Acc / AVERAGE_SAMPLES;
    uint16_t freq3 = ch3Acc / AVERAGE_SAMPLES;

    ledcWrite(pwmChannel1, MAX_DUTY_CYCLE / 2);
    ledcWrite(pwmChannel2, MAX_DUTY_CYCLE / 2);
    ledcWrite(pwmChannel3, MAX_DUTY_CYCLE / 2);
    
    ledcWriteTone(pwmChannel1, freq1);
    ledcWriteTone(pwmChannel2, freq2);
    ledcWriteTone(pwmChannel3, freq3);


    if(m_nextPrint < millis()) {
        m_pConsole->println("CH1=" + String(freq1) + " CH2=" + String(freq2) + " CH3=" + String(freq3));
        m_nextPrint = millis() + 1000;
    }
}