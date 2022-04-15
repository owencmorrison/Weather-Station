//Owen Morrison
//Project: Weather Station
//Purpose of program: Software for the constructed weather station
//IMPORTANT: desolder the 0-ohm resistors that connects PD0 to PB6 and PD1 to PB7 before running this program on the TIVA
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "tm4c123ge6pm.h"

void delay(uint32_t l){
    for(uint32_t x = 0; x < l; x++){}
}

void portE(){
    //Init of Port E
    //Enable GPIO for Port E
    SYSCTL_RCGCGPIO_R|=0x10; //enable clock to port E
    while((SYSCTL_PRGPIO_R&(0x10))==0);
    GPIO_PORTE_CR_R=0x3C; //PE2-PE5 can be written
    GPIO_PORTE_DIR_R=0x3C; //Set PE2 - PE5 to outputs
    GPIO_PORTE_AFSEL_R=0; //Set all pins in port E as GPIO
    GPIO_PORTE_DEN_R=0x3C; //Digital functions for PE2 - PE5 enabled
    //Port E pins connect to these motor wires
    /*PORT E |  MOTOR WIRE
     *PIN 2  |  RED
     *PIN 3  |  YELLOW
     *PIN 4  |  WHITE
     *PIN 5  |  BLUE*/
}

void motorFor(uint32_t steps, uint32_t speed){
    for(int x = 0; x < steps; x++){
        //50 of this loop provides one complete rotation
        delay(speed); //Delay of .03 seconds
        //excitation sequence step 1
        GPIO_PORTE_DATA_R=0x14; //0b00010100 Red and White HIGH
        delay(speed); //Delay of .03 seconds
        //excitation sequence step 2
        GPIO_PORTE_DATA_R=0xC; //0b00001100 Red and Yellow HIGH
        delay(speed); //Delay of .03 seconds
        //excitation sequence step 3
        GPIO_PORTE_DATA_R=0x28; //0b00101000 Blue and Yellow HIGH
        delay(speed); //Delay of .03 seconds
        //excitation sequence step 4
        GPIO_PORTE_DATA_R=0x30; //0b00110000 Blue and White HIGH
    }
    //Turn off motor
    GPIO_PORTE_DATA_R=0;
}

void motorBac(uint32_t steps, uint32_t speed){
    for(int x = 0; x < steps; x++){
        //200 (or 50?) of this loop provides one complete rotation
        delay(speed); //Delay of .03 seconds
        //excitation sequence step 1
        GPIO_PORTE_DATA_R=0x30; //0b00110000 Blue and White HIGH
        delay(speed); //Delay of .03 seconds
        //excitation sequence step 2
        GPIO_PORTE_DATA_R=0x28; //0b00101000 Blue and Yellow HIGH
        delay(speed); //Delay of .03 seconds
        //excitation sequence step 3
        GPIO_PORTE_DATA_R=0xC; //0b00001100 Red and Yellow HIGH
        delay(speed); //Delay of .03 seconds
        //excitation sequence step 4
        GPIO_PORTE_DATA_R=0x14; //0b00010100 Red and White HIGH
    }
    //Turn off motor
    GPIO_PORTE_DATA_R=0;
}

void enableI2C(void){
    SYSCTL_RCGCI2C_R=0x2; //enable I2C mod 1
    SYSCTL_SRI2C_R=0x2; //Reset I2C mod 1
    delay(10); //small delay for reset to occur
    SYSCTL_SRI2C_R=0x0; //Reset byte low
    SYSCTL_RCGCGPIO_R=0x1; //Enable GPIO port A
    GPIO_PORTA_AFSEL_R=0xC0; //Alt functions for PA6-PA7
    GPIO_PORTA_DEN_R=0xC0; //Digital functions for PA6-PA7 enabled
    GPIO_PORTA_ODR_R=0x80; //Open drain on I2C1SDA (PA7)
    GPIO_PORTA_PCTL_R=0x33000000; //configure PA6 - I2C1SCL, PA7 - I2C1SDA
    I2C1_MCR_R=0x10; //Initialize I2C Master
    I2C1_MTPR_R=0x7; //Desired clock speed for 16 MHz
}

