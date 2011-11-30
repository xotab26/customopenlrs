//**************************************************************
//**************** thUndeadMODs ********************************
//*    library which includes my Mods for the openLRS
//*   Includes:
//*              - RSSI MOD
//*              - In FLight Fail-Safe
//*              Neat ain't it ? :D
//*              - More stuff will be added
//*
//* Updates here: http://www.fpvuk.org/forum/index.php?topic=3642.0
//*     and here: http://forum.flytron.com/viewtopic.php?f=7&t=207          
//**************************************************************




//work vars
unsigned char i;


//############ Le Stuff :D ########################


void undeadFSwrite(void)
{
                                             	  if (RF_Rx_Buffer[0] == 'F') // thUndead's FS :P
						{
                                             //   Red_LED_OFF;
                                                  for(i = 0; i<8; i++) //Write into the Servo Buffer
                                                        {                                                          
                                                         temp_int = (256*RF_Rx_Buffer[1+(2*i)]) + RF_Rx_Buffer[2+(2*i)];
                                                         if ((temp_int>1500) && (temp_int<4500)) Servo_Buffer[i] = temp_int; 
                                                                                                                  
                                                        }
                                                        save_failsafe_values();
                                                        rx_reset();
                                                        digitalWrite(RED_LED_pin,1);

                                                   Red_LED_ON;


						
						} 
}

void thUndeadRSSI(void)
{
                                 //***************************************
                                //*   thUndead's RSSI MOD
                                //*   info: RSSI voltage according to avg rssi value :D
                                
                                 Rx_RSSI = _spi_read(0x26); // Read the RSSI value
                                 rssicounter++;  //counter which resets after getting to 41

                                 if (rssicounter <= RSSI_SMOOTH) rssipwm= rssipwm + Rx_RSSI ; //adds values into temp buffer
                                 else rssicounter =0;
                               
                                 if (rssicounter == RSSI_SMOOTH) 
                                 {
                                 rssipwm = rssipwm /  RSSI_SMOOTH; // averege 40 rssi values to get stable reading
                                 rssibuf = map(rssipwm,40,RSSI_MAX,10,250);  //map value for pwm: MAX = 2.6v bad rssi unver 1 v
                                 analogWrite(RSSI_OUT,rssibuf);    //write the RSSI voltage

                                 #if defined(Serial_RSSI)    
                                 Serial.print("RSSI:");      //some debugging
                                  //  Serial.println(rssipwm);
                                 Serial.println(rssipwm);
                                 Serial.println(rssibuf);
                                 #endif   
                                 
                                 }
                                 
                                //***************************************        

}


boolean thedelay(int ms)
{
  static long temptime =0 ;
  
  if ((millis() - temptime) >ms)
      {
       temptime = millis();
       return 1;
       
        
      } else return 0;
  
}


