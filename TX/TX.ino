// **********************************************************
// ******************   OpenLRS Tx Code   *******************
// ***  OpenLRS Designed by Melih Karakelle on 2010-2011  ***
// **  an Arudino based RC Rx/Tx system with extra futures **
// **       This Source code licensed under GPL            **
// **********************************************************
// Version Number     : 1.10.1
// Latest Code Update : 2011-11-18
// Supported Hardware : OpenLRS Tx boards (M1 & M2) (store.flytron.com)
// Project Forum      : http://forum.flytron.com/viewforum.php?f=7
// Google Code Page   : http://code.google.com/p/openlrs/ << Original FW by Melih
// **********************************************************

// ******************** OpenLRS DEVELOPERS ****************** 
// Melih Karakelle (http://www.flytron.com) (forum nick name: Flytron)
// Jan-Dirk Schuitemaker (http://www.schuitemaker.org/) (forum nick name: CrashingDutchman)
// Etienne Saint-Paul (http://www.gameseed.fr) (forum nick name: Etienne) 
// Mihai Andrian aka thUndead (http://www.fpvuk.org/forum/index.php?topic=3642.0)
// Mods by Kermit

//##########################################################
//THIS FW IS A BETA and should be treated with respect!
//Features: 435 Mhz FW
//Pseudo Random FHSS on 20ch 
// Hopping freq: 20ms
// 
// FW running @ 38.4 baud to improve range
// Any questions here: http://www.fpvuk.org/forum/index.php?topic=3642.0
//
//##################################################################
// Last but not least, 
//a lot of hours and nights went into this firmware 
//so you can buy me some Pringles and a Red Bull to keep me going :D 
//                 http://tinyurl.com/425x4hj
// Thanks to all who kindly provided me with Pringles :D
//##################################################################

//##########################################################
// README
// Change FHSSseed 89 in the config.h section for a unique hopping sequence
//##########################################################



#include "config.h"
#include "tVAR.h"

#include <EEPROM.h>

unsigned char FHSS[256];       // 30bytes of FHSS random hopping pattern goodness :D

void setup() {   
     randomSeed(FHSSseed);  
     for (int i=0;i<256;i++)                   //FHSS pattern generator
     {

          FHSS[i]=random(0,20);       //           //Pseudo Random Number generator
     }


     //RF module pins
     pinMode(SDO_pin, INPUT); //SDO
     pinMode(SDI_pin, OUTPUT); //SDI        
     pinMode(SCLK_pin, OUTPUT); //SCLK
     pinMode(IRQ_pin, INPUT); //IRQ
     pinMode(nSel_pin, OUTPUT); //nSEL

     //LED and other interfaces
     pinMode(Red_LED, OUTPUT); //RED LED
     pinMode(Green_LED, OUTPUT); //GREEN LED
     pinMode(BUZZER, OUTPUT); //Buzzer
     pinMode(BTN, INPUT); //Buton


     pinMode(PPM_IN, INPUT); //PPM from TX 
     pinMode(RF_OUT_INDICATOR, OUTPUT);

     Serial.begin(SERIAL_BAUD_RATE);
     Power_Set(0); //rf power max 7


#if (CONTROL_TYPE == 0)
     PPM_Pin_Interrupt_Setup // turnon pinchange interrupts
     TCCR1B   =   0x00;   //stop timer
     TCNT1H   =   0x00;   //setup
     TCNT1L   =   0x00;
     ICR1     =   60005;   // used for TOP, makes for 50 hz
     TCCR1A   =   0x02;   
     TCCR1B   =   0x1A; //start timer with 1/8 prescaler for measuring 0.5us PPM resolution
#endif


     //---------------------------- Servo Init

     for (int i=0;i<16;i++) // set default High Resulution servo position values.
          ServoPos[i]=2048; // set the center position



} //--End Setup


#if (CONTROL_TYPE == 0) 
//##### PPM INPUT INTERRUPT #####
//Port change interrupt detects the PPM signal's rising edge and calculates the signal timing from Timer1's value.

