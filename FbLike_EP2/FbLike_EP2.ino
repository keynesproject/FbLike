#include <LedControl.h>
#include <SoftwareSerial.h>
#include <Bounce.h>
#include "WifiMt7681.h"
#include "WifiProcess.h"
#include "MyEEPROM.h"

#ifdef DEBUG
#ifndef DBG(message)
#define DBG(message)    DebugSerial.print(message)
#define DBGL(message)   DebugSerial.println(message)
#define DBGW(message)   DebugSerial.write(message)
#endif
#else
#define DBG(message)
#define DBGL(message)
#define DBGW(message)
#endif // DEBUG


///////////////////////////////////////////////////////////////////
// EEPROM管理
///////////////////////////////////////////////////////////////////
MyEeprom g_Eeprom;

///////////////////////////////////////////////////////////////////
// 按鍵及RESET相關
///////////////////////////////////////////////////////////////////
#define PIN_ORIENTATION 13
Bounce g_BtnReset = Bounce( PIN_ORIENTATION, 50 );
#define PIN_RESET 2

///////////////////////////////////////////////////////////////////
// 七段顯示器相關
///////////////////////////////////////////////////////////////////
byte g_NetState = WIFI_DEFAULT; //七段顯示器顯示的狀態;//
const byte g_Digis = 6;         //七段顯示器位數;//
const int g_DeviceNum = 1;      //使用的 MAX7219 數量;//
#define PIN_DATA 8              //接MAX7219的P1(左上第一隻腳);//
#define PIN_CLK  9              //接MAX7219的P13;//
#define PIN_CS   7              //接MAX7219的P12;//
short g_RunCount = 0;
short g_RunTime = 0;
LedControl g_Led = LedControl( PIN_DATA, PIN_CLK, PIN_CS, g_DeviceNum );

///////////////////////////////////////////////////////////////////
// 網路相關
///////////////////////////////////////////////////////////////////
WifiProcess *g_WifiPro = 0;
WifiServerProcess g_WifiServer;
WifiClientProcess g_WifiClient;
#define PIN_WIFI_ES    5
#define PIN_WIFI_RESET 6

void SetEEPROM()
{ 
/*
   g_Eeprom.ClearEEPROM();
  
   while(1)
   {};
*/
}

void Set7Seg()
{
    //初始化七段顯示器;//
    g_Led.shutdown(0,false);       //The MAX72XX is in power-saving mode on startup
    g_Led.setIntensity(0,15);      // Set the brightness to default value
    g_Led.clearDisplay(0);         // and clear the display
    LedPrintString(F("InIt  "));
}

bool SetWifi()
{ 
    //通電後讓其他模組先運作再開始主程式;//
    //delay(1000);  
     
    bool IsWifiSetup = false;

    String FbQueryID = "";
    String Ssid;
    String PW;

    byte BordState = g_Eeprom.GetBordState();
    if( BordState == 0 || BordState == 1 )
    {      
        g_NetState = WIFI_SET_SERVER;
     
        g_WifiPro = &g_WifiServer;   
        Ssid = "FB_LIKE04";
        PW = "1234567890";
    }
    else
    {
        g_NetState = WIFI_SET_CLIENT;
        
        g_WifiPro = &g_WifiClient;

        //取得 FB ID;// 
        FbQueryID = g_Eeprom.GetFbId();
        
        //取得網路設定資訊;//
        Ssid = g_Eeprom.GetSsid();
        PW = g_Eeprom.GetPassWord();
    }

    //七段顯示器更改狀態畫面;//
    Process7Seg( g_NetState );

    if( !g_WifiPro->Initial( PIN_WIFI_ES, PIN_WIFI_RESET ) )
    {
        DBG("Wifi Init error\n");
        g_NetState = WIFI_ERR_INITIAL;
        return false;
    }

    if( !g_WifiPro->Setup( Ssid, PW,  FbQueryID ) )
    {
        DBG("Wifi Setup error\n");
        g_NetState = WIFI_ERR_SETUP;
        return false;
    }    

    return true;
}

void setup() 
{  
    //初始化EEPROM;//
    SetEEPROM();

    //初始化Reset相關;//
    digitalWrite( PIN_RESET, HIGH );
    delay(200);
    pinMode( PIN_RESET, OUTPUT );

    //初始化按鈕;//
    pinMode( PIN_ORIENTATION, INPUT );
    digitalWrite( PIN_ORIENTATION, LOW );

    //初始化7段顯示器;// 
    Set7Seg();

    //初始化wifi;//
    if( !SetWifi() )
    {
        //七段顯示氣顯示設定錯誤畫面;//
        Process7Seg( g_NetState );

        while(1)
        {
            //等待按鈕重製或是機器斷電重開;//
            ProcessBtn();
        };
    } 
}

