#include "i2c_master_noint.h"
#include "ssd1306.h"
#include "font.h"

// this sucks but see note in drawString
#define NUM_ELEMS 50 // define number of elements in character array message

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
void blinkPixel();
void drawChar(int x, int y, int ascii_num); 
void drawString(int x, int y, char m[]);


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
    
    //Set up i2c comm
    i2c_master_setup();
    
    //Set up OLED display and clear pixels
    ssd1306_setup();
            
    //Create a string by calling sprintf then use drawString to display text on OLED
    int i = 5;
    double fps;
    
    // Write initial message to OLED
    char message1[NUM_ELEMS] = "";
    
    sprintf(message1, "Yo! ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    drawString(0,0,message1);
    ssd1306_update();
    
    
    int update_count = 0;
    
    while(1) {
        
//        heartbeat();
//        blinkPixel();
        
        // Calculate FPS
        _CP0_SET_COUNT(0); // Set core timer to 0
        ssd1306_update();
        update_count++;
        double seconds = _CP0_GET_COUNT()/24000000.0; // Read core timer after updating OLED (1s = 24000000)
        double fps = 1.0/seconds;
        
        // Print number of updates to display        
        char message3[NUM_ELEMS] = "";
        sprintf(message3, "updated frames = %d",update_count);
        drawString(0,16,message3);
        
        // Print the FPS to the display
        char message2[NUM_ELEMS] = "";
        sprintf(message2, "fps = %4.2f",fps);
        drawString(0,24,message2);
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

void blinkPixel() {
    
    ssd1306_drawPixel(1, 1, 1);
    ssd1306_update();
    ssd1306_drawPixel(1, 1, 0);
    ssd1306_update();
 
}

// Takes an ascii character, converts it to type int to find it's bitmap in font.c,
// then sets pixels based on that bitmap to display character
void drawChar(int x, int y, int ascii_num) {
    int k,j;
    for (k = 0; k < 8; k++){
        for (j = 0; j < 5; j++) {
             ssd1306_drawPixel(j+x,k+y,((ASCII[ascii_num-0x20][j])>>k)&1);
        }
    }

}

// Takes an array of characters and calls the drawChar function for each character in
// the array at the appropriate position on OLED display
void drawString(int x, int y, char m[]){
//    int num_elems = (sizeof(m)/sizeof(m[0])); // Can't make this work?? Shouldn't have to hard code num of elements tho
    int i;

    for (i = 0; i < NUM_ELEMS; i++){
        if (m[i]=='\0'){
            m[i] = ' ';
        }
        drawChar(x,y,m[i]);
        x+=5;
        if (x > 125){ //New line when reach horizontal limit
            y+=8;
            x=0;
        }
    }
}