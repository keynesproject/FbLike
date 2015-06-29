#ifndef __MY_EEPROM__
#define __MY_EEPROM__

#include <Arduino.h>
#include <avr/EEPROM.h>
 
class MyEeprom
{
public:
    MyEeprom();
    ~MyEeprom();
    
    void ClearEEPROM();

    void SetBoardState( byte State );
    byte GetBordState();               //0:出廠未使用狀態 1:Server 狀態 2:Client 狀態;//

    void   SetSsid( String Ssid );
    String GetSsid();

    void   SetPassWord( String PW );
    String GetPassWord();

    void   SetFbId( String ID );
    String GetFbId();
    
private:
    void SetDefault();       //設定預設狀態;//
    void SetFactoryState();  //設定出廠狀態;//
};

#endif

