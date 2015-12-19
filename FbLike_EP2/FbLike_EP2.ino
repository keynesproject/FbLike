#include <LedControl.h>
#include <SoftwareSerial.h>
#include <Bounce.h>
#include <avr/wdt.h>
#include "WifiMt7681.h"
#include "WifiProcess.h"
#include "MyEEPROM.h"
#include "MySevenSegment.h"

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

MySevenSegment g_Led = MySevenSegment( PIN_DATA, PIN_CLK, PIN_CS, g_Digis, g_DeviceNum );

///////////////////////////////////////////////////////////////////
// 網路相關
///////////////////////////////////////////////////////////////////
WifiProcess *g_WifiPro = 0;
WifiServerProcess g_WifiServer;
WifiClientProcess g_WifiClient;
#define PIN_WIFI_ES    5
#define PIN_WIFI_RESET 6

void Set7Seg()
{
    g_Led.RollSubtitle("HELLO");
}

bool SetWifi()
{ 
    //通電後讓其他模組先運作再開始主程式;//
    //delay(1000);  

    String FbQueryID = "";
    String Ssid;
    String PW;

    byte BordState = g_Eeprom.GetBordState();
    if( BordState == 0 || BordState == 1 )
    {      
        g_NetState = WIFI_SET_SERVER;
     
        g_WifiPro = &g_WifiServer;   
        Ssid = "FB_LIKE_DEMO";
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
    //關閉Watchdog;//
    wdt_disable();
    
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

    //Watch dog 監控,當8秒無反應系統自動RESET;//
    wdt_enable( WDTO_8S );
}

void loop() 
{     
    ProcessBtn();

    Process7Seg( g_NetState );

    ProcessNet();

    wdt_reset();
}

void HardwareReset()
{
    wdt_disable();
    
    DBGL( "System Reboot!" );
    delay(10);
    
    //此行相當於按下Reset鍵,但是REGEIST相關並不會清除;//
    //運作原理為程式運作跳到位址0x0000000地方開始執行;//
    //asm volatile ("  jmp 0");

    //此會觸發RESET;//
    //digitalWrite( PIN_RESET, LOW );

    //重新啟動WIFI,這邊會延遲4.5秒;//
    g_WifiPro->Reset();

    //使用watchdog軟體方式重新啟動;//    
    wdt_enable(WDTO_15MS);
    while (1) {}
}

void Process7Seg( int State )
{
    switch( State )
    {
    //不處裡任何事情的狀態;//
    case WIFI_NONE:
        break;
        
    //初始化狀態;//
    case WIFI_DEFAULT:
        g_Led.PrintString(F("______"));
        break;

    case WIFI_SET_SERVER:
        g_Led.PrintString(F("SEt  S"));
        break;

    case WIFI_SET_CLIENT:
        g_Led.PrintString(F("SEt  C"));
        break;

    //伺服器狀態等待設定;//
    case WIFI_SERVER_WAIT:
        g_Led.Effect( LED_SERVER );
        break;
        
    //Client回傳資料設定完畢;//           
    case WIFI_CLIENT_SETUPED:
        //因播放特效有可能較久,所以關閉Watchdog;//
        wdt_disable();
        g_Led.RollSubtitle(F("CLOSE"));
        break;

    //WIFI 模組初始化失敗;//
    case WIFI_ERR_INITIAL:
        g_Led.PrintString(F("IF E01"));
        break;
    
    //WIFI模組設定連線資訊失敗;//
    case WIFI_ERR_SETUP:
        g_Led.PrintString(F("IF E02"));
        break;

   //WIFI模組與ap連線失敗;//
    case WIFI_ERR_NO_LINK:
        g_Led.PrintString(F("IF E03"));
        break;
        
    //FB數字顯示;//  
    case WIFI_REQ_SUCESS:    
        //因播放特效有可能較久,所以關閉Watchdog;//
        wdt_disable();
        
        g_Led.Effect( LED_NUM, g_WifiPro->GetRequestValue( WIFI_REQ_FB_FIELD_NUM ) );

        //啟動Watchdog;//
        wdt_enable( WDTO_8S );
        break;
     
    //FB ID請求錯誤;//  
    case WIFI_ERR_NO_DATA:
        g_Led.PrintString(F("Fb EId"));
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
        g_Eeprom.SetSsid( Str );

        Str = g_WifiPro->GetRequestString( WIFI_REQ_PW );
        g_Eeprom.SetPassWord( Str );

        Str = g_WifiPro->GetRequestString( WIFI_REQ_FBID );
        g_Eeprom.SetFbId( Str );

        //取得完整設定資訊，設定板子狀態;//
        g_Eeprom.SetBoardState(2);

        //延遲一段時間確保資料寫入;//
        delay(500);

        //重新啟動 MCU;//
        HardwareReset();
    }
}

void ProcessBtn()
{
    g_BtnReset.update();
       
    // has our orientation changed for more the 3 second
    while( g_BtnReset.read() == HIGH )
    {
        g_BtnReset.update();
        
        if ( g_BtnReset.duration() > 2000 ) 
        {              
            //七段顯示關閉動畫;//
            Process7Seg( WIFI_CLIENT_SETUPED );

            //恢復EEPROM為原始設定;//
            g_Eeprom.ClearEEPROM();

            //重新啟動 MCU;//
            HardwareReset();
        }
    }
}

