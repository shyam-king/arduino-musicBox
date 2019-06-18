/*
    AUTHOR: Shyam
    Domain: Embedded and Electronics
    Sub Domain: Embedded Systems
    Functions:  initPins, initTimers, startTimer1, startTimer0, stopTimer1, stopTimer0, main
    Macros: PIN_LOW, PIN_HIGH
    Global variables: pin_speaker, pin_switch[3], freq[8], generating


    Note: The generated wave is centered at 2.5 V, i.e., 2.5 + 2.5sin(x)
*/

#include<avr/interrupt.h>
#include<avr/io.h>

#define PIN_LOW(port, pin) PORT##port &= !(1 << pin)
#define PIN_HIGH(port, pin) PORT##port |= (1 << pin)

const int pin_speaker = 6; //PORTD ~ OC0A
const int pin_switch[] = {0, 1, 2}; //PORTB
const float freq[] = {0, 880, 987.77f, 523.25, 587.33, 659.25, 698.46, 783.99}; //NOTHING, A5, B5, C5, D5, E5, F5, G5
uint8_t generating = 0; // flag to check when the generation of wave should stop

/*
    Function name: initPins
    Input: none
    Output: none
    Logic: Set the pins as output/input
    Example call: initPins()
*/
void initPins() {
    DDRD |= (1 << pin_speaker); //set as output
    DDRB &= !((1 << pin_switch[0]) | (1 << pin_switch[1]) | (1 << pin_switch[2])); //input
}

/*
    Function name: initTimers
    Input: none
    Output: none
    Logic: Initialize the timers used in this program
    Timer 1 is set to CTC mode 
    The TCNT1 is taken as time for sine wave genration

    timer0 is used to generate voltage at pin_speaker corresponding to the sine wave using PWM
    5V from 0 to OCR0A and then low till 255. Therefore o/p voltage is OCR0A / 256 * 5V.
    Example call: initTimers()
*/
void initTimers() {
    TCCR1B |= (1 << WGM12); //CTC MODE
    TCCR0A |= (1 << COM0A1) | (1 << WGM00) | (1 << WGM01); //FAST PWM
}

/*
    Function name: startTimer1
    Input: uint16_t T: time period (not in seconds) this is calculated in generateWave function
    Output: none
    Logic: start the timer1 with 64 prescalar (easier to set frequencies around 100Hz)
    set OCR1A to the given argument (acts like Time period)
    Example call: startTimer1(1600);
*/
void startTimer1(uint16_t T) {
    OCR1A = T;
    TCCR1B |= (1 << CS11) | (1 << CS10);  //64 prescalar
}

/*
    Function name: startTimer0
    Input: none
    Output: none
    Logic: start the timer0 with no prescalar
    Example call: startTimer0();
*/
void startTimer0() {
    TCCR0B |= (1 << CS00);
}

/*
    Function name: stopTimer1
    Input: none
    Output: none
    Logic: stop timer 1
    Example call: stopTimer1()
*/
void stopTimer1() {
    TCCR1B &= !((1 << CS11) | (1 << CS10));
}

/*
    Function name: stopTimer0
    Input: none
    Output: none
    Logic: stop timer 0
    Example call: stopTimer0()
*/
void stopTimer0() {
    TCCR0B = 0;
}

/*
    Function name: setFrequency
    Input: float fhz : frequency in hertz
    Output: none
    Logic: calculate OCR1A and set it
    start timer 1 
    Example call: setFrequency(100);
*/
void setFrequency(float fhz) {
    uint16_t T =  10000.0f / fhz * 25; // for 64 prescalar
    startTimer1(T);
}

/*
    Function name: changeFrequency
    Input: float fhz : frequency in hertz
    Output: none
    Logic: calculate OCR1A and set it
    Example call: changeFrequency(455.5);
*/
void changeFrequency(float fhz) {
    uint16_t T =  10000.0f / fhz * 25; // for 64 prescalar
    OCR1A = T;
}

/*
    Function name: generateWave
    Input: none
    Output: none
    Logic: a sine wave generated of the given frequency
    T is calculated using the given frequency ( this value is set as OCR1A )
        T is the number of ticks for 2pi phase angle
    Timer1 is started (which produces output at pin_speaker)
    the sine function is 2.5V + 2.5V * sin(2 * PI * ft)
    Now ft = TCNT1/T (why? Magic:p :: TCNT1/T is the phase angle / 2pi)
    Since op = OCR0A/256 * 5, OCR0A is correspondingly set
    Example call: generateWave(100);
*/
void generateWave() {
    changeFrequency(freq[generating]); //to change OCR1A according to the selected frequency
    float op =  2.5 + 2.5* sin(TCNT1 * 6.28 / OCR1A); //output voltage
    OCR0A = op * 51; //51 is approx = 256/5
}

/*
    Function name: initGeneration
    Input: none
    Output: none
    Logic: to be called before generateWave()
    set initial frequency to 0 (no sound) and start timer 0
    Example call: initGeneration()
*/
void initGeneration() {
    setFrequency(freq[0]);
    OCR0A = 127; //2.5 V (initial voltage)
    startTimer0();
}

/*
    Function name: readSwitches
    Input: none
    Output: uint8_t : the combined value of three switches in portb (0,1,2)
    if no switch is pressed, produces 0
    switch 1 alone in "on" produces 1
    switch 2 alone in "on" produces 2
    switches (1,2) in "on" produces 3
    switch 3 in "on" produces 4
    switches (3,1) in "on" produces 5
    switches (2,3) in "on" produces 6
    switches (1,2,3) in "on" produces 7
    Logic: the first three bits of PINB correspond to the three switches therefore 
    PINB & 7 results in the output as described above
    Example call: uint8_t value = readSwitches();
*/
uint8_t readSwitches() {
    uint8_t r = 0;
    r = (PINB & 7);
    return r;
}

int main() {
    initPins();
    initTimers();
    sei(); //enable global interrupts
    Serial.begin(9600);
    initGeneration();
    while (1) {
        generating = readSwitches();
        generateWave();
    }
    stopTimer0();
    stopTimer1();
}