void enableKEYLCD(void){
    //Based on initialization and configuration of SSI on pg. 965 and initialization and configuration of GPIO on pg. 656
    //IMPORTANT: desolder the 0-ohm resistors that connects PD0 to PB6 and PD1 to PB7
    //SPI for port D
    //PD0 = SSI3Clk (SCK), PD1 = SSI3Fss (CS), PD2 = SSI3Rx (SDO), and PD3 = SSI3Tx (SDI)
    SYSCTL_RCGCSSI_R=0x8; //Enable and provide a clock to SSI module 3 (SSI for Port D) in run mode
    SYSCTL_RCGC2_R=0xA; //Port D receives a clock and functions. Also writes a corresponding bit in RCGCGPIO (Enables and provides a clock for GPIO Port D)
    GPIO_PORTD_AFSEL_R=0xF; //PD0 - PD3 set as peripheral signal controlled by alternate hardware function
    GPIO_PORTD_PCTL_R=0x00001111; //PDO - PD3 peripheral signals set as SSI3
    GPIO_PORTD_DEN_R=0xF; //Enable digital functions for pins PD0 - PD3
    GPIO_PORTD_PUR_R=0xF; //Enable pull-up resistors for pins PD0 - PD3
    SSI3_CR1_R=0; //Clear SSICR1 before making changes (this line also configures ssi as a master)
    SSI3_CC_R=0; //Sets SSI Baud Clock Source as System clock
    SSI3_CPSR_R= 254; //Sets a division factor of 254 (highest possible)
    SSI3_CR0_R=0x7; //SCR = 0, Data captured on the first clock edge transition (SPH=0), steady low value placed on SSI3Clk(SPO=0), Freescale SPI, and SSI data size set to 8-bit data
    SSI3_CR1_R|=0x2; //Enable SSI operation
    //GPIO for port B
    SYSCTL_RCGCGPIO_R|=0x2; //enable clock to port B
    //while((SYSCTL_PRGPIO_R&(0x2))==0);
    GPIO_PORTB_LOCK_R=0x4C4F434B; //unlock port B
    GPIO_PORTB_CR_R=0xFE;
    GPIO_PORTB_DIR_R=0x1E; //Set PB1 - PB4 to outputs and PB4 - PB7 to inputs
    GPIO_PORTB_AFSEL_R=0; //Set all pins in port B as GPIO
    GPIO_PORTB_AMSEL_R = 0x0;
    GPIO_PORTB_PCTL_R=0x0;
    GPIO_PORTB_DR2R_R=0xFE; //Gives all pins in port B a 2-mA drive (also clears corresponding bits in GPIODR4R and GPIODR8R)
    GPIO_PORTB_PUR_R=0x1E; //Enables pull-up resistors on PB1 - PB4 (port B outputs)
    GPIO_PORTB_PDR_R=0xE0; //Enables pull-down resistor on PB5 - PB7
    GPIO_PORTB_DEN_R=0xFE; //Digital functions for PB1 - PB7 enabled
}

void sendByteLCD(char byte){
    SSI3_DR_R=byte; //Send byte to SSI 3 Data reg
    while((SSI3_SR_R&(0x1))==0); //Checks bit 0 of SSI3 status reg to see if the transmit FIFO is empty. This line ensures the char is sent before the next one is sent.
    //SSI3_SR_R&(0x1) returns 0 when bit 0 is not empty and returns 1 when it is empty.
}

void sendStringLCD(char *str){ //Function prints given string to LCD
   unsigned char length=strlen(str); //obtains length of string
   for (uint8_t x=0;x<length;x++){ //for loop for the length of string. This for loop prints the string onto the LCD.
      sendByteLCD(str[x]); //Sends char str[x] to SSI3 data reg
   }
}
uint32_t inputPortB(void){
    return (GPIO_PORTB_DATA_R&(0xE0)); //Returns inputs PB5-PB7
}

void enableRow(unsigned int x){
    switch (x){
        case 0:
            GPIO_PORTB_DATA_R=0x2; //PB1 HIGH
            delay(400); //small delay
            break;
        case 1:
            GPIO_PORTB_DATA_R=0x4; //PB2 HIGH
            delay(400); //small delay
            break;
        case 2:
            GPIO_PORTB_DATA_R=0x8; //PB3 HIGH
            delay(400); //small delay
            break;
        case 3:
            GPIO_PORTB_DATA_R=0x10; //PB4 HIGH
            delay(400); //small delay
            break;
        default:
            break;
    }
}

