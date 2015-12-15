#ifndef __WIFI_PROCESS_H__
#define __WIFI_PROCESS_H__

#include <Arduino.h>
#include "WifiMt7681.h"

#define WIFI_NONE           0    //無狀態,不處裡任何事物;//
#define WIFI_DEFAULT        1    //初始化狀態;//               ______
#define WIFI_SET_SERVER     2    //設定WIFI為SERVER狀態中;//   SEt  S
#define WIFI_SET_CLIENT     3    //設定WIFI為CLIENT狀態中;//   SEt  C
#define WIFI_SERVER_WAIT    4    //伺服器狀態等待設定;   //    SEr __ -> SEr_ _ -> SEr__
#define WIFI_CLIENT_SETUPED 5    //Client回傳資料設定完畢;//    OFF
#define WIFI_ERR_INITIAL    6    //WIFI模組初始化失敗;  //     IF E01
#define WIFI_ERR_SETUP      7    //WIFI模組設定連線資訊失敗;// IF E02
#define WIFI_ERR_NO_LINK    8    //WIFI模組沒有成功連線到AP;// IF E03
#define WIFI_REQ_SUCESS     9    //FB數字顯示;// 
#define WIFI_ERR_NO_DATA    10   //沒請求到任何數字;//         Fb EId

#define WIFI_REQ_SSID         1
#define WIFI_REQ_PW           2
#define WIFI_REQ_FBID         3
#define WIFI_REQ_FB_FIELD_NUM 4

//////////////////////////////////////////////////////////
// WifiProcess
//////////////////////////////////////////////////////////
class WifiProcess
{
public:
    WifiProcess();
    ~WifiProcess();
    bool Initial( byte PinES, byte PinReset );
    virtual bool Setup( String Ssid, String PW, String FbID="" ) = 0;
    virtual int  Process() = 0;
    
    virtual unsigned long  GetRequestValue( int Type ) = 0; //取得請求欄位的值//
    virtual String  GetRequestString( int Type ) = 0;
 
    void Reset();
protected:
    WifiMt7681  m_Wifi;
    bool m_IsInitial;
    
};


//////////////////////////////////////////////////////////
// WifiServerProcess
// Wifi模組作為Server的程序
//////////////////////////////////////////////////////////
class WifiServerProcess : public WifiProcess
{
public:
    WifiServerProcess();
    ~WifiServerProcess();
    
    bool Setup( String Ssid, String PW, String FbID="" );
    int Process();
    
    unsigned long  GetRequestValue( int Type ); //取得請求欄位的值//
    String  GetRequestString( int Type );
private:            
    void ReplyHead( int HtmlLength = 0 );
    void ReplyPageSetting();     
    void ReplyPageSucces();
        
private:
    String m_ClientSetSSID;
    String m_ClientSetPW;
    String m_ClientSetFBID;    
    String m_ServerIP;
};

//////////////////////////////////////////////////////////
// WifiClientProcess
// //Wifi模組作為Client的程序;//
//////////////////////////////////////////////////////////
class WifiClientProcess : public WifiProcess
{
public:
    WifiClientProcess();
    ~WifiClientProcess();
    
    bool Setup( String Ssid, String PW, String FbID="" );
    int Process();
    
    unsigned long GetRequestValue( int Type ); //取得請求欄位的值//
    String GetRequestString( int Type );
private:
    void FbRequest();

    unsigned long  m_RequestValue; //請求得到的值;//
    String m_RequestStr;           //請求得到的字串;//
    String m_FbField;              //要請求欄位的名稱;//
    String m_FbHost;
    char   m_FbPort;
    String m_FbID;
    short  m_FbDataErrCount;       //向FB請求資料有回應,但沒得到正確的欄位資訊,若此次數大於3次表示FB ID設定有誤;//
    short  m_WifiRequestErrCount;  //向Wifi模組請求無回應計數,若此次數大於3次表示WIFI與AP連線失敗;//
};

#endif




