#ifndef __WIFI_PROCESS_H__
#define __WIFI_PROCESS_H__

#include <Arduino.h>
#include "MyWIFI.h"

#define WIFI_REQ_SUCESS     0
#define WIFI_DEFAULT        1
#define WIFI_SERVER_WAIT    2
#define WIFI_CLIENT_SETUPED 3
#define WIFI_ERR_SETUP      4
#define WIFI_ERR_NO_DATA    5
#define WIFI_ERR_FB_REQ     6

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
    
    virtual bool Setup( String Ssid, String PW, String FbID="" ) = 0;
    virtual int  Process() = 0;
    
    virtual int  GetRequestValue( int Type ) = 0; //取得請求欄位的值//
    virtual String  GetRequestString( int Type ) = 0;
 
protected:
    WIFI   m_Wifi;
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
    int  Process();
    
    int  GetRequestValue( int Type ); //取得請求欄位的值//
    String  GetRequestString( int Type );
private:            
    bool ReplyHead();
    bool ReplyPageSetting();     
    bool ReplyPageSucces();
    
    
private:
    String m_ClientSetSSID;
    String m_ClientSetPW;
    String m_ClientSetFBID;    
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
    int  Process();
    
    int  GetRequestValue( int Type ); //取得請求欄位的值//
    String  GetRequestString( int Type );
private:

    bool FbRequest();

    int    m_RequestValue;    
    String m_FbJsonField;  //要請求欄位的名稱;//
    String m_FbHost;
    char   m_FbPort;
    String m_FbID;
    
};

#endif