void clearScreen(){
    sendByteLCD(0x7C); //Enter Settings Mode.
    sendByteLCD(0x2D); //Clear display. Move cursor to home position.
}

char detectKey(){
    //Array of characters that represent a key on the keypad
    char keys[12]={'1','2','3',
                   '4','5','6',
                   '7','8','9',
                   '*','0','#'};
    uint32_t input; //this variable will hold the value read from GPIO
    char key = 0; //this char will hold the character that represents the pressed key
    while(key == 0){
        for(uint32_t x = 0; x < 4; x++){
            enableRow(x); //turns one row high and the others low
            input = inputPortB(); //reads gpioB data reg
            if(input > 0){ //if key is pressed input > 0
                if(input == 0x20){ //if key in columun one is pressed then input is 0x20
                    //recheck because of key bounce
                    delay(40000); //small delay to check for key bounce
                    if(inputPortB() == 0x20){key = (keys[0+(3*x)]);} //if true then key pressed in column 1 row x
                    while(inputPortB() == 0x20); //Wait for key to be released
                }
                else if(input == 0x40){ //if key in columun one is pressed then input is 0x40
                    //recheck because of key bounce
                    delay(40000); //small delay to check for key bounce
                    if(inputPortB() == 0x40){key = (keys[1+(3*x)]);} //if true then key pressed in column 2 row x
                    while(inputPortB() == 0x40); //Wait for key to be released
                }
                else if(input == 0x80){ //if key in columun one is pressed then input is 0x80
                    //recheck because of key bounce
                    delay(40000); //small delay to check for key bounce
                    if(inputPortB() == 0x80){key = (keys[2+(3*x)]);} //if true then key pressed in column 3 row x
                    while(inputPortB() == 0x80); //Wait for key to be released
                }
            }
        }
    }
    return key; //returns pressed key
}

uint8_t* readTemp(){
    //based on I2C flowcharts from TM4C123GH6PM data sheets
    static uint8_t readData[7];
    //skip checking the status word for now, just assume that the regs are fine.
    I2C1_MSA_R=0x70; //write to slave address 0x38
    //delay(2000); //small delay
    I2C1_MDR_R=0xAC; //command for trigger measurement
    while(I2C1_MCS_R&(0x40) == 0x40); //Check BUSBUSY bit if 1 then wait
    I2C1_MCS_R=0x3; //RUN START start sending bytes
    //maybe the busy and error check should be their own functions
    while(I2C1_MCS_R&(0x1) == 1); //Check BUSY bit if 1 then wait
    if(I2C1_MCS_R&(0x2) == 0x2){ //Check if error it is high
        if(I2C1_MCS_R&(0x10) == 0){ //Check if ARBLST is low
            I2C1_MCS_R=0x4; //STOP in case of error
        }
        return 0;
    }
    I2C1_MDR_R=0x33; //Load data reg with 0x33
    I2C1_MCS_R=0x1; //RUN continue sending bytes
    while(I2C1_MCS_R&(0x1) == 1); //Check BUSY bit if 1 then wait
    if(I2C1_MCS_R&(0x2) == 0x2){ //Check if error it is high
        if(I2C1_MCS_R&(0x10) == 0){ //Check if ARBLST is low
            I2C1_MCS_R=0x4; //stop in case of error
        }
        return 0;
    }
    I2C1_MDR_R=0x0; //Load data reg with 0x00
    I2C1_MCS_R=0x5; //STOP RUN sends last byte
    while(I2C1_MCS_R&(0x1) == 1); //Check BUSY bit if 1 then wait
    //don't care about the checking this error bit, but if I am getting issues with errors then I will rewrite this part.
    I2C1_MSA_R=0x71; //read to slave address 0x38
    delay(4545454); //delays program to allow measurements to occur
    while(I2C1_MCS_R&(0x40) == 0x40); //Check BUSBUSY bit if 1 then wait
    I2C1_MCS_R=0xB; //RUN, START and ACK(recieved data byte aknowledged by the master) start receving bytes
    for(uint32_t x = 0; x < 6; x++){
        while(I2C1_MCS_R&(0x1) == 1); //Check BUSY bit if 1 then wait
            if(I2C1_MCS_R&(0x2) == 0x2){ //Check if error it is high
                if(I2C1_MCS_R&(0x10) == 0){ //Check if ARBLST is low
                    I2C1_MCS_R=0x4; //STOP in case of error
                }
                return 0;
            }
        readData[x] = I2C1_MDR_R&(0xFF); //send received data byte to array
        if(x == 5){ //if the last byte is being read
            I2C1_MCS_R=0x5; //STOP RUN receive last byte
        }
        else{
            I2C1_MCS_R=0x9; //ACK RUN continue receiving bytes
        }
    }
    while(I2C1_MCS_R&(0x1) == 1); //Check BUSY bit if 1 then wait
    if(I2C1_MCS_R&(0x2) == 0x2){ //Check if error it is high
        return readData; //return array
    }
    readData[6] = I2C1_MDR_R&(0xFF); //send received data byte to last element of the array
    return readData; //return array
}