ISR(PPM_Signal_Interrupt){

     unsigned int time_temp;
     unsigned int servo_temp;
     unsigned int servo_temp2;

     if (PPM_Signal_Edge_Check) // Only works with rising edge of the signal
     {
          time_temp = TCNT1; // read the timer1 value
          TCNT1 = 0; // reset the timer1 value for next
          if (channel_no<16) channel_no++; 
          {

               if (time_temp > 8000) // new frame detection : >4ms LOW
               {	
                    channel_count = channel_no;
                    channel_no = 0;
                    transmitted = 0;                               
               }
               else
               {
                    if ((time_temp>=952) && (time_temp<=5047)) // check the signal time and update the channel if it is between 476us-2523,5us
                    {
                         ServoPos[channel_no-1]=time_temp-952; // Normieren auf 0- 4095 ( 12 bit)
                    }
               }
          }
     }

}
#endif 

//############ MAIN LOOP ##############
void loop() {

     unsigned char i;

     //wdt_enable(WDTO_1S);

     RF22B_init_parameter(); 
     frequency_configurator(CARRIER_FREQUENCY); // Calibrate the RF module for this frequency, frequency hopping starts from this frequency.

     sei();

     digitalWrite(BUZZER, HIGH);
     digitalWrite(BTN, HIGH);
     Red_LED_ON ;
     delay(100);	

     Check_Button();

     Red_LED_OFF;
     digitalWrite(BUZZER, LOW);

     digitalWrite(RF_OUT_INDICATOR,LOW);
     digitalWrite(PPM_IN,HIGH);

     transmitted = 0;
     rx_mode;

     time = millis();
     old_time = time;

     while(1)
     {    /* MAIN LOOP */


          undeadFS();        //thUndead's in flight FS button check

          time = millis();

          if (_spi_read(0x0C)==0) // detect the locked module and reboot
          {
               Red_LED_ON;
               RF22B_init_parameter();
               frequency_configurator(CARRIER_FREQUENCY);
               rx_mode;
          }

          //#if (TELEMETRY_ENABLED==1)
          //
          //          if (nIRQ_0) //TODO Statemachine
          //          {
          //               Red_LED_ON;  
          //               send_read_address(0x7f); // Send the package read command
          //               for(i = 0; i<17; i++) //read all buffer 
          //               { 
          //                    RF_Rx_Buffer[i] = read_8bit_data(); 
          //               }  
          //               //               rx_reset(); 
          //
          //     #if (TELEMETRY_MODE == 1)  // OpenLRS Standard Telemetry mode                
          //          #if (Rx_RSSI_Alert_Level>0) 
          //               Tx_RSSI = ((Tx_RSSI/5)*4) + (_spi_read(0x26)/5); // Read the RSSI value
          //               //#endif
          //               //#if (Rx_RSSI_Alert_Level>0) 
          //               Rx_RSSI = ((Rx_RSSI/5)*4) + (RF_Rx_Buffer[1]/5); // Rx Rssi value from telemetry data
          //          #endif
          //               if ((Rx_RSSI < Rx_RSSI_Alert_Level)||(Tx_RSSI < Tx_RSSI_Alert_Level)) // RSSI level alerts
          //                    digitalWrite(BUZZER, HIGH);
          //               else
          //                    digitalWrite(BUZZER, LOW);
          //          #if (TELEMETRY_OUTPUT_ENABLED==1)
          //               for(i = 0; i<16; i++) //write serial
          //                    Serial.print(RF_Rx_Buffer[i]);
          //               Serial.println(int(RF_Rx_Buffer[16]));
          //          #endif                  
          //     #endif
          //
          //     #if (TELEMETRY_MODE == 0)  // Transparent Bridge Telemetry mode                
          //          #if (TELEMETRY_OUTPUT_ENABLED==1)
          //               if (RF_Rx_Buffer[0]=='B') // Brige values
          //               {
          //                    for(i = 2; i<RF_Rx_Buffer[1]+2; i++) //write serial
          //                         Serial.print(RF_Rx_Buffer[i]);
          //               }   
          //          #endif                  
          //     #endif
          //
          //               Red_LED_OFF;
          //               Rx_Pack_Received = 0;
          //          }	    
          //#endif

#if (CONTROL_TYPE==1) //TODO Umwandeln in plaintext eingabe
          if (Serial.available()>3)    // Serial command received from the PC
          {
               int cmd = Serial.read();
               if (cmd=='S')            // Command 'S'+ channel number(1 bytes) + position (2 bytes)
               {
                    Red_LED_ON;
                    int ch = Serial.read();
                    Servo_Buffer[2*ch] = Serial.read();
                    Servo_Buffer[(2*ch)+1] = Serial.read();
                    Red_LED_OFF;
               }
          }
#endif       

          //#if (CONTROL_TYPE == 0)
          //          if ((transmitted==0))// && (channel_count>3) && (channel_count<16))
          //#else              
          if (time> old_time+10) // Automatic 50hz position transmit code 
               //#endif        
          {
               old_time = time; 
               //Green LED will be on during transmission  
               Green_LED_ON ;


#if (TELEMETRY_MODE == 0) // Transparent Brige ground to air code

               byte total_rx_byte = Serial.available();  // Read the Serial RX buffer size
               if (total_rx_byte>0)
               {
                    RF_Tx_Buffer[0] = 'B';
                    RF_Tx_Buffer[1] = seed-1;
                    if (total_rx_byte>21) total_rx_byte = 21; // Limit the package size as 24-3 byte
                    RF_Tx_Buffer[2]= total_rx_byte;
                    for (byte i=0;i<total_rx_byte;i++)
                         RF_Tx_Buffer[3+i] = Serial.read();
               }
               else   
#endif  

               // Servo stuff
               if (fscount == 0)  // check if FS was initialised
               {
                    RF_Tx_Buffer[0] = 'S';
                    RF_Tx_Buffer[1] = seed-1;     //Send next hop target to sync RX with TX :D

                    //Serial.println( RF_Tx_Buffer[0],DEC);

                    for(i = 0; i<6; i++) // fill the next 18 rf-tx buffer (2-19) with 12 channel (0-11) 12 bit servo signal
                    {
                         RF_Tx_Buffer[(i*3)+2] = highByte(ServoPos[i*2]<<4);
                         RF_Tx_Buffer[(i*3)+3] = lowByte(ServoPos[i*2]<<4)&highByte(ServoPos[(i*2)+1]);
                         RF_Tx_Buffer[(i*3)+4] = lowByte(ServoPos[(i*2)+1]);
                         //     Serial.println(Servo_Buffer[i],DEC);
                    }

                    for(i = 12; i<16; i++) // fill thelast 4 rf-tx buffers (20-23) with 4 channels (12-15) 8 bit servo signal
                    {
                         RF_Tx_Buffer[i+8] = lowByte(ServoPos[i]>>4);
                         //     Serial.println(Servo_Buffer[i],DEC);
                    }
               } 
               else thUndeadFS();



               // Send the data over RF
               tx_mode();
               transmitted = 1;

               //Green LED will be OFF
               Green_LED_OFF; 


               //Hop to the next frequency
#if (FREQUENCY_HOPPING==1)
               if (thedelay(30))  // hop every 20ms :D offers packet redundancy since more packets are soent on freq before hop
               {

                    seed--;   //going backwards because I can :D
                    seed = seed % 256;  //limit hop buff position

                    //if (seed ==71) seed  = 70; //doesn't like ch 70 in the PN Sequence for some reason so jump over
                    Hopping();
               }
#endif   


               //#if (TELEMETRY_ENABLED==1) //Receiver mode enabled for the telemetry
               //                    rx_mode(); 
               //#if (Lost_Package_Alert != 0) // Lost Package Alert 
               //                    if (Rx_Pack_Received < Lost_Package_Alert) // last Rx packs didnt received
               //                         digitalWrite(BUZZER, LOW);
               //                    else
               //                         digitalWrite(BUZZER, HIGH);
               //#endif   
               //                    Rx_Pack_Received++;
               //#endif


#if (DEBUG_MODE == 1)
               if (time%100 < 10) // once a second
               {
                    Serial.println("Servo: ");
                    for(i = 0; i<16; i++) 
                    {

                         Serial.print(int(i));
                         Serial.print("-");
                         Serial.print(int( (Servo_Buffer[((2*i))]*256) + Servo_Buffer[1+(2*i)]));
                         Serial.print(" ");
                    }
                    Serial.println(' ');
                    Serial.println( char(RF_Tx_Buffer[0]));
                    Serial.println( RF_Tx_Buffer[1],DEC);
               }
#endif  


               //  delay(1); // a delay but not needed now...
          }



     }
}














