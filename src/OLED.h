
#ifndef OLED_H
#define OLED_H

#include <Arduino.h>

void OLED_begin();

void OLED_update();

bool OLED_click();

uint8_t OLED_getSelection();

void OLED_goHome();

void OLED_programmRun();
void OLED_showIMU();
void OLED_showIR();

void OLED_calibrate();

#endif
