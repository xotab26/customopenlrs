// **********************************************************
// **      RFM22B/Si4432 control functions for OpenLRS     **
// **       This Source code licensed under GPL            **
// **********************************************************
// Latest Code Update : 2011-12-02
// Supported Hardware : OpenLRS Tx/Rx boards (store.flytron.com)
// Project Forum      : http://forum.flytron.com/viewforum.php?f=7
// Origin Code Page   : http://code.google.com/p/openlrs/
// Mods Code Page     : http://code.google.com/p/customopenrc/
// **********************************************************
#define NOP() __asm__ __volatile__("nop") 


#define RF22B_PWRSTATE_POWERDOWN     0x00 
#define RF22B_PWRSTATE_READY         0x01 // TUNE
#define RF22B_PWRSTATE_TX            0x0A // TX automatic cleared
#define RF22B_PWRSTATE_RX            0x06 // RX automatic cleared

#define RF22B_PACKET_RECIVED_INTERRUPT    0x02 
#define RF22B_PACKET_SENT_INTERRUPT       0x04 



unsigned char ItStatus1, ItStatus2; 




//-------------------------------------------------------------- 
void Write0( void ) 
{ 
     SCK_off; 
    NOP(); 
     SDI_off; 
        NOP(); 
     SCK_on;
}
//-------------------------------------------------------------- 
void Write1( void ) 
{ 
     SCK_off;
        NOP(); 
     SDI_on;
        NOP(); 
     SCK_on; 
} 
//-------------------------------------------------------------- 
void Write8bitcommand(unsigned char command)    // keep sel to low 
{ 
     unsigned char n=8; 
     nSEL_on;
        NOP(); 
     SCK_off;
        NOP(); 
     nSEL_off; 
     while(n--) 
     { 
          if(command&0x80) 
               Write1(); 
          else 
               Write0();    
          command = command << 1; 
     } 
     SCK_off;
}  


//-------------------------------------------------------------- 
unsigned char _spi_read(unsigned char address) 
{ 
     unsigned char result; 
     send_read_address(address); 
     result = read_8bit_data();  
     nSEL_on; 
     return(result); 
}  

//-------------------------------------------------------------- 
void _spi_write(unsigned char address, unsigned char data) 
{ 
     address |= 0x80; 
     Write8bitcommand(address); 
     send_8bit_data(data);  
     nSEL_on;
}  

//-------------------------------------------------------------- 
void send_read_address(unsigned char i) 
{ 
     i &= 0x7f; 

     Write8bitcommand(i); 
}  
//-------------------------------------------------------------- 
void send_8bit_data(unsigned char i) 
{ 
     unsigned char n = 8; 
     SCK_off;
     while(n--) 
     { 
          if(i&0x80) // B10000000
               Write1(); 
          else 
               Write0();    
          i = i << 1; 
     } 
     SCK_off;
}  
//-------------------------------------------------------------- 

unsigned char read_8bit_data(void) 
{ 
     unsigned char Result, i; 

     SCK_off;
     Result=0; 
     for(i=0;i<8;i++) 
     {                    //read fifo data byte 
          Result=Result<<1; 
          SCK_on;
          NOP(); 
          if(SDO_1) 
          { 
               Result|=1; 
          } 
          SCK_off;
        NOP(); 
     } 
     return(Result); 
}  
//-------------------------------------------------------------- 