void loop() 
{  
    ProcessBtn();

    Process7Seg( g_NetState );

    ProcessNet();
}

void HardwareReset()
{
    DBGL( "System Reboot!" );
    delay(10);
    
    //此行相當於按下Reset鍵,但是REGEIST相關並不會清除;//
    asm volatile ("  jmp 0");

    //此會觸發RESET,相當於斷電;//
    digitalWrite( PIN_RESET, LOW );
}

//顯示指定數字;//
void LedPrintNumber( unsigned long Num ) 
{
    //照順序從個位數往高位數點亮;//
    for( int i=0; i<g_Digis; i++ )
    { 
        g_Led.setDigit( 0, i, (byte)(Num%10), false );
        Num = Num / 10;
    }
}

//顯示字串;//
void LedPrintString( String Str )
{
    if( Str.length() > g_Digis )
        return;        
    
    for( int i=0; i<g_Digis; i++ )
    {
        if( i < Str.length() )
            g_Led.setChar( 0, Str.length()-1-i, Str[i], false );
        else
            g_Led.setChar( 0, i, ' ', false );
    }
}

void Process7Seg( int State )
{
    switch( State )
    {
    //初始化狀態;//
    case WIFI_DEFAULT:
        LedPrintString(F("______"));
        break;

    case WIFI_SET_SERVER:
        LedPrintString(F("SEt  S"));
        break;

    case WIFI_SET_CLIENT:
        LedPrintString(F("SEt  C"));
        break;

    //伺服器狀態等待設定;//
    case WIFI_SERVER_WAIT:
        {
            if( g_RunTime == 3500 )
            {
                if( g_RunCount == 0 )
                {
                    LedPrintString(F("SEr __"));
                }
                else if( g_RunCount == 1 )
                {
                    LedPrintString(F("SEr_ _"));
                }
                else
                {
                    LedPrintString(F("SEr__ "));
                }
                g_RunCount = (g_RunCount+1) % 3;
                g_RunTime = g_RunTime%3500;
            }
            g_RunTime++;
        }
        break;
        
    //Client回傳資料設定完畢;//           
    case WIFI_CLIENT_SETUPED:
        LedPrintString(F("   OFF"));
        break;

    //WIFI 模組初始化失敗;//
    case WIFI_ERR_INITIAL:
        LedPrintString(F("IF E01"));
        break;
    
    //WIFI模組初始化失敗;//
    case WIFI_ERR_SETUP:
        LedPrintString(F("IF E02"));
        break;

   //WIFI模組設定連線資訊失敗;//
    case WIFI_ERR_NO_LINK:
        LedPrintString(F("IF E03"));
        break;
        
    //FB數字顯示;//  
    case WIFI_REQ_SUCESS:    
        LedPrintNumber( g_WifiPro->GetRequestValue( WIFI_REQ_FB_FIELD_NUM ) );
        //LedPrintString( g_WifiPro->GetRequestString( WIFI_REQ_FB_FIELD_NUM ) );
        break;
     
    //FB ID請求錯誤;//  
    case WIFI_ERR_NO_DATA:
        LedPrintString(F("Fb EId"));
        break;

    default:
        break;
    }
}

void ProcessNet()
{
    g_NetState = g_WifiPro->Process();
    if( g_NetState == WIFI_CLIENT_SETUPED )
    {
        //更改;//
        Process7Seg( WIFI_CLIENT_SETUPED );

        //取得設定資訊;//
        String Str = g_WifiPro->GetRequestString( WIFI_REQ_SSID );
        if( Str.length() > 0 )
        {
            g_Eeprom.SetSsid( Str );
            Str = g_WifiPro->GetRequestString( WIFI_REQ_PW );
            if( Str.length() > 0 )
            {
                g_Eeprom.SetPassWord( Str );
                Str = g_WifiPro->GetRequestString( WIFI_REQ_FBID );
                if( Str.length() > 0 )
                {
                    g_Eeprom.SetFbId( Str );

                    //取得完整設定資訊，設定板子狀態;//
                    g_Eeprom.SetBoardState(2);

                    //延遲一段時間確保資料寫入;//
                    delay(1000);

                    //重新啟動 MCU;//
                    HardwareReset();
                }
            } 
        }
    }
}

void ProcessBtn()
{
    g_BtnReset.update();
       
    // has our orientation changed for more the 3 second
    while( g_BtnReset.read() == HIGH )
    {
        g_BtnReset.update();
        
        if ( g_BtnReset.duration() > 3000 ) 
        {  
            LedPrintString(F("  OFF"));
                
            //恢復EEPROM為原始設定;//
            g_Eeprom.ClearEEPROM();
            
            //重新啟動WIFI,這邊會延遲4.5秒;//
            g_WifiPro->Reset();

            //重新啟動 MCU;//
            HardwareReset();
        }
    }
}

