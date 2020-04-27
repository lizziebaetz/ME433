#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include "spi.h"
#include<math.h>

#define NUMPTS 500 //# points in wave form

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

static volatile double SineWave[NUMPTS];
static volatile double TriWave[NUMPTS];

//Any function protos here
void delay();
void initSPI();
unsigned char spi_io(unsigned char o);
void makeTriWave();
void makeSineWave();


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
    
    initSPI();
    
    __builtin_enable_interrupts();
    
    // make 16bit 

//    makeTriWave();
//    makeSineWave();
    
    while(1) {
        // write one byte over SPI1
//        
//        makeTriWave();
//        makeSineWave();
        
        // Send bytes over SPI1 to create tri wave
//         unsigned char cb = 1;
//         unsigned short pb;
//         unsigned int j;
//        unsigned char ca = 0;
//        unsigned short pa;
//        
 
//         for(j=0;j<NUMPTS;++j) {
//             
//            int voltage2 = TriWave[j]*4095; //scale tri wave to range 0 to max volts
//
//            pb = cb<<15;
//            pb = pb|(0b111<<12);
//            pb = pb|voltage2;
//
//            LATAbits.LATA0 = 0; //Bring CS low
//            spi_io(pb>>8); //write the high bits
//            spi_io(pb); //write the low bits
//            LATAbits.LATA0 = 1; //Bring CS high
//
//           
//            
//            int voltage1 = SineWave[j]*4095; //scale sine wave to range 0 to 3.3 v
//
//            pa = ca<<15;
//            pa = pa|(0b111<<12);
//            pa = pa|voltage1;
//
//
//            LATAbits.LATA0 = 0; //Bring CS low
//            spi_io(pa>>8); //write the high bits
//            spi_io(pa); //write the low bits
//            LATAbits.LATA0 = 1; //Bring CS high
//            _CP0_SET_COUNT(0);
//            while (_CP0_GET_COUNT() < 16000) { // 1 second = 24000000
//            ;
//            }
//            
//          }
        
            makeTriWave();
            makeSineWave();
    

    }
          
}

void delay() { //delay half second
        _CP0_SET_COUNT(0);
        while (_CP0_GET_COUNT() < 24000000) { // 1 second = 24000000
            ;
        }
}

// initialize SPI1
void initSPI() {
    // Pin B14 has to be SCK1
    
    // Turn off analog pins
    ANSELA = 0; //turns off all analog pins on A ports 
    // Make an output pin for CS
    TRISAbits.TRISA0 = 0; // Sets A0 as output 
    LATAbits.LATA0 = 1;
    
    // Set SDO1 to pin A1
    RPA1Rbits.RPA1R = 0b0011;

    // Set B5 as SDI1 
    SDI1Rbits.SDI1R = 0b0001;

    // setup SPI1
    SPI1CON = 0; // turn off the spi module and reset it
    SPI1BUF; // clear the rx buffer by reading from it
    SPI1BRG = 1000; // 1000 for 24kHz, 1 for 12MHz; // baud rate to 10 MHz [SPI1BRG = (48000000/(2*desired))-1]
    SPI1STATbits.SPIROV = 0; // clear the overflow bit
    SPI1CONbits.CKE = 1; // data changes when clock goes from hi to lo (since CKP is 0)
    SPI1CONbits.MSTEN = 1; // master operation
    SPI1CONbits.ON = 1; // turn on spi 
}

unsigned char spi_io(unsigned char o) {
    SPI1BUF = o;
    while (!SPI1STATbits.SPIRBF) { //wait to receive the byte
        ;
    }
    return SPI1BUF;
}


void makeTriWave() {
  
    // Create yvals for one period of a triangle wave ranging 0 to 1
    int i;
    for (i=0; i<NUMPTS; ++i) {
      if (i<NUMPTS/2) {
          TriWave[i] = i*2./NUMPTS;
        }
      else {
          TriWave[i] = (1-(i-NUMPTS/2.)*(2./NUMPTS));
      }
    }
    
//    // Send bytes over SPI1
      unsigned char cb = 1;
      unsigned short pb;
      unsigned int j;
      int voltage2;
      for(j=0;j<NUMPTS;++j) {
        voltage2 = TriWave[j]*4095;

                pb = cb<<15;
                pb = pb|(0b111<<12);
                pb = pb|voltage2;
                
                LATAbits.LATA0 = 0; //Bring CS low
                spi_io(pb>>8); //write the high bits
                spi_io(pb); //write the low bits
                LATAbits.LATA0 = 1; //Bring CS high
                
        _CP0_SET_COUNT(0);
        while (_CP0_GET_COUNT() < 24000) { // 1 second = 24000000
            ;
        }
      }

}


void makeSineWave() {
   
    // Create yvals for two periods of a sine wave ranging 0 to 1
    int i;
    for (i=0; i<NUMPTS; ++i) {
        SineWave[i] = sin(i*4*M_PI/NUMPTS)/2+0.5;       
    }
    
    // Send bytes over SPI1
    unsigned char ca = 0;
    unsigned short pa;
    unsigned short j;
        for(j=0;j<NUMPTS;++j) {
            int voltage1 = SineWave[j]*4095;
            
                pa = ca<<15;
                pa = pa|(0b111<<12);
                pa = pa|voltage1;


                LATAbits.LATA0 = 0; //Bring CS low
                spi_io(pa>>8); //write the high bits
                spi_io(pa); //write the low bits
                LATAbits.LATA0 = 1; //Bring CS high
        _CP0_SET_COUNT(0);
        while (_CP0_GET_COUNT() < 24000) { // 1 second = 24000000
            ;
        }
        }
}