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
#define RF22B_PWRSTATE_TX            0x08 // TX automatic cleared
#define RF22B_PWRSTATE_RX            0x06 // RX automatic cleared

#define RF22B_PACKET_RECIVED_INTERRUPT    0x02 
#define RF22B_PACKET_SENT_INTERRUPT       0x04 



unsigned char ItStatus1, ItStatus2; 


//typedef struct   
//{ 
//unsigned char reach_1s    : 
//     1; 
//} 
//FlagType; 
//FlagType               Flag; 

//void send_8bit_data(unsigned char i); 
//void send_read_address(unsigned char i); 
//void _spi_write(unsigned char address, unsigned char data); 

//unsigned char read_8bit_data(void); 
//unsigned char _spi_read(unsigned char address); 

//void Write0( void ); 
//void Write1( void ); 
//void Write8bitcommand(unsigned char command); 
//
//void RF22B_init_parameter(void);
//void timer2_init(void); 
//void port_init(void);
//
//void to_sleep_mode(void); 
//void to_tx_mode(void); 
//void to_ready_mode(void); 


//***************************************************************************** 
//***************************************************************************** 

//-------------------------------------------------------------- 
void Write0( void ) 
{ 
     SCK_off;  
     NOP(); 

     SDI_off; 
     NOP(); 

     SCK_on;  
     NOP(); 
} 
//-------------------------------------------------------------- 
void Write1( void ) 
{ 
     SCK_off;
     NOP(); 

     SDI_on;
     NOP(); 

     SCK_on; 
     NOP(); 
} 
//-------------------------------------------------------------- 
void Write8bitcommand(unsigned char command)    // keep sel to low 
{ 
     unsigned char n=8; 
     nSEL_on;
     SCK_off;
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
          if(i&0x80) 
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

     _spi_write(0x1c, 0x02); // Datenrate
     _spi_write(0x20, 0x68);//  0x20 calculate from the datasheet= 500*(1+2*down3_bypass)/(2^ndec*RB*(1+enmanch)) 
     _spi_write(0x21, 0x01); // 0x21 , rxosr[10--8] = 0; stalltr = (default), ccoff[19:16] = 0; 
     _spi_write(0x22, 0x3a); // 0x22    ncoff =5033 = 0x13a9 
     _spi_write(0x23, 0x93); // 0x23 
     _spi_write(0x24, 0x04); // 0x24 
     _spi_write(0x25, 0xD5); // 0x25 
     _spi_write(0x2a, 0x1E); 

     _spi_write(0x30, 0x8c);    // enable packet handler, msb first, enable crc, 
     _spi_write(0x32, 0x0F);    // 0x32address enable for headere byte 0, 1,2,3, receive header check for byte 0, 1,2,3 
     _spi_write(0x33, 0x4a);    // header 3, 2, 1,0 used for head length, fixed packet length, synchronize word length 3, 2, 
     _spi_write(0x34, 0x08);    // 7 default value or   // 64 nibble = 32byte preamble 
     _spi_write(0x35, 0x22);    // synchronize word 
     _spi_write(0x36, 0x2d);    // synchronize word 
     _spi_write(0x37, 0xd4); 
     _spi_write(0x38, 0x00); 
     _spi_write(0x39, 0x00); 
     _spi_write(0x3a, RF_Header[0]);    // tx header 
     _spi_write(0x3b, RF_Header[1]); 
     _spi_write(0x3c, RF_Header[2]); 
     _spi_write(0x3d, RF_Header[3]); 
     _spi_write(0x3e, 0x22);    // total tx 34 byte 

     //RX HEADER
     _spi_write(0x3f, RF_Header[0]);   // check hearder 
     _spi_write(0x40, RF_Header[1]); 
     _spi_write(0x41, RF_Header[2]); 
     _spi_write(0x42, RF_Header[3]); 
     _spi_write(0x43, 0xff);    // all the bit to be checked 
     _spi_write(0x44, 0xff);    // all the bit to be checked 
     _spi_write(0x45, 0xff);    // all the bit to be checked 
     _spi_write(0x46, 0xff);    // all the bit to be checked 

     _spi_write(0x6d, 0x07); // 7 set power max power 
     _spi_write(0x6e, 0x09); //case RATE_57.6K 
     _spi_write(0x6f, 0xD5); //case RATE_57.6K 

     _spi_write(0x70, 0x00);    // disable manchest 
     _spi_write(0x71, 0x23); // Gfsk, fd[8] =0, no invert for Tx/Rx data, fifo mode, txclk -->gpio 
     _spi_write(0x72, 0x30); // frequency deviation setting to 19.6khz (for 38.4kbps)

     _spi_write(0x7a, 0x05); // 50khz step size (10khz x value) // no hopping

     _spi_write(0x75, 0x53); // Band  // Frequenz 433,09 Mhz
     _spi_write(0x76, 0x4d); // Frequenz 433,09 Mhz
     _spi_write(0x77, 0x40); // Frequenz 433,09 Mhz
}