// RFM 22 INIT
void RF22B_init_parameter(void) 
{ 
     ItStatus1 = _spi_read(0x03); // read status, clear interrupt   
     ItStatus2 = _spi_read(0x04); 
     _spi_write(0x06, 0x00);    // no wakeup up, lbd, 
     _spi_write(0x07, 0x02);    // TUNE Mode
     _spi_write(0x09, 0x7f);    // c = 12.5p DEF  
     _spi_write(0x0a, 0x05);    // Clock 2Mhz
     _spi_write(0x0b, 0x12);    // gpio0 TX State
     _spi_write(0x0c, 0x15);    // gpio1 RX State 
     _spi_write(0x0d, 0xfd);    // gpio2 VDD
     _spi_write(0x0e, 0x00);    // gpio    0, 1,2 NO OTHER FUNCTION. DEF

     _spi_write(0x1c, 0x16); // Datenrate
     _spi_write(0x20, 0x45);//  0x20 calculate from the datasheet= 500*(1+2*down3_bypass)/(2^ndec*RB*(1+enmanch)) 
     _spi_write(0x21, 0x01); // 0x21 , rxosr[10--8] = 0; stalltr = (default), ccoff[19:16] = 0; 
     _spi_write(0x22, 0xD7); // 0x22    ncoff =5033 = 0x13a9 
     _spi_write(0x23, 0xDC); // 0x23 
     _spi_write(0x24, 0x07); // 0x24 
     _spi_write(0x25, 0x6E); // 0x25 
     _spi_write(0x2a, 0x1B); 

     _spi_write(0x30, 0x8c);    // enable packet handler, msb first, enable crc, 
     _spi_write(0x32, 0x0E);    // 0x32address enable for headere byte 0, 1,2,3, receive header check for byte 0, 1,2,3 
     _spi_write(0x33, 0x3A);    // header 3, 2, 1,0 used for head length, fixed packet length, synchronize word length 3, 2, 
     _spi_write(0x34, 0x08);    // 8 default value or   // 64 nibble = 32byte preamble 
     _spi_write(0x35, 0x22);    // preamble detection + rssi offset 
     _spi_write(0x36, 0x2d);    // synchronize word 
     _spi_write(0x37, 0xd4);    // synchronize word 
     _spi_write(0x38, 0x00);    // synchronize word 
     _spi_write(0x39, 0x00);    // synchronize word 
     _spi_write(0x3a, RF_Header[0]);    // tx header 
     _spi_write(0x3b, RF_Header[1]); 
     _spi_write(0x3c, RF_Header[2]); 
     _spi_write(0x3d, RF_Header[3]); 
     _spi_write(0x3e, 0x18);    //  tx 24 byte packages

     //RX HEADER
     _spi_write(0x3f, RF_Header[0]);   // check header 
     _spi_write(0x40, RF_Header[1]); 
     _spi_write(0x41, RF_Header[2]); 
     _spi_write(0x42, RF_Header[3]); 
     _spi_write(0x43, 0xff);    // all the bit to be checked 
     _spi_write(0x44, 0xff);    // all the bit to be checked 
     _spi_write(0x45, 0xff);    // all the bit to be checked 
     _spi_write(0x46, 0x00);    // all the bit to be checked 

     //_spi_write(0x6d, 0x07); // 7 set power max power 
     _spi_write(0x6e, 0xEB); //case RATE_28,8K 
     _spi_write(0x6f, 0xEE); //case RATE_28,8K 

     _spi_write(0x70, 0x2C);    // disable manchest 
     _spi_write(0x71, 0x23); // Gfsk, fd[8] =0, no invert for Tx/Rx data, fifo mode, txclk -->gpio 
     _spi_write(0x72, 0x17); // frequency deviation 

     _spi_write(0x7a, 0x05); // 50khz step size (10khz x value) // no hopping

     _spi_write(0x75, 0x53); // Band  // Frequenz 433,09 Mhz
     _spi_write(0x76, 0x4d); // Frequenz 433,09 Mhz
     _spi_write(0x77, 0x40); // Frequenz 433,09 Mhz


#if (DEBUG_MODE==99)
     Serial.println("init-rfm22");  
#endif
}


//----------------------------------------------------------------------- 
void rx_mode(void) // Aim for RX Package
{ 
#if (DEBUG_MODE==90)
     Serial.println("rx_mode");
#endif
     ItStatus1 = _spi_read(0x03);      //clear Interupt Status 1
     _spi_write(0x7e, 24); //FiFo Threshold
     _spi_write(0x08, 0x03);    // clear fifo 
     _spi_write(0x08, 0x00);    // clear fifo 
     _spi_write(0x07, RF22B_PWRSTATE_RX );  // to rx mode 
     _spi_write(0x05, RF22B_PACKET_RECIVED_INTERRUPT); // Aim IRQ at Recived Package

}  

//-------------------------------------------------------------- 
void tx_mode(void) // Transmit Package, wait to be send OK IRQ
{ 
#if (DEBUG_MODE==90)
     Serial.println("tx_mode");
#endif
     unsigned char i;
     ItStatus1 = _spi_read(0x03);      //clear Interupt Status 1
     _spi_write(0x08, 0x03);    // clear fifo 
     _spi_write(0x08, 0x00);    // clear fifo 

     // fifo burst write
     Write8bitcommand(0x7f | 0x80); // select fifo
     for (i = 0; i<24; i++)  // TX schreiben
     { 
          send_8bit_data(RF_Tx_Buffer[i]); 
     }
     nSEL_on; // finish burst

     _spi_write(0x05, RF22B_PACKET_SENT_INTERRUPT);  // Aim IRQ at Package sent

     _spi_write(0x07, RF22B_PWRSTATE_TX);    // to tx mode and send 1 package

     while(nIRQ_1); // wait until package send interupt from rf22
}  

//--------------------------------------------------------------   

void frequency_configurator(long frequency){

     // frequency formulation from Si4432 chip's datasheet
     // original formulation is working with mHz values and floating numbers, I replaced them with kHz values.
     frequency = frequency / 10;
     frequency = frequency - 24000;
     frequency = frequency - 19000; // 19 for 430–439.9 MHz band from datasheet
     // frequency = frequency - 21000; // 21 for 450–459.9 MHz band from datasheet
     frequency = frequency * 64; // this is the Nominal Carrier Frequency (fc) value for register setting

     byte byte0 = (byte) frequency;
     byte byte1 = (byte) (frequency >> 8);

     _spi_write(0x76, byte1);    
     _spi_write(0x77, byte0); 

}

//############# RF POWER SETUP #################
void Power_Set(unsigned short level)
{
     //Power Level value between 0-7
     //0 = +1 dBm
     //1 = +2 dBm
     //2 = +5 dBm
     //3 = +8 dBm
     //4 = +11 dBm
     //5 = +14 dBm
     //6 = +17 dBm
     //7 = +20 dB 
     if (level<8) _spi_write(0x6d, level);  //TODO Beeper when reduced power

}
