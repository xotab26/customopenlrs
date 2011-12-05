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

//**************** In-Flight FAIL-SAFE *************************

void undeadFS(void)        //button check 
{
     if ((bzzz!=0)&&( (millis()-bzzz) >= 2000))
     {

          digitalWrite(BUZZER, LOW); // Buzzer off 
          bzzz =0;

#if (DEBUG_MODE == 6)
          Serial.println("Stopped Buzzing....");
#endif
     } //else Serial.println("Still Buzzing...");


     if (digitalRead(BTN)==0)
     {
          if (fstime==0)
          {
               fstime =millis();
               //    Serial.println(fstime);

          } 
          else if ((millis() - fstime) >= 1500 )
          {
#if (DEBUG_MODE == 6)
               Serial.println("FS Engaged!!!!!!!!! ");
#endif

               fstime =0;
               fscount =10;
          }
     } 
     else 
     {
          fstime =0;
     }
}

void thUndeadFS(void)            //actual FS sending func
{

     if ((fscount <=10)&&(fscount >= 0))

     {

          RF_Tx_Buffer[0] = 'F';
          RF_Tx_Buffer[1] = seed-1;
          for(i = 0; i<16; i++) // fill the rf-tx buffer with 8 channel (2x8 byte) servo signal
          {
               RF_Tx_Buffer[i+2] = Servo_Buffer[i];
          }
          fscount--;

#if (DEBUG_MODE == 6)
          Serial.println(fscount);
#endif

          if (fscount ==0)
          {
               bzzz = millis();
#if (DEBUG_MODE == 6)
               Serial.println("Started buzzing....");
#endif
               digitalWrite(BUZZER, HIGH);
          }


     } 
}



