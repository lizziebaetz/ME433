#include "i2c_master_noint.h"

// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = FRCPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1048576 // use largest wdt
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations


//Any function protos here
void delay(int time);
void heartbeat();
void setPin(unsigned char address, unsigned char reg, unsigned char value);
unsigned char readPin(unsigned char reg);
void blinkA7();

int main() {

    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISAbits.TRISA4 = 0; //set A4 (LED) to output pin
    TRISBbits.TRISB4 = 1; //set B4 (button)  to input pin
    LATAbits.LATA4 = 1; //turn on green LED
    
    
    __builtin_enable_interrupts();
    
    
    //Initialize MCP23017 so GPB0 is input, GPA7 is output
    i2c_master_setup();
    setPin(0b01000000,0x00,0x00); //send write address, send IODIRA address, set A pins as outputs
    setPin(0b01000000,0x01,0xFF); //send write address, send IODIRB address, set B pins as inputs

    
    while(1) {
        
        
        heartbeat(); // pulse green LED to show program is running
        //blinkA7(); // check to make sure controlling GPA7 correctly
        unsigned char data = readPin(0x19); //read from GPIOB
        if(data == 0x00) { // If button is pushed, all GPIOB bits will be low
            setPin(0b01000000,0x0A,0xFF); //Make A7 high, turn on LED
        }
        else {
            setPin(0b01000000,0x0A,0x00); //Make A7 low, turn of LED
        }
        
    }
}

void delay(int time) { 
    _CP0_SET_COUNT(0);
    while(_CP0_GET_COUNT() < time) { // 1 second: time = 24000000
        ;
    }
}

void heartbeat() {
    int time = 2400000;
    LATAbits.LATA4 = 1; //turn on LED
    delay(time);
    LATAbits.LATA4 = 0; //turn off LED
    delay(time);
            
}


void setPin(unsigned char address, unsigned char reg, unsigned char value) {
    i2c_master_start(); // send start bit
    i2c_master_send(address); // send write address
    i2c_master_send(reg); //send register address
    i2c_master_send(value); //set register to desired value
    i2c_master_stop(); // send stop bit
}

unsigned char readPin(unsigned char reg) {
    i2c_master_start(); // send start bit
    i2c_master_send(0b01000000); // send write address
    i2c_master_send(reg); //send register address
    i2c_master_restart(); // restart
    i2c_master_send(0b01000001); //set read address
    unsigned char data = i2c_master_recv(); //receive a byte from slave
    i2c_master_ack(1); //send acknowledgement of byte received
    i2c_master_stop(); // send stop bit
    return data;
}

void blinkA7() {
    
    setPin(0b01000000,0x0A,0b10000000); //Make A7 high, turn on LED
    delay(24000000/2);
    setPin(0b01000000,0x0A,0x00); //Make A7 low, turn off LED
    delay(24000000/2);

}
