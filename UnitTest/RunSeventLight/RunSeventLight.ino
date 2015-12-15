#include <LedControl.h>
#include <Bounce.h>

///////////////////////////////////////////////////////////////////
// 按鍵及RESET相關
///////////////////////////////////////////////////////////////////
#define PIN_ORIENTATION 13
Bounce g_CountBtn = Bounce( PIN_ORIENTATION, 50 );

///////////////////////////////////////////////////////////////////
// 七段顯示器相關
///////////////////////////////////////////////////////////////////
const byte g_Digis = 6;         //七段顯示器位數;//
const int g_DeviceNum = 1;      //使用的 MAX7219 數量;//
#define PIN_DATA 8              //接MAX7219的P1(左上第一隻腳);//
#define PIN_CLK  9              //接MAX7219的P13;//
#define PIN_CS   7              //接MAX7219的P12;//

LedControl g_Led = LedControl( PIN_DATA, PIN_CLK, PIN_CS, g_DeviceNum );

unsigned long g_CurrentNum = 0;
unsigned long g_TestNum = 0;

void setup() 
{
    Serial.begin(9600);
    
    //初始化七段顯示器;//
    g_Led.shutdown(0,false);       //The MAX72XX is in power-saving mode on startup
    g_Led.setIntensity(0,7);      // Set the brightness to default value
    g_Led.clearDisplay(0);         // and clear the display

}


//顯示字串;//
//支援的字元限以下幾種;//
//'0','1','2','3','4','5','6','7','8','9','0',
//'A','b','c','d','E','F','H','L','P',
//'.','-','_',' ' 
void LedPrintString( String Str )
{
    //字串顯示從輸入的最後一個字往前顯示;//
    for( int i=0; i<g_Digis; i++ )
    {
        if( i < Str.length() )
            g_Led.setChar( 0, i, Str[Str.length()-1-i], false );
        else
            g_Led.setChar( 0, i, ' ', false );
    }
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

unsigned long Pow( unsigned long X, unsigned long Y )
{
    unsigned long Temp = 1;
    if( Y == 0 )
        return 1;
        
    for( int i=0; i<Y; i++ )
    {
        Temp *= X;
    }

    return Temp;
}

//整理陣列數字,將字串移至指定位數,不足的前面補字串0,超過的位數則刪除;//
void ArrangeArrary( char *NumArray, byte Digit )
{
    //將原始的數字轉換為字串,並只取個位數開始的指定位數;//     
      byte NumLen = strlen(NumArray);
      if( NumLen < Digit )
      {
          //將字串往右移至指定位數,前頭指定為'0';//
          for( int i=Digit-1, j=0; i>=0; i--, j++  )
          {
              if( j < NumLen )
                  NumArray[ i ] = NumArray[ NumLen - 1 - j ];
              else
                  NumArray[ i ] = '0';
          }
          NumArray[Digit] = 0;
      }
      else if( NumLen > Digit )
      {
          //前面多餘的位數刪除,並把指定的位數字串往左移;//
          for( int i= NumLen-Digit, j=0; i<NumLen; i++, j++ ) 
              NumArray[j] = NumArray[i];
  
          NumArray[Digit] = 0;
      }    
}

void LedEffect( unsigned long Num )
{    
    //先判斷數字是否有改變;//
    if( g_CurrentNum == Num )
        return;

    //新數字比舊數字小,直接更改數字不顯示效果;//
    if( g_CurrentNum > Num )
    {
        g_CurrentNum = Num;
        LedPrintNumber( g_CurrentNum );
    }

    //閃爍間格時間;//
    short ShineTime = 500;
    short RollTime = 100;
  
    //閃兩下;//
    LedPrintString( "" );
    delay( ShineTime );
    LedPrintNumber( g_CurrentNum );
    delay( ShineTime );
    LedPrintString( "" );
    delay( ShineTime );
    LedPrintNumber( g_CurrentNum );
    delay( ShineTime );

    //將新數字轉換成字串處理;//      
    char StrNum[16] ;
    ltoa( Num, StrNum, 10 );
    ArrangeArrary( StrNum, g_Digis );
    
    //將原始的數字轉換為字串,並只取個位數開始的指定位數;//
    char LedNum[16];        
    ltoa( g_CurrentNum, LedNum, 10 );
    ArrangeArrary( LedNum, g_Digis );

    Serial.print( "Current LED Num:" );
    Serial.print( LedNum );
    Serial.print( "      New Num:");
    Serial.println( StrNum );
    
    //若g_CurrentNum為0表示第一次設定,快速滾動至定位;//
    if( g_CurrentNum == 0 )
    {        
        //這邊為第一次顯示數字,快速滾動至定位;//
        byte CheckNum=0;
        while( CheckNum < g_Digis )
        {
            CheckNum = 0;
            //每一個位數+1若已經相等則不再變動;//
            for( byte i=0; i<g_Digis; i++ )
            {
                if( StrNum[i] != LedNum[i] )
                    LedNum[i]++;
                else
                    CheckNum++;
                    
                if( LedNum[i] > '9' )
                  LedNum[i] = '0';
            }            
            LedPrintString( LedNum );
            delay( RollTime );
        } 
    }
    else
    {
        //先計算有幾個位數不同,在依照最高位數先跑一輪後到定位,在輪下一位數;//
        byte DiffentNum = 0;
        for(byte i=0; i<g_Digis; i++ )
        {
            if( LedNum[i] != StrNum[i] )
            {
                DiffentNum = g_Digis-i;
                break;
            }
        }

        //由不同的數值最高位數一起遞增滾動至指定位數;//
        for( byte i = g_Digis - DiffentNum; i<g_Digis; i++ )
        {
            //先全部遞增滾動一輪到自己的位數;//
            for( byte j = 0; j<10; j++ )
            {
                for( byte k = i; k<g_Digis; k++ )
                {
                    LedNum[k]++;
                    if( LedNum[k] > '9' )
                        LedNum[k] = '0';
                }

                LedPrintString( LedNum );
                delay( RollTime );
            }

            //將最高位數滾動到指定數字;//
            while( LedNum[i] != StrNum[i] )
            {
                for( byte k = i; k<g_Digis; k++ )
                {
                    LedNum[k]++;
                    if( LedNum[k] > '9' )
                        LedNum[k] = '0';
                }
                LedPrintString( LedNum );
                delay( RollTime );
            }
        }
    }

    //閃兩下;//
    LedPrintString( "" );
    delay( ShineTime );
    LedPrintNumber( Num );
    delay( ShineTime );
    LedPrintString( "" );
    delay( ShineTime );
    LedPrintNumber( Num );

    //記錄新數字;//
    g_CurrentNum = Num;
}

void ProcessBtn()
{
    g_CountBtn.update();
       
    // has our orientation changed for more the 3 second
    while( g_CountBtn.read() == HIGH )
    {
        g_CountBtn.update();
        
        if ( g_CountBtn.duration() > 100 ) 
        {  
            g_TestNum+=6;
            break;
            //g_TestNum++;
        }
    }
}

void loop() 
{
    ProcessBtn();

    Process7Seg();
}

void Process7Seg( )
{
    LedEffect( g_TestNum );
}