void readSensor(){
        uint8_t* bytes = readTemp(); //reads data from temp sensor and stores it in bytes
        uint32_t humD = (bytes[1] << 12) | (bytes[2] << 4) | ((bytes[3]&(0xF0)) >> 4); //grabs the humidity data from bytes
        uint32_t tempD = ((bytes[3]&(0xF)) << 16) | (bytes[4] << 8) | (bytes[5]); //grabs the temp data from bytes
        float temp = (tempD*(1.9073486e-04))-50; //convert tempD into temp, 1.9073486e-04 is equal to 200/(2^20)
        float hum = humD*(9.53674363e-05); //convert humD to realitive humidity, 9.53674363e-05 is equal to 100/(2^20)
        char tempS[5]; //char array to hold temp string
        char humS[5]; //char array to hold hum string
        snprintf(tempS, 5, "%3f", temp); //temp being converted into a string and stored into tempS (rounded to 3 digits)
        snprintf(humS, 5, "%3f", hum); //hum being converted into a string and stored into humS (rounded to 3 digits)
        //sprintf(tempS, "%d", temp); //converts int to string
        //sprintf(humS, "%d", hum); // converts int to string
        clearScreen(); //clear screen
        sendStringLCD("Temp = "); //prints string
        sendStringLCD(tempS); //prints temp
        sendStringLCD("     Hum = "); //prints string
        sendStringLCD(humS); //prints rel humidity
        sendStringLCD(" [X-#]"); //tells the user to press # to leave
        while(detectKey() != '#'){} //waits for # to be pressed before leaving
}

void motorControl(){
    uint32_t steps = 50; //amount of loops required for a complete rotation
    uint32_t speed; //length of delay
    char dir; //stores character that selects the direction
    clearScreen(); //clear screen
    sendStringLCD("Select speed    using 0-9"); //print instruction string
    speed = detectKey(); //waits for pressed key and stores it in speed
    while(speed == '*' || speed == '#'){speed = detectKey();} //continue reading keys until 0-9 are pressed
    //ascii numbers minus 48 converts them into dec numbers
    speed = 9999*(1+(1*(speed-48))); //math to create unique delay based on pressed button
    clearScreen(); //clear screen
    sendStringLCD("1- Forwards     2- Backwards"); //sends instructions to LCD
    dir = detectKey(); //waits for pressed key and stores it in dir
    while((dir != '1') && (dir != '2')){dir = detectKey();} //continue reading keys until 1 or 2 are pressed
    clearScreen(); //clear screen
    sendStringLCD("Rotating..."); //displaying rotating while motor rotates
    if(dir == '1'){ //checks if dir is forwards
        motorFor(steps, speed);
    }
    else{ //else dir is backwards
        motorBac(steps, speed);
    }

}



