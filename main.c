#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

const uint8_t segmentdigit[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

void CHIPinit() {//7 Segment display and 595 chip set up
  DDRH |= (1 << DDH6); // Pin 9 Data
  DDRB |= (1 << DDB4) | (1 << DDB5); //Pin 10 Latch | Pin 11 Clock
  PORTH &= ~(1 << PH6);
  PORTB &= ~(1 << PB4);
  DDRA |= (1 << DDA0) | (1 << DDA1) | (1 << DDA2) | (1 << DDA3);// (4 Digit 7 Segment Display Digit Select Pins)
  PORTA &= ~((1 << PA0) | (1 << PA1) | (1 << PA2) | (1 << PA3)); // D22 | D23 | D24 | D25 (4 Digit 7 Segment Display Digit Select Pins)
}

void cleardigits() {//Clears 7 segment displays
    PORTA |= (1 << PA0) | (1 << PA1) | (1 << PA2) | (1 << PA3);
}

void setregister(uint8_t digit) {//Sets the 595 register to display a digit on a 7 segment display
  PORTB &= ~(1 << PB4);

  for(int8_t i = 7; i >= 0; i--){
    if(digit & (1 << i)) {
      PORTH |= (1 << PH6);
    } else {
      PORTH &= ~(1 << PH6);
    }

    PORTB |= (1 << PB5);
    PORTB &= ~(1 << PB5);

  }
  PORTB |= (1 << PB4);
}

uint8_t HTsensorreadbyte() {//Reads the bits and bytes sent by the humidity sensor and puts it into data
  uint8_t data = 0;

  for(uint8_t i = 0; i < 8; i++) { 
    while (!(PINB & (1 << PB6)));
    _delay_us(35);
    data <<= 1;

    if(PINB & (1 << PB6)) {
      data |= 1;
      while(PINB & (1 << PB6));
    }
  }
  return data;
}

uint8_t HTsensorread(uint8_t *temp, uint8_t *humidity) {//Reads the humidity sensor data by waking sensor up, strating transmission and placing each decoded byte into a variable
  uint8_t humidity_int, humidity_dec, temp_int, temp_dec, checksum = 0;

  DDRB |= (1 << DDB6);
  PORTB &= ~(1 << PB6);
  _delay_ms(18);
  PORTB |= (1 << PB6);
  _delay_us(40);
  DDRB &= ~(1 << PB6);

  while(PINB & (1 << PB6));   
  while(!(PINB & (1 << PB6)));
  while(PINB & (1 << PB6));

  humidity_int = HTsensorreadbyte();
  humidity_dec = HTsensorreadbyte();
  temp_int = HTsensorreadbyte();
  temp_dec = HTsensorreadbyte();
  checksum = HTsensorreadbyte();

  if((uint8_t)(humidity_int + humidity_dec + temp_int + temp_dec) == checksum) {
    *temp = temp_int;
    *humidity = humidity_int;

    return 1;
  }
  return 0;
}

int main(void) {
  CHIPinit();
  uint8_t temp_int, humidity_int;

  while(1) {
    cleardigits();
    HTsensorread(&temp_int, &humidity_int);

    uint8_t temp10 = temp_int / 10;
    uint8_t temp1 = temp_int % 10;
    uint8_t humidity10 = humidity_int / 10;
    uint8_t humidity1 = humidity_int % 10;

    for(uint16_t i = 0; i < 600; i++) {//Displays temp for 4 seconds

      cleardigits();
      setregister(segmentdigit[temp10]);
      PORTA &= ~(1 << PA0);
      _delay_ms(2);

      cleardigits();
      setregister(segmentdigit[temp1]);
      PORTA &= ~(1 << PA1);
      _delay_ms(2);

      cleardigits();
      setregister(0b0111001);
      PORTA &= ~(1 << PA2);
      _delay_ms(2);

    }

    for(uint16_t i = 0; i < 600; i++) {//Displays humidity for 4 seconds
      cleardigits();
      setregister(segmentdigit[humidity10]);
      PORTA &= ~(1 << PA0);
      _delay_ms(2);

      cleardigits();
      setregister(segmentdigit[humidity1]);
      PORTA &= ~(1 << PA1);
      _delay_ms(2);

      cleardigits();
      setregister(0b01110110);
      PORTA &= ~(1 << PA2);
      _delay_ms(2);
    }
  }

  return 0;

}