#ifndef JOYSTICK_H
#define JOYSTICK_H

#define HORIZONTAL_CHANNEL 1
#define VERTICAL_CHANNEL 0

void ADC_init(){
    ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

void ADC_change_channel(unsigned char channel){
    switch(channel){
        case 0:
            ADMUX = 0xC0;
            break;
        case 1:
            ADMUX = 0xC1;
            break;
        default:
            ADMUX = 0x00;
            break;
    }
    delay_ms(4);
}

#endif