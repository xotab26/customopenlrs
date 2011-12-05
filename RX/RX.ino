// **********************************************************
// ******************   OpenLRS Rx Code   *******************
// ***  OpenLRS Designed by Melih Karakelle on 2010-2011  ***
// **  an Arudino based RC Rx/Tx system with extra futures **
// **       This Source code licensed under GPL            **
// **********************************************************
// Version Number     : 1.10
// Latest Code Update : 2011-10-04
// Supported Hardware : OpenLRS Rx boards (store.flytron.com)
// Project Forum      : http://forum.flytron.com/viewforum.php?f=7
// Google Code Page   : http://code.google.com/p/openlrs/
// **********************************************************
// # PROJECT DEVELOPERS # 
// Melih Karakelle (http://www.flytron.com) (forum nick name: Flytron)
// Jan-Dirk Schuitemaker (http://www.schuitemaker.org/) (forum nick name: CrashingDutchman)
// Etienne Saint-Paul (http://www.gameseed.fr) (forum nick name: Etienne) 
// Mihai Andrian aka thUndead (http://www.fpvuk.org/forum/index.php?topic=3642.0)
// Additional Mods by Kermit


//##########################################################
//THIS FW IS A BETA and should be treated with respect!
//Features:  435 Mhz FW
//Pseudo Random FHSS on 20ch 
// TX Hopping freq: 20ms
//  RX recover time: 30ms
// FW running @ 38.4 baud to improve range
// Any questions here: http://www.fpvuk.org/forum/index.php?topic=3642.0

//##################################################################
// Last but not least, 
//a lot of hours and nights went into this firmware 
//so you can buy me some Pringles and a Red Bull to keep me going :D 
//                 http://tinyurl.com/425x4hj
// Thanks to all who kindly provided me with Pringles :D
//##################################################################

//##########################################################
// README
// Change FHSSseed 89 in the config.h section for a unique hopping sequence should match your TX value
//##########################################################


//***************************************
//*   thUndead's RSSI MOD
//*   info: My take on RSSI voltage :D
//8    Regulates the dodgy rssi and outputs a voltage between 0 and 2.6v (min - max)
//***************************************

unsigned char prev;
#include "config.h"
#include <EEPROM.h>

unsigned char FHSS[256];  //FHSS random Hopping table :D

void setup()
{   
     //FHSS pattern generator
     randomSeed(FHSSseed);  
     for (int i=0;i<256;i++)                   
     {
          FHSS[i]=random(0,20);         //  PRN generator
     }

     //LEDs
     pinMode(GREEN_LED_pin, OUTPUT);  
     pinMode(RED_LED_pin, OUTPUT);

     //RF module pins
     pinMode(SDO_pin, INPUT); //SDO
     pinMode(SDI_pin, OUTPUT); //SDI        
     pinMode(SCLK_pin, OUTPUT); //SCLK
     pinMode(IRQ_pin, INPUT); //IRQ
     pinMode(nSel_pin, OUTPUT); //nSEL

     // Serial
     pinMode(0, INPUT); // Serial Rx
     pinMode(1, OUTPUT);// Serial Tx

     //RSSI
     pinMode(RSSI_OUT, OUTPUT); //RSSI pinout

     //Servos
     pinMode(Servo1_OUT, OUTPUT); //Servo1
     pinMode(Servo2_OUT, OUTPUT); //Servo2
     pinMode(Servo3_OUT, OUTPUT); //Servo3
     pinMode(Servo4_OUT, OUTPUT); //Servo4
     pinMode(Servo5_OUT, OUTPUT); //Servo5
     pinMode(Servo6_OUT, OUTPUT); //Servo6
     pinMode(Servo7_OUT, OUTPUT); //Servo7
     pinMode(Servo8_OUT, OUTPUT); //Servo8


     Serial.begin(SERIAL_BAUD_RATE); //Serial Transmission 

     INIT_SERVO_DRIVER();

     attachInterrupt(IRQ_interrupt,RFM22B_Int,FALLING);

}


//############ SERVO INTERRUPT ##############
// We configured the ICR1 value for 40.000 into the "init_servo_driver" function. 
// It's mean this interrupt works when the Timer1 value equal 40.000
// we are configuring it for 40.000 - servo_signal_time for each servo channel, 
// and The interrupt generating perfect servo timings for us.
// Timer1 configured for 1/8 CPU clock. with this configuration, each clock time is equal 0.5us 
// and we are driving the servo with 2048 step resolution.

