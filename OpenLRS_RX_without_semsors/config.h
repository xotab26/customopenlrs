// ***********************************************************
// ***          OpenLRS Rx Configuration file               **
// **        Developed by Melih Karakelle on 2010-2011      **
// **       This Source code licensed under GPL             **
// ***********************************************************
// Version Number     : 1.10
// Latest Code Update : 2011-10-04
// Supported Hardware : OpenLRS Rx boards (store.flytron.com)
// Project Forum      : http://forum.flytron.com/viewforum.php?f=7
// Google Code Page   : http://code.google.com/p/openlrs/

//###### RSSI STUFF ########
// 
#define RSSI_MAX 110 //turn on the RSSI debug and see what values you r getting at close range.... 
#define RSSI_SMOOTH  100   //increase if the Vout is still jumpy.



//####### RX BOARD TYPE #######
// 1 = Rx v1 Board
// 2 = Rx v2 Board
#define RX_BOARD_TYPE 1


//######### DEBUG MODES ##########
// 0 = No Debug Output
// 1 = Servo Position values 
// 4 = Failsafe values from "load_failsafe_values" function
// 5 = Hopping Channel number from "Hopping" function
// 6 = Wii Motion Plus's gyro values from "wmp_receiveData" function
// 7 = MMA7455 accelerometer values from "MMA7455_Read" function
// 8 = HMC5883L magnetometer values from "HMC5883L_Read" function
// 98= RSSI
// 99= Status informations
#define DEBUG_MODE 99

//######### TRANSMISSION VARIABLES ##########
#define CARRIER_FREQUENCY 433090  // 435Mhz startup frequency
//#define CARRIER_FREQUENCY 458550  // 459 Mhz startup frequency
#define FREQUENCY_HOPPING 1 // 1 = Enabled  0 = Disabled
#define FHSSseed 13     //MODIFY THIS FOR CUSTOM HOP PATTERN!



//###### HOPPING CHANNELS #######
//Select the hopping channels between 0-255
// Default values are 13,54 and 23 for all transmitters and receivers, you should change it before your first flight for safety.
//Frequency = CARRIER_FREQUENCY + (StepSize(60khz)* Channel_Number) 
//static unsigned char hop_list[3] = {13,54,23};
//Frequency = CARRIER_FREQUENCY + (StepSize(50khz) * Channel_Number) 
static unsigned char hop_list[20] = {
        0,12,4,30,8,20,2,14,6,18,10,32,24,3,28,16,22,34,26,5};


//###### RF DEVICE ID HEADERS #######
// Change this 4 byte values for isolating your transmission, RF module accepts only data with same header
static unsigned char RF_Header[4] = {
     't','r','a','c'}; //***change this value from the default !!*** 

//###### SERIAL PORT SPEED #######

#define SERIAL_BAUD_RATE 115200 //115.200 baud serial port speed

//###### SERIAL PPM Type #######
// Plug a jumper between Ch1 and CH3 for switching your Rx to SerialPPM mode
// =Serial PPM Types=
// 0 is classic SerialPPM, disables parallel outputs and uses CH8 for serial PPM output.
// 1 is Mixed PPM, uses CH4 for serial PPM output and last 4 channels (CH5,CH6,CH7,CH8) works normally as ParallelPPM mode. 
#define SERIAL_PPM_TYPE 0

//###### TELEMETRY MODES ########
#define TELEMETRY_ENABLED 0  // 1 = Enabled(bidirectional mode)  0 = Disabled(unidirectional mode)
#define TELEMETRY_MODE 0 // 0 = Transparent Bridge(750 byte/second max.) // 1 = Standard OpenLRS Telemetry



// Channel Names and Numbers
#define AILERON 0
#define ELEVATOR 1
#define THROTTLE 2
#define RUDDER 3
#define RETRACTS 4
#define FLAPS 5
#define AUX1 6
#define AUX2 7
// MOD
#define AUX3 8
#define AUX4 9
#define AUX5 10
#define AUX6 11
#define AUX7 12
#define AUX8 13
#define AUX9 14
#define AUX10 15


