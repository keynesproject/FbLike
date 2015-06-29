#include "MyEEPROM.h"

#define EEPROM_SIZE 512
#define EEPROM_STATE_ADDR 0
#define EEPROM_STATE_SIZE 1

#define EEPROM_SSID_ADDR (EEPROM_STATE_ADDR+EEPROM_STATE_SIZE)
#define EEPROM_SSID_SIZE 24
 
#define EEPROM_PW_ADDR (EEPROM_SSID_ADDR+EEPROM_SSID_SIZE)
#define EEPROM_PW_SIZE 24

#define EEPROM_FBID_ADDR (EEPROM_PW_ADDR+EEPROM_PW_SIZE)
#define EEPROM_FBID_SIZE 24

MyEeprom::MyEeprom()
{

}

MyEeprom::~MyEeprom()
{

}

void MyEeprom::SetDefault()
{    
    //設定為Server狀態;//
    eeprom_write_byte( (unsigned char *)EEPROM_STATE_ADDR, 1 );
    
    //設定SSID;//
    String Str = F("FB_LIKE");
    for( int i=0; i<EEPROM_SSID_SIZE; i++ )
    {
        if( i >= Str.length() )
            eeprom_write_byte( (unsigned char *)(EEPROM_SSID_ADDR+i), 0x00 );
        else
            eeprom_write_byte( (unsigned char *)(EEPROM_SSID_ADDR+i), (uint8_t )Str[i] );
    }
    
    //設定密碼;//
    Str = F("0123456789");
    for( int i=0; i<EEPROM_PW_SIZE; i++ )
    {
        if( i >= Str.length() )
            eeprom_write_byte( (unsigned char *)(EEPROM_PW_ADDR+i), 0x00 );
        else
            eeprom_write_byte( (unsigned char *)(EEPROM_PW_ADDR+i), (uint8_t )Str[i] );
    }
    
    //設定FB_ID 清空為0;//
    for( int i=0; i<EEPROM_FBID_SIZE; i++ )
        eeprom_write_byte( (unsigned char *)(EEPROM_FBID_ADDR+i), 0x00 );
}

void MyEeprom::SetFactoryState()
{
    SetDefault();
    
    //設定為出廠未使用狀態;//
    eeprom_write_byte( (unsigned char *)EEPROM_STATE_ADDR, 0 );
}

////////////////////////////////////////////////////////
// 清除EPPROM上的資料
// 並還原成初始狀態(非出廠狀態)
////////////////////////////////////////////////////////
void MyEeprom::ClearEEPROM()
{
    for( int i=0; i<EEPROM_SIZE ; i++ )
    {
        eeprom_write_byte( (unsigned char *)i, (uint8_t)0 );
    }
    
    SetDefault();
}

////////////////////////////////////////////////////////
// 設定主機板狀態
// State: 0	出廠未使用狀態
//        1	Server 狀態
//        2 Client 狀態
//        3 Server - Client 共存 (目前不支援)
////////////////////////////////////////////////////////
void MyEeprom::SetBoardState( byte State )
{
    if( State < 0 || State > 2 )
        return;
    
    eeprom_write_byte( (unsigned char *)EEPROM_STATE_ADDR, (uint8_t)State );
}

////////////////////////////////////////////////////////
// 取得主機板狀態
// State: 0	出廠未使用狀態
//        1	Server 狀態
//        2 Client 狀態
//        3 Server - Client 共存(目前不支援)
////////////////////////////////////////////////////////
byte MyEeprom::GetBordState()
{
    return eeprom_read_byte((unsigned char *)EEPROM_STATE_ADDR);
}

////////////////////////////////////////////////////////
// 設定Wifi連線熱點SSID名稱
// Ssid : 熱點SSID名稱，最長 24 個字元
////////////////////////////////////////////////////////
void MyEeprom::SetSsid( String Ssid )
{
    for( int i=0; i<EEPROM_SSID_SIZE; i++ )
    {
        if( i >= Ssid.length() )
            eeprom_write_byte( (unsigned char *)(EEPROM_SSID_ADDR+i), 0x00 );
        else
            eeprom_write_byte( (unsigned char *)(EEPROM_SSID_ADDR+i), (uint8_t )Ssid[i] );
    }
}

String MyEeprom::GetSsid()
{
    String Ssid;
    char Temp;
    for( int i=0; i<EEPROM_SSID_SIZE; i++ )
    {      
      Temp = eeprom_read_byte( (unsigned char *)(EEPROM_SSID_ADDR+i) );
      if( Temp != 0x00 )
          Ssid += Temp;
      else
          return Ssid;
    }
    
    return Ssid;
}

////////////////////////////////////////////////////////
// 設定Wifi連線熱點 Pass Word
// PW : 熱點密碼，最長 24 個字元
////////////////////////////////////////////////////////
void   MyEeprom::SetPassWord( String PW )
{
    for( int i=0; i<EEPROM_PW_SIZE; i++ )
    {
        if( i >= PW.length() )
            eeprom_write_byte( (unsigned char *)(EEPROM_PW_ADDR+i), 0x00 );
        else
            eeprom_write_byte( (unsigned char *)(EEPROM_PW_ADDR+i), (uint8_t )PW[i] );
    }
}

String MyEeprom::GetPassWord()
{
    String PW;
    char Temp;
    for( int i=0; i<EEPROM_PW_SIZE; i++ )
    {
      Temp = eeprom_read_byte( (unsigned char *)(EEPROM_PW_ADDR+i) );
      if( Temp != 0x00 )
          PW += Temp;
      else
          return PW;
    }    
    return PW;
}

////////////////////////////////////////////////////////
// 設定連接的Facebook ID
// ID : Facebook ID，最長 24 個字元
////////////////////////////////////////////////////////
void   MyEeprom::SetFbId( String ID )
{
    for( int i=0; i<EEPROM_FBID_SIZE; i++ )
    {
        if( i >= ID.length() )
            eeprom_write_byte( (unsigned char *)(EEPROM_FBID_ADDR+i), 0x00 );
        else
            eeprom_write_byte( (unsigned char *)(EEPROM_FBID_ADDR+i), (uint8_t )ID[i] );
    }
}

String MyEeprom::GetFbId()
{
    String ID;
    char Temp;
    for( int i=0; i<EEPROM_FBID_SIZE; i++ )
    {
      Temp = eeprom_read_byte( (unsigned char *)(EEPROM_FBID_ADDR+i) );
      if( Temp != 0x00 )
          ID += Temp;
      else
          return ID;
    }    
    return ID;    
}

