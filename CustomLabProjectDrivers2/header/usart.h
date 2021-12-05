#ifndef USART_H
#define USART_H

#define FOSC 1843200
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

#define TRANSMISSION_REQUEST 0X01
#define TRANSMISSION_ACKNOWLEDGEMENT 0xFF

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>


void USART_Init(unsigned int ubrr){
    //Set Baud Rate
    UBRR0H = (unsigned char) (ubrr >> 8);
    UBRR0L = (unsigned char) (ubrr);

    //Set Frame Format: 8data, 2stop bits
    UCSR0C = (1 << USBS0) | (3 << UCSZ00);

    //Enable Receiver and Transmitter
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
}

//Waits until the data has been received
void USART_WaitForReceive(){
    while ( !(UCSR0A & (1 << RXC0)) ){}
    return;
}


//Waits until the data has been sent
void USART_WaitForTransmit(){
    while ( !(UCSR0A & (1 << TXC0)) ){}
    return;
}

//Checks the Receive flag
unsigned char USART_CheckTransmitFlag(){
    return (UCSR0A & (1 << TXC0));
}


//Checks the receive flag
unsigned char USART_CheckReceiveFlag(){
    return (UCSR0A & (1 << RXC0));
}

void USART_FlusDataBuffer(){
    unsigned char data_dump;
    while (UCSR0A & (1 << RXC0)) {
        data_dump = UDR0;
    }
    return;
}

void USART_Transmit(unsigned char data){
    // Wait until the data buffer is empty
    while ( !(UCSR0A & (1 << UDRE0)) ){}

    // Write the data
    UDR0 = data;
}

void USART_off(){
    UCSR0B = (0 << RXEN0) | (0 << TXEN0);
}

unsigned char USART_Receive(){
    while( !(UCSR0A & (1 << RXC0)) ){}

    return UDR0;
}
#endif