// **********************************************************
// ******************   OpenLRS Tx Code   *******************
// ***  OpenLRS Designed by Melih Karakelle on 2010-2011  ***
// **  an Arudino based RC Rx/Tx system with extra futures **
// **       This Source code licensed under GPL            **
// **********************************************************
// Version Number     : 0.1
// Latest Code Update : 2011-12-05
// Supported Hardware : OpenLRS Tx boards (M1 & M2) (store.flytron.com)
// Project Forum      : http://forum.flytron.com/viewforum.php?f=7
// Google Code Page   : http://code.google.com/p/openlrs/ << Original FW by 
// Google Code page   : https://code.google.com/p/customopenlrs/
// **********************************************************

// ******************** OpenLRS DEVELOPERS ****************** 
// Melih Karakelle (http://www.flytron.com) (forum nick name: Flytron)
// Jan-Dirk Schuitemaker (http://www.schuitemaker.org/) (forum nick name: CrashingDutchman)
// Etienne Saint-Paul (http://www.gameseed.fr) (forum nick name: Etienne) 
// Mihai Andrian aka thUndead (http://www.fpvuk.org/forum/index.php?topic=3642.0)
// Mods by Kermit

//##########################################################
//THIS FW IS A BETA and should be treated with respect!
//Features: 433 Mhz FW GERMANY
//Pseudo Random FHSS on 20ch 
// Hopping freq: 20ms
// 12 channels each 12 bitt
// 4 Channels each 8 Bit
// no telemetry
// FW running @ 28.8 baud to improve range
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

#include <EEPROM.h>

unsigned char FHSS[256];       // 30bytes of FHSS random hopping pattern goodness :D

void setup() {
     //FHSS pattern generator
     randomSeed(FHSSseed);
     for (int i=0;i<256;i++)
          FHSS[i]=random(0,20);

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

//##### PPM INPUT INTERRUPT #####
//Port change interrupt detects the PPM signal's rising edge and calculates the signal timing from Timer1's value.
#if (CONTROL_TYPE == 0)
ISR(PPM_Signal_Interrupt)
{
     unsigned int time_temp;
     unsigned int servo_temp;

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

//--------------------------------------
//############ MAIN LOOP ##############
//--------------------------------------
void loop() 
{
     unsigned char i;

     RF22B_init_parameter(); // init radio module
     frequency_configurator(CARRIER_FREQUENCY); // Calibrate the RF module for this frequency, frequency hopping starts from this frequency.

     sei(); // enable interupt

     // Startup Button Check
     digitalWrite(BUZZER, HIGH);
     digitalWrite(BTN, HIGH);
     Red_LED_ON ;
     delay(100);	
     Check_Button();
     Red_LED_OFF;
     digitalWrite(BUZZER, LOW);

     digitalWrite(RF_OUT_INDICATOR,LOW);
     digitalWrite(PPM_IN,HIGH);

     time = millis();
     old_time = time;

     while(1)
     {    /* MAIN LOOP */


          checkFS();        //thUndead's in flight FS button check

          time = millis();

          if (_spi_read(0x0C)==0) // detect the locked module and reboot
          {
               Red_LED_ON;
               RF22B_init_parameter();
               frequency_configurator(CARRIER_FREQUENCY);
          }




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
          if (time> old_time+20) // Automatic 50hz position transmit code 
               //#endif        
          {
               old_time = time; 
               //Green LED will be on during transmission  


               // Serial Package
               byte total_rx_byte = Serial.available();  // Read the Serial RX buffer size
               if (total_rx_byte>0)
               {
                    RF_Tx_Buffer[0] = 'B';
                    RF_Tx_Buffer[1] = seed-1;
                    if (total_rx_byte>21) total_rx_byte = 21; // Limit the package size as 24-3 byte
                    RF_Tx_Buffer[2]= total_rx_byte;  //lenth of serial package
                    for (byte i=0;i<total_rx_byte;i++) // Fill RX Buffer 3-23
                         RF_Tx_Buffer[3+i] = Serial.read();
               }
               else
               {
                    //checkFS();

                    // Servo stuff
                    if (fscount != 0)
                    {
                         RF_Tx_Buffer[0] = 'F'; // Failsave Package
                    }
                    else
                    {
                         RF_Tx_Buffer[0] = 'S'; // Servo Package
                    }

                    RF_Tx_Buffer[1] = seed-1; //Send next hop target to sync RX with TX :D

                    for(i = 0; i<6; i++) // fill the next 18 rf-tx buffer (2-19) with 12 channel (0-11) 12 bit servo signal
                    {
                         RF_Tx_Buffer[(i*3)+2] = highByte(ServoPos[i*2]<<4);
                         RF_Tx_Buffer[(i*3)+3] = lowByte(ServoPos[i*2]<<4)&highByte(ServoPos[(i*2)+1]);
                         RF_Tx_Buffer[(i*3)+4] = lowByte(ServoPos[(i*2)+1]);
                    }

                    for(i = 12; i<16; i++) // fill thelast 4 rf-tx buffers (20-23) with 4 channels (12-15) 8 bit servo signal
                    {
                         RF_Tx_Buffer[i+8] = lowByte(ServoPos[i]>>4);
                    }
               } 


               // Send the data over RF
               tx_mode();

               //Hop to the next frequency
#if (FREQUENCY_HOPPING==1)
               if (hoppingdelay(30))  // hop every 30ms offers packet redundancy since more packets are sent on freq before hop
               { 
#if (DEBUG_MODE == 5)
                    Serial.print("Hopping from : ");
                    Serial.print(seed,DEC);
#endif
                    seed--;   //going backwards because ThUndeath can :D
                    seed = seed % 256;  //limit hop buff position
#if (DEBUG_MODE == 5)
                    Serial.print(" to: ");
                    Serial.println(seed,DEC);
#endif
                    Hopping(); // hop to the next channel
               }
#endif   


#if (DEBUG_MODE == 1)
               if (time-debugtime< 1000) // once a second
               {
                    debugtime =time;
                    Serial.println("Tx: ");
                    Serial.print( char(RF_Tx_Buffer[0]));
                    for(i = 1; i<24; i++) 
                    {
                         Serial.print( RF_Tx_Buffer[i],DEC);
                         Serial.print(" ");
                    }
                    Serial.println(' ');
               }
#endif  


               //  delay(1); // a delay but not needed now...
          }



     }
}

















