#include "ws2812b.h"

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
    
    // Initialize timer and output pin
    ws2812b_setup();
    

    wsColor color[4]; // color array for 4 LEDs
    
        // Test setting colors
//    color[0] = HSBtoRGB(30.0,1.0,0.25);
//    color[1] = HSBtoRGB(120.0,1.0,0.25);
//    color[2] = HSBtoRGB(210.0,1.0,0.25);
//    color[3] = HSBtoRGB(300.0,1.0,0.25);
//  
    // create array of hues to cycle through on each LED
    int numHues = 2000; // set resolution
    float hues[4][numHues];
    int i=0; int j=0; //for loops
    
    for (i = 0; i<4; i++) {
        for (j = 0; j < numHues ; j++){
            if ((i*30 + j*(360.0/numHues)) < 360){
                hues[i][j] = i*30 + j*(360.0/numHues);
            }
            else { // when reach hue value 360, start bck at 0 
                hues[i][j] = i*30 + j*(360.0/numHues) - 360;
            }
        }
    }
 
    while(1) {
        
        // loop through different hue values to make rainbow
        
           for (i=0; i<4; i++){      
               for (j = 0; j<numHues; j++){   
                    color[i] = HSBtoRGB(hues[i][j],.75,0.15);                               
                    ws2812b_setColor(color,4);
                    
                 }    
               
            }
        

        
         heartbeat();

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