ISR(TIMER1_OVF_vect)
{
     unsigned int us; // this value is not real microseconds, we are using 0.5us resolution (2048 step),
     // this is why the all values 2 times more than real microseconds.

     Servo_Ports_LOW;

     Servo_Number++; // jump to next servo
     if (Servo_Number>8) // back to the first servo 
     {
          total_ppm_time = 0; // clear the total servo ppm time
          Servo_Number=0;
     }


     if (Servo_Number == 8)  // Check the servo number. 
     {
          //Servos accepting 50hz ppm signal, this is why we are waiting for 20ms before second signal brust. 
          us = 40000 - total_ppm_time; //wait for total 20ms loop.  waiting time = 20.000us - total servo times
     }
     else
          us = Servo_Position[Servo_Number]; // read the servo timing from buffer

     total_ppm_time += us; // calculate total servo signal times.

     if (receiver_mode==0) // Parallel PPM
     {  
          switch (Servo_Number) {
          case 0:
               Servo1_OUT_HIGH;
               break;
          case 1:
               Servo2_OUT_HIGH;
               break;
          case 2:
               Servo3_OUT_HIGH;
               break;
          case 3:
               Servo4_OUT_HIGH;
               break;
          case 4:
               Servo5_OUT_HIGH;
               break;
          case 5:
               Servo6_OUT_HIGH;
               break;
          case 6:
               Servo7_OUT_HIGH;
               break;
          case 7:
               Servo8_OUT_HIGH;
               break;
          case 8:
               Servo9_OUT_HIGH;
               break;  
          }     
     }
#if (SERIAL_PPM_TYPE == 0)
     else  // Serial PPM over 8th channel
     {
          Serial_PPM_OUT_HIGH;
     }
#else
     else //Mixed Serial+Parallel PPM
     {
          Servo4_OUT_HIGH; //serial from 4th channel  
          switch (Servo_Number) { //last 4 channel works normally
          case 4:
               Servo5_OUT_HIGH;
               break;
          case 5:
               Servo6_OUT_HIGH;
               break;
          case 6:
               Servo7_OUT_HIGH;
               break;
          case 7:
               Servo8_OUT_HIGH;
               break;
          case 8:
               Servo9_OUT_HIGH;
               break;    
          }   
     }  
#endif  

     TCNT1 = 40000 - us; // configure the timer interrupt for X micro seconds     
}


