// **********************************************************
// **                   OpenLRS Functions                  **
// **        Developed by Melih Karakelle on 2010-2011     **
// **          This Source code licensed under GPL         **
// **********************************************************
// Latest Code Update : 2011-10-04
// Supported Hardware : OpenLRS Rx boards (store.flytron.com)
// Project Forum      : http://forum.flytron.com/viewforum.php?f=7
// Google Code Page   : http://code.google.com/p/openlrs/


void INIT_SERVO_DRIVER(void)
{
     TCCR1B   =   0x00;   //stop timer
     TCNT1H   =   0x00;   //setup
     TCNT1L   =   0x00;
     ICR1   =   40000;   // used for TOP, makes for 50 hz

     TCCR1A   =   0x02;   
     TCCR1B   =   0x1A; //start timer with 1/8 prescaler for 0.5us PPM resolution

     TIMSK1 = _BV (TOIE1);   
} 

void RFM22B_Int()
{
     if (RF_Mode == Transmit) 
     {
          RF_Mode = Transmitted; 
     } 
     if (RF_Mode == Receive) 
     {
          RF_Mode = Received; 
     }  
}

void Red_LED_Blink(unsigned short blink_count)
{
     unsigned char i;
     for (i=0;i<blink_count;i++)
     {
          delay(100);
          Red_LED_ON;
          delay(100);
          Red_LED_OFF;
     }
}



void load_failsafe_values(){

     for (int i=0;i<16;i++)
     {
          Servo_Buffer[i] =  word(EEPROM.read(11+(2*i)),EEPROM.read(12+(2*i)));
#if (DEBUG_MODE == 4)
     Serial.print(i);
     Serial.print(": ");
     Serial.println(Servo_Buffer[i]); // value x 0.5uS = PPM time, 3000 x 0.5 = 1500uS 
#endif
     }

}


void save_failsafe_values(void){

     for (int i=0;i<16;i++)
     {
          EEPROM.write(11+(2*i),highByte(Servo_Buffer[i])); 
          EEPROM.write(12+(2*i),lowByte(Servo_Buffer[i]));
     } 
}

unsigned char check_modes(void){

     //-- Serial PPM Selection (jumper between Ch1 and ch3)
     pinMode(Servo3_OUT, INPUT); //CH3 input
     digitalWrite(Servo3_OUT, HIGH); // pull up
     digitalWrite(Servo1_OUT, HIGH); // CH1 is HIGH
     delayMicroseconds(1);
     if ( digitalRead(Servo3_OUT) == HIGH) 
     {
          digitalWrite(Servo1_OUT, LOW); // CH1 is LOW
          delayMicroseconds(1);
          if (digitalRead(Servo3_OUT) == LOW) // OK jumper plugged
          {
               pinMode(Servo3_OUT, OUTPUT);
               return  1; //Serial PPM OUT
          }
     }


     pinMode(Servo3_OUT, OUTPUT);

     return  0; // Parallel PPM OUT
}

//############# FREQUENCY HOPPING FUNCTIONS #################
#if (FREQUENCY_HOPPING==1)
void Hopping(void)
{
     hopping_channel = FHSS[seed]; // Select Channel position from Random Array
     _spi_write(0x79, hop_list[hopping_channel]); // Select Channel from Hoplist and write to RF22

#if (DEBUG_MODE == 5)
//     Serial.print("Seed : ");
     Serial.println(int(seed));
//     Serial.print(" Hopped to: ");
//     Serial.println(int(hop_list[hopping_channel]));
#endif  

}
#endif

//void Direct_Servo_Drive(void)
//{
//     Servo_Position[AILERON] = Servo_Buffer[AILERON];  
//     Servo_Position[ELEVATOR] = Servo_Buffer[ELEVATOR];  
//     Servo_Position[THROTTLE] = Servo_Buffer[THROTTLE];  
//     Servo_Position[RUDDER] = Servo_Buffer[RUDDER];  
//     Servo_Position[RETRACTS] = Servo_Buffer[RETRACTS];  
//     Servo_Position[FLAPS] = Servo_Buffer[FLAPS];  
//     Servo_Position[AUX1] = Servo_Buffer[AUX1];  
//     Servo_Position[AUX2] = Servo_Buffer[AUX2];  
//     
//     Servo_Position[AUX3] = Servo_Buffer[AUX3];  
//     Servo_Position[AUX4] = Servo_Buffer[AUX4];  
//     Servo_Position[AUX5] = Servo_Buffer[AUX5];  
//     Servo_Position[AUX6] = Servo_Buffer[AUX6];  
//     Servo_Position[AUX7] = Servo_Buffer[AUX7];  
//     Servo_Position[AUX8] = Servo_Buffer[AUX8];  
//     Servo_Position[AUX9] = Servo_Buffer[AUX9];  
//     Servo_Position[AUX10] = Servo_Buffer[AUX10];  
//
//}    