//----------------------------------------------------------------------- 
void rx_mode(void) // Aim for RX Package
{ 
#if (DEBUG_MODE==99)
     Serial.println("rx_mode");
#endif
     ItStatus1 = _spi_read(0x03);      //clear Interupt Status 1
     _spi_write(0x07, 0x02);    // TUNE Mode
     _spi_write(0x08, 0x03);    // clear fifo 
     _spi_write(0x08, 0x00);    // clear fifo 

     //     _spi_write(0x7e, 36);    // threshold for rx almost full, interrupt when 1 byte received 

     _spi_write(0x05, RF22B_PACKET_RECIVED_INTERRUPT); // Aim IRQ at Recived Package
     _spi_write(0x07, RF22B_PWRSTATE_RX );  // to rx mode 


}  
////-----------------------------------------------------------------------    
//
//void to_rx_mode(void) 
//{  
//     to_ready_mode(); 
//     delay(50); 
//     rx_reset(); 
//     NOP(); 
//}  

//-------------------------------------------------------------- 
void tx_mode(void) // Transmit Package, wait to be send OK IRQ
{ 
#if (DEBUG_MODE==99)
     Serial.println("tx_mode");
#endif
     unsigned char i;
     ItStatus1 = _spi_read(0x03);      //clear Interupt Status 1
     _spi_write(0x07, 0x02);    // TUNE Mode
     _spi_write(0x08, 0x03);    // clear fifo 
     _spi_write(0x08, 0x00);    // clear fifo 

     for (i = 0; i<34; i++)  // TX schreiben
     { 
          _spi_write(0x7f, RF_Tx_Buffer[i]); //TODO can be optimised BURST
     } 
     _spi_write(0x05, RF22B_PACKET_SENT_INTERRUPT);  // Aim IRQ at Package sent

     _spi_write(0x07, RF22B_PWRSTATE_TX);    // to tx mode and send 1 package

     while(nIRQ_1); // wait until package send interupt from rf22
     //     
     //     ItStatus1 = _spi_read(0x03);      //clear Interupt Status 1
     //     
     //
     //     rx_mode(); // done automaticaly
}  
////-------------------------------------------------------------- 
//void to_ready_mode(void) 
//{ 
//     ItStatus1 = _spi_read(0x03);   
//     ItStatus2 = _spi_read(0x04); 
//     _spi_write(0x07, RF22B_PWRSTATE_READY); 
//}  
//-------------------------------------------------------------- 
void to_sleep_mode(void) 
{ 
     //  TXEN = RXEN = 0; 
     //LED_RED = 0; 
     _spi_write(0x07, RF22B_PWRSTATE_READY);  

     ItStatus1 = _spi_read(0x03);  //read the Interrupt Status1 register 
     ItStatus2 = _spi_read(0x04);    
     _spi_write(0x07, RF22B_PWRSTATE_POWERDOWN); 

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