//############ MAIN LOOP ##############
void loop() 
{

     unsigned char i,tx_data_length;
     unsigned char first_data = 0;


     receiver_mode = check_modes(); // Check the possible jumper positions for changing the receiver mode.

     load_failsafe_values();   // Load failsafe values on startup

     if (receiver_mode == 1) Red_LED_Blink(3); // 3x Red LED blinks for serial PPM mode.


     Red_LED_ON;

     RF22B_init_parameter(); // Configure the RFM22B's registers
     frequency_configurator(CARRIER_FREQUENCY); // Calibrate the RFM22B to this frequency, frequency hopping starts from here.
     rx_mode(); 

     sei(); // Interupt Aktivate


     //Hop to first frequency from Carrier
#if (FREQUENCY_HOPPING==1)
     Hopping();
#endif   

     RF_Mode = Receive; 

     time = millis();

     last_pack_time = time; // reset the last pack receiving time for first usage
     last_hopping_time = time; // reset hopping timer

     while(1)
     {    /* MAIN LOOP */

          //Serial.println(seed,DEC);	 				 

          time = millis();

          // detect the locked module and reboot		                       
          if (_spi_read(0x0C)==0) 
          {
               Red_LED_ON;
               RF22B_init_parameter();
               frequency_configurator(CARRIER_FREQUENCY);
               rx_mode(); 
#if (DEBUG_MODE==99)
               Serial.println("RF22Reboot");
#endif
          }	 


          //Detect the broken RF link and switch it to failsafe mode after 1 seconds  
          if ((time-last_pack_time > 1000) && (failsafe_mode == 0))
          {
               Red_LED_ON;
#if (DEBUG_MODE==99)
               Serial.println("Package recive Timeout");  
#endif
               failsafe_mode = 1; // Activate failsafe mode
               load_failsafe_values(); // Load Failsafe positions from EEPROM
//               Direct_Servo_Drive(); // Set directly the channels form Servo buffer
          }


#if (FREQUENCY_HOPPING==1)	
          if ((time-last_hopping_time > 15))//automatic hopping for clear channel when rf link down for 30 ms.	
          {

               Red_LED_ON;
#if (DEBUG_MODE==99)
               Serial.print("Hopping Timeout on seed: ");
               Serial.println(seed,DEC);
#endif
               last_hopping_time = time;
               seed--;  
               seed = seed % 256;

               Hopping(); //Hop Hop Hop :D

          }  
#endif   

          if(RF_Mode == Received)   // RFM22B recived valid data package
          { 
               Green_LED_ON;

               failsafe_mode = 0; // deactivate failsafe mode
               last_pack_time = time; // record last package time

               send_read_address(0x7f); // Send the package read command

               for(i = 0; i<24; i++) //read all buffer 
               { 
                   RF_Rx_Buffer[i] = read_8bit_data(); 
               }  
               
               rx_mode();


               switch (RF_Rx_Buffer[0])  // Deside what is the pupose of the packet
               {
               case 'S':          // Servo Datas
                    for(i = 0; i<6; i++) //Write Rx Buffer 2-19 into the first 12 Servo  12 bit Position Array
                    {                                                          
                         Servo_Buffer[i*2] = 952 + word(RF_Rx_Buffer[(i*3)+2],(RF_Rx_Buffer[(i*3)+3]>>4));  // mount 
                         Servo_Buffer[(i*2)+1] = 952 + word((RF_Rx_Buffer[i*3+3]&0x0F),(RF_Rx_Buffer[(i*3)+4]));  // mount 
                    }
                    for(i = 12; i<16; i++) //Write RX Buffer 20-23 into the last 4 Servo  8 bit Position Array 
                    {                                                          
                         Servo_Buffer[i] = 952 + RF_Rx_Buffer[i+8]<<4;  // mount 
                    }
                    break;

               case 'F':           //FS packet detected and saved
                    for(i = 0; i<6; i++) //Write Rx Buffer 2-19 into the first 12 Servo  12 bit Position Array
                    {                                                          
                         Servo_Buffer[i*2] = 952 + word(RF_Rx_Buffer[(i*3)+2],(RF_Rx_Buffer[(i*3)+3]>>4));  // mount 
                         Servo_Buffer[(i*2)+1] = 952 + word((RF_Rx_Buffer[i*3+3]&0x0F),(RF_Rx_Buffer[(i*3)+4]));  // mount 
                    }
                    for(i = 12; i<16; i++) //Write RX Buffer 20-23 into the last 4 Servo  8 bit Position Array 
                    {                                                          
                         Servo_Buffer[i] = 952 + RF_Rx_Buffer[i+8]<<4;  // mount 
                    }
                    save_failsafe_values();
                    Red_LED_ON;
                    break;

//               case 'T':           // RS232 Tx data received
//                    tx_data_length = RF_Rx_Buffer[1]; // length of RS232 data
//                    for(i = 0; i<tx_data_length; i++)
//                         RS232_Tx_Buffer[i+1] = RF_Rx_Buffer[i+2]; // fill the RS232 buffer	
//                    break;
//
//#if (TELEMETRY_MODE == 0)  
//               case 'B':          // Transparent Bridge Telemetry mode          
//                    for(i = 2; i<RF_Rx_Buffer[1]+2; i++) //write serial
//                         Serial.print(RF_Rx_Buffer[i]);
//                    break;
//#endif
               }


//               Direct_Servo_Drive(); // use stick commands directly for standard rc plane flights



#if (TELEMETRY_ENABLED==1)
#if (TELEMETRY_MODE==0) 
               Telemetry_Bridge_Write(); // Write the serial buffer
#endif  

#if (TELEMETRY_MODE==1) 
               Telemetry_Write(); // Write the telemetry vals
#endif   

#endif

               thUndeadRSSI();


#if (FREQUENCY_HOPPING==1)

               // Frequency Hopping Algorithm

               if (RF_Rx_Buffer[1] != lastseed) // Check if Valid Packet and seed changed
               {
#if (DEBUG_MODE==99)
                    Serial.print("New Seed found in Servo Transmission : ");
                    Serial.print("Actual seed: ");
                    Serial.print(seed,DEC);
                    Serial.print(" Lastseed: ");
                    Serial.print(lastseed,DEC);                
                    Serial.print(" Newseed: ");
                    Serial.println(RF_Rx_Buffer[1],DEC);
#endif

                    if (RF_Rx_Buffer[1] != seed  )   // Paket mit neuem seed gefunden!
                    {    
#if (DEBUG_MODE==99)
                         Serial.print("Random DIFF detected! : ");
                         Serial.print(seed,DEC);           
                         Serial.print(" / ");
                         Serial.println(RF_Rx_Buffer[1],DEC);
#endif
                         seed = RF_Rx_Buffer[1] ;                     //Resync 


                    }  
                    if (RF_Rx_Buffer[1] !=lastseed ) //hopping
                    {
                         //if (seed == 84) seed = 83; // don't like ch 70....

#if (DEBUG_MODE==99)
                         Serial.println("Normal Hopping");
#endif
                         Hopping();    // hop like the wind
                         lastseed = RF_Rx_Buffer[1]; //keep track of last hop 
                         seed--;  
                         seed = seed % 256; 
                         last_hopping_time = time;    
                    }
               }


#endif     

               //              delay(1);
               RF_Mode = Receive;
               Green_LED_OFF;
               Red_LED_OFF;
          } 

#if (DEBUG_MODE == 1)
          if (time%100 < 3) // once a second
          {
               Serial.println("Servo: ");
               for(i = 0; i<16; i++) 
               {

                    Serial.print(int(i));
                    Serial.print(":");
                    Serial.print(Servo_Buffer[i]);
                    Serial.print(" ");
               }
               Serial.println(' ');
               Serial.println( char(RF_Rx_Buffer[0]));
               Serial.println( RF_Rx_Buffer[1],DEC);
          }
#endif  

          /* //######### EXPERIMENTAL CODE PART, DONT USE IT ######## 
           // Telemetry data transmitted.
           if (RF_Mode == Transmitted)
           {
           to_ready_mode(); 
           
           to_rx_mode(); 
           
           Hopping();
           
           RF_Mode = Receive;
           
           last_hopping_time = time;    
           
           }		 
           */

     }


}











