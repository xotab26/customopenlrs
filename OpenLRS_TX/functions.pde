// **********************************************************
// **                   OpenLRS Functions                  **
// **       This Source code licensed under GPL            **
// **********************************************************
// Latest Code Update : 2011-09-26
// Supported Hardware : OpenLRS Tx boards (M1 & M2) (store.flytron.com)
// Project Forum      : http://forum.flytron.com/viewforum.php?f=7
// Google Code Page   : http://code.google.com/p/openlrs/
// **********************************************************

//############# RED LED BLINK #################
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

//############# GREEN LED BLINK #################
void Green_LED_Blink(unsigned short blink_count)
{
     unsigned char i;
     for (i=0;i<blink_count;i++)
     {
          delay(100);
          Green_LED_ON;
          delay(100);
          Green_LED_OFF;
     }
}  

//############# FREQUENCY HOPPING ################# thUndead FHSS
#if (FREQUENCY_HOPPING==1)

boolean thedelay(int ms)
{
     static long temptime =0 ;
     if ((millis() - temptime) >ms)
     {
          temptime = millis();
          return 1;
     } 
     else return 0;
}

void Hopping(void){
     hopping_channel = FHSS[seed]; // Select Channel position from Random Array
     _spi_write(0x79, hop_list[hopping_channel]); // Select Channel from Hoplist and write to RF22

#if (DEBUG_MODE == 5)
     Serial.print("Seed : ");
     Serial.print(int(seed));
     Serial.print(" Hopped to: ");
     Serial.println(int(hop_list[hopping_channel]));
#endif  

}
#endif



//############# BUTTON CHECK #################
void Check_Button(void)
{
     unsigned long loop_time;


     if (digitalRead(BTN)==0) // Check the button
     {
          delay(1000); // wait for 1000mS when buzzer ON 
          digitalWrite(BUZZER, LOW); // Buzzer off

          time = millis();  //set the current time
          loop_time = time; 

          while ((digitalRead(BTN)==0) && (loop_time < time + 4000)) // wait for button reelase if it is already pressed.
          {
               loop_time = millis(); 
          }     

          //Check the button again. If it is already pressed start the binding proscedure    
          if (digitalRead(BTN)==0) // Binding Mode
          {
               randomSeed(analogRead(7)); //use empty analog pin as random value seeder.
               loop_time = millis(); // count the button pressed time for extra randomization

               digitalWrite(BUZZER, HIGH); //100ms single beep for binding mode.
               delay(100);
               digitalWrite(BUZZER, LOW);

               while((digitalRead(BTN)==0) ) {
               }; // wait for button release

               //Here is the code for binding. 
               time = millis();
               Binding_Mode(time-loop_time);  

          }    
          else // if button released, reduce the power for range test.
          {
               Power_Set(0); //set the minimum output power +1dbm
               // ToDo Beeper
          }  


     }


}

//############# BINDING #################
void Binding_Mode(unsigned int btn_press_time)
{

     //randomSeed(analogRead(7)); //use empty analog pin as random value seeder.
     //randNumber = random(300);

     //we will write this part soon


}


//void SetServoPos (unsigned char channel,int value)
//{
//     unsigned char ch = channel*2; // MSB first
//     Servo_Buffer[ch+0] = highByte(value);
//     Servo_Buffer[ch+1] = lowByte(value);
//}
//