void backlight(){
    clearScreen(); //clear screen
    sendStringLCD("Press 0-9 to    select the BL"); //instruction displays on screen
    uint32_t selection = detectKey(); //waits for pressed key and stores in selection
    while(selection == '*' || selection == '#'){selection = detectKey();} //continues reading keys until 0-9 are pressed
    switch(selection){ //switch statement for backlight settings
        case '1':
            //color 1 black (R:0 B:0 G:0)
            sendByteLCD(0x7C); //Enter Settings Mode.
            sendByteLCD(0x2B); //Set RGB backlight
            sendByteLCD(0x00); //Red backlight byte
            sendByteLCD(0x00); //Blue backlight byte
            sendByteLCD(0x00); //Green backlight byte
            break;
        case '2':
            //color 2 white (R:FF B:FF G:FF)
            sendByteLCD(0x7C); //Enter Settings Mode.
            sendByteLCD(0x2B); //Set RGB backlight
            sendByteLCD(0xFF); //Red backlight byte
            sendByteLCD(0xFF); //Blue backlight byte
            sendByteLCD(0xFF); //Green backlight byte
            break;
        case '3':
            //color 3 red (R:FF B:0 G:0)
            sendByteLCD(0x7C); //Enter Settings Mode.
            sendByteLCD(0x2B); //Set RGB backlight
            sendByteLCD(0xFF); //Red backlight byte
            sendByteLCD(0x00); //Blue backlight byte
            sendByteLCD(0x00); //Green backlight byte
            break;
        case '4':
            //color 4 blue (R:0 B:FF G:0)
            sendByteLCD(0x7C); //Enter Settings Mode.
            sendByteLCD(0x2B); //Set RGB backlight
            sendByteLCD(0x00); //Red backlight byte
            sendByteLCD(0xFF); //Blue backlight byte
            sendByteLCD(0x00); //Green backlight byte
            break;
        case '5':
            //color 5 green (R:0 B:0 G:FF)
            sendByteLCD(0x7C); //Enter Settings Mode.
            sendByteLCD(0x2B); //Set RGB backlight
            sendByteLCD(0x00); //Red backlight byte
            sendByteLCD(0x00); //Blue backlight byte
            sendByteLCD(0xFF); //Green backlight byte
            break;
        case '6':
            //color 6 purple (R:80 B:80 G:0)
            sendByteLCD(0x7C); //Enter Settings Mode.
            sendByteLCD(0x2B); //Set RGB backlight
            sendByteLCD(0x80); //Red backlight byte
            sendByteLCD(0x80); //Blue backlight byte
            sendByteLCD(0x00); //Green backlight byte
            break;
        case '7':
            //color 7 hot pink (R:FF B:B4 G:69)
            sendByteLCD(0x7C); //Enter Settings Mode.
            sendByteLCD(0x2B); //Set RGB backlight
            sendByteLCD(0xFF); //Red backlight byte
            sendByteLCD(0xB4); //Blue backlight byte
            sendByteLCD(0x69); //Green backlight byte
            break;
        case '8':
            //color 8 coral (R:FF B:50 G:7F)
            sendByteLCD(0x7C); //Enter Settings Mode.
            sendByteLCD(0x2B); //Set RGB backlight
            sendByteLCD(0xFF); //Red backlight byte
            sendByteLCD(0x50); //Blue backlight byte
            sendByteLCD(0x7F); //Green backlight byte
            break;
        case '9':
            //color 9 dark orange (R:FF B:8C G:0)
            sendByteLCD(0x7C); //Enter Settings Mode.
            sendByteLCD(0x2B); //Set RGB backlight
            sendByteLCD(0xFF); //Red backlight byte
            sendByteLCD(0x8C); //Blue backlight byte
            sendByteLCD(0x00); //Green backlight byte
            break;
        default:
            //the switch should not end up here if it does then break
            break;
    }
}

int main(void)
{
    //System clock is 16 MHz by default
    enableI2C(); //enable I2C for the temp sensor
    enableKEYLCD(); //Enable SPI for LCD and GPIO for keypad
    portE(); //Enable GPIO for the motor
    delay(999999); //delay waiting for splash screen
    clearScreen(); //clear screen
    sendStringLCD("1.Check Sensor  2.Motor Control"); //sends instructions to LCD
    while(1){ //while always
        switch (detectKey()) {
            case '1': //Check Sensor
                //check temp/hum
                readSensor(); //Read Sensor
                clearScreen();//clear screen
                sendStringLCD("1.Check Sensor  2.Motor Control"); //sends instructions to LCD
                break;
            case '2': //Motor Control
                //motor control
                motorControl(); //function to control motor
                clearScreen(); // clear screen
                sendStringLCD("1.Check Sensor  2.Motor Control"); //sends instructions to LCD
                break;
            case '*':
                //backlight settings
                backlight(); //function to change backlight
                clearScreen(); //clear screen
                sendStringLCD("1.Check Sensor  2.Motor Control"); //sends instructions to LCD
                break;
            default:
                break;
        }

    }
}