//unsigned char RF_Rx_Buffer[17];
//unsigned char RF_Tx_Buffer[17]; 
//unsigned char RS232_Tx_Buffer[20] = {
//     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	//rs232 tx buffer
//unsigned int Servo_Buffer[10] = {
//     3000,3000,3000,3000,3000,3000,3000,3000};	//servo position values from RF
//unsigned int Servo_Position[10] = {
//     3000,3000,3000,3000,3000,3000,3000,3000};	//real servo position values
     
unsigned char RF_Rx_Buffer[34];
unsigned char RF_Tx_Buffer[34]; 
unsigned char RS232_Tx_Buffer[20] = {
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	//rs232 tx buffer
unsigned int Servo_Buffer[16] = {
     3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000};	//servo position values from RF
unsigned int Servo_Position[20] = {
     3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000,3000};//real servo position values
     
static unsigned char Servo_Number = 0;
unsigned int total_ppm_time = 0;
unsigned short Rx_RSSI,vbat = 0;


//***************************************
//*   thUndead's RSSI MOD
//*   info: variable used for RSSI MOD
unsigned int rssipwm,rssibuf = 0;
unsigned short rssicounter = 0;
unsigned char seed,lastseed =1;
//***************************************


static unsigned char receiver_mode = 0;
static unsigned char hopping_channel = 1;

unsigned char temp_char;
unsigned int temp_int;

unsigned long time;
unsigned long last_pack_time ;
unsigned long last_hopping_time;
unsigned char failsafe_mode = 0; //Falsafe modes  0 = Deactive, 1 = Active

volatile unsigned char RF_Mode = 0;
#define Available 0
#define Transmit 1
#define Transmitted 2
#define Receive 3
#define Received 4


unsigned char loop_counter = 0; // telemetry loop counter


#if (RX_BOARD_TYPE==1)
//## RFM22B Pinouts for Rx v1 Board
#define SDO_pin A0
#define SDI_pin A1        
#define SCLK_pin 2 
#define IRQ_pin 3
#define nSel_pin 4
#define IRQ_interrupt 1

#define  nIRQ_1 (PIND & 0x08)==0x08 //D3
#define  nIRQ_0 (PIND & 0x08)==0x00 //D3

#define  nSEL_on PORTD |= (1<<4) //D4
#define  nSEL_off PORTD &= 0xEF //D4

#define  SCK_on PORTD |= (1<<2) //D2
#define  SCK_off PORTD &= 0xFB //D2

#define  SDI_on PORTC |= (1<<1) //C1
#define  SDI_off PORTC &= 0xFD //C1

#define  SDO_1 (PINC & 0x01) == 0x01 //C0
#define  SDO_0 (PINC & 0x01) == 0x00 //C0

//#### Other interface pinouts ###
#define GREEN_LED_pin A2
#define RED_LED_pin A3

#define Red_LED_ON  PORTC &= ~_BV(2);PORTC |= _BV(3);
#define Red_LED_OFF  PORTC &= ~_BV(2);PORTC &= ~_BV(3);

#define Green_LED_ON  PORTC &= ~_BV(3);PORTC |= _BV(2);
#define Green_LED_OFF  PORTC &= ~_BV(3);PORTC &= ~_BV(2);    

#define Servo_Ports_LOW PORTB &= 0x00; PORTD &= 0x1F; // pulling down the servo outputs

#define RSSI_MODE 1 //0=disable  1=enable 
#define RSSI_OUT 6 //Servo9 or RSSI

#define Servo1_OUT 5 //Servo1
#define Servo2_OUT 7 //Servo2
#define Servo3_OUT 8 //Servo3
#define Servo4_OUT 9 //Servo4
#define Servo5_OUT 10 //Servo5
#define Servo6_OUT 11 //Servo6
#define Servo7_OUT 12 //Servo7
#define Servo8_OUT 13 //Servo8
#define Servo9_OUT 13 //Servo9 // not have on this version


#define Servo1_OUT_HIGH PORTD |= _BV(5) //Servo1
#define Servo2_OUT_HIGH PORTD |= _BV(7) //Servo2
#define Servo3_OUT_HIGH PORTB |= _BV(0) //Servo3
#define Servo4_OUT_HIGH PORTB |= _BV(1) //Servo4
#define Servo5_OUT_HIGH PORTB |= _BV(2) //Servo5
#define Servo6_OUT_HIGH PORTB |= _BV(3) //Servo6
#define Servo7_OUT_HIGH PORTB |= _BV(4) //Servo7
#define Servo8_OUT_HIGH PORTB |= _BV(5) //Servo8
#define Servo9_OUT_HIGH PORTB = PORTB  //Servo9 // not have on this version

#define Serial_PPM_OUT_HIGH PORTB = _BV(5) //Serial PPM out on Servo 8
#endif

//------------------------------------------------------------------------------------------------
#if (RX_BOARD_TYPE==2)
//### PINOUTS OF OpenLRS Rx V2 Board
#define SDO_pin A0
#define SDI_pin A1        
#define SCLK_pin A2 
#define IRQ_pin 2
#define nSel_pin 4
#define IRQ_interrupt 0

#define  nIRQ_1 (PIND & 0x04)==0x04 //D2
#define  nIRQ_0 (PIND & 0x04)==0x00 //D2

#define  nSEL_on PORTD |= 0x10 //D4
#define  nSEL_off PORTD &= 0xEF //D4

#define  SCK_on PORTC |= 0x04 //C2
#define  SCK_off PORTC &= 0xFB //C2

#define  SDI_on PORTC |= 0x02 //C1
#define  SDI_off PORTC &= 0xFD //C1

#define  SDO_1 (PINC & 0x01) == 0x01 //C0
#define  SDO_0 (PINC & 0x01) == 0x00 //C0

//#### Other interface pinouts ###
#define GREEN_LED_pin 13
#define RED_LED_pin A3

#define Red_LED_ON  PORTC |= _BV(3);
#define Red_LED_OFF  PORTC &= ~_BV(3);

#define Green_LED_ON  PORTB |= _BV(5);
#define Green_LED_OFF  PORTB &= ~_BV(5);

#define Servo_Ports_LOW PORTB &= 0xE0; PORTD &= 0x17; // pulling down the servo outputs

#define Servo1_OUT 3 //Servo1
#define Servo2_OUT 5 //Servo2
#define Servo3_OUT 6 //Servo3
#define Servo4_OUT 7 //Servo4
#define Servo5_OUT 8 //Servo5
#define Servo6_OUT 9 //Servo6
#define Servo7_OUT 10 //Servo7
#define Servo8_OUT 11 //Servo8
#define Servo9_OUT 12 //Servo9

#define RSSI_MODE 1 //0=disable  1=enable 
#define RSSI_OUT 10 //Servo7 or RSSI

#define Servo1_OUT_HIGH PORTD |= _BV(3) //Servo1
#define Servo2_OUT_HIGH PORTD |= _BV(5) //Servo2
#define Servo3_OUT_HIGH PORTD |= _BV(6) //Servo3
#define Servo4_OUT_HIGH PORTD |= _BV(7) //Servo4
#define Servo5_OUT_HIGH PORTB |= _BV(0) //Servo5
#define Servo6_OUT_HIGH PORTB |= _BV(1) //Servo6
#define Servo7_OUT_HIGH PORTB |= _BV(2) //Servo7
#define Servo8_OUT_HIGH PORTB |= _BV(3) //Servo8
#define Servo9_OUT_HIGH PORTB |= _BV(4) //Servo9 

#define Serial_PPM_OUT_HIGH PORTB = _BV(3) //Serial PPM out on Servo 8
#endif

