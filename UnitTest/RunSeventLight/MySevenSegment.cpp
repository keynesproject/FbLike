#include "MySevenSegment.h"

MySevenSegment::MySevenSegment (int PinData, int PinClk, int PinCs, byte digits, int NumDevices )
: mLed(  PinData, PinClk, PinCs, NumDevices )
{  
    //初始化七段顯示器;//
    mLed.shutdown(0,false);       //The MAX72XX is in power-saving mode on startup
    mLed.setIntensity(0,7);       // Set the brightness to default value
    mLed.clearDisplay(0);         // and clear the display

    mDigits = digits;

    mCurrentNum = 0;

    mRollTime = 50;

    mServerWaitCount = 0;
    mServerWaitTime;
}

MySevenSegment::~MySevenSegment()
{
      
}

void MySevenSegment::PrintString( String Str )
{
    //字串顯示從輸入的最後一個字往前顯示;//
    for( int i=0; i<mDigits; i++ )
    {
        if( i < Str.length() )
            mLed.setChar( 0, i, Str[Str.length()-1-i], false );
        else
            mLed.setChar( 0, i, ' ', false );
    }
}

void MySevenSegment::PrintNumber( unsigned long Num )
{    
    //照順序從個位數往高位數點亮;//
    for( int i=0; i<mDigits; i++ )
    { 
        mLed.setDigit( 0, i, (byte)(Num%10), false );
        Num = Num / 10;
    }  
}

void MySevenSegment::RollSubtitle( String Str )
{
    memset( mLedNum, ' ', mDigits );
    mLedNum[ mDigits ] = 0;

    for( short i=0; i<Str.length()+mDigits; i++ )
    {
        //將陣列裡的字全部往左移一位;//
        for( short j=1; j<mDigits; j++ )
        {
            mLedNum[ j-1 ] = mLedNum[j];
        }
        if( i<Str.length() )
            mLedNum[mDigits-1] = Str[i];
        else
            mLedNum[mDigits-1] = ' ';

        PrintString( mLedNum );
        delay( 200 );
    }
}

void MySevenSegment::Effect( byte mode, unsigned long Value )
{
    switch( mode )
    {
        case LED_NUM:            
            //若數字相同則不做任何反應;//
            if( mCurrentNum == Value )
                return;
                
            //新數字比舊數字小,直接更改數字不顯示效果;//
            if( mCurrentNum > Value )
            {
                mCurrentNum = Value;
                PrintNumber( mCurrentNum );
                return;
            }

            //單位數滾動;//
            EffectPlusOne( Value );

            //閃動2次;//
            EffectFlicker(2);
            break;

        case LED_SERVER:
            EffectServer();
            break;
    }
}

void MySevenSegment::EffectFlicker( int Times )
{
    for( int i=0; i<Times; i++ )
    {
        delay( 500 );
        mLed.clearDisplay(0);
        delay( 500 );
        PrintNumber( mCurrentNum );
    }
}

//直接從現在數字滾動到指定數字;//
void MySevenSegment::EffectPlusImmediately( unsigned long Num )
{
    //先判斷數字是否有改變;//
    if( mCurrentNum == Num )
        return;
  
    //重新設定數字字串陣列;//
    ResetNumToArray( Num );
    
    //這邊為第一次顯示數字,快速滾動至定位;//
    byte CheckNum=0;
    while( CheckNum < mDigits )
    {
        CheckNum = 0;
        //每一個位數+1若已經相等則不再變動;//
        for( byte i=0; i<mDigits; i++ )
        {
            if( mNewNum[i] != mLedNum[i] )
                mLedNum[i]++;
            else
                CheckNum++;
                  
            if( mLedNum[i] > '9' )
              mLedNum[i] = '0';
        }            
        PrintString( mLedNum );
        delay( mRollTime );
    } 

    //記錄新數字;//
    mCurrentNum = Num;
}

void MySevenSegment::EffectPlusAll( unsigned long Num )
{
    //先判斷數字是否有改變;//
    if( mCurrentNum == Num )
        return;

    //重新設定數字字串陣列;//
    ResetNumToArray( Num );
    
    //先計算有幾個位數不同,在依照最高位數先跑一輪後到定位,在輪下一位數;//
    byte DiffentNum = 0;
    for(byte i=0; i<mDigits; i++ )
    {
        if( mLedNum[i] != mNewNum[i] )
        {
            DiffentNum = mDigits-i;
            break;
        }
    }

    //由不同的數值最高位數一起遞增滾動至指定位數;//
    for( byte i = mDigits - DiffentNum; i<mDigits; i++ )
    {
        //先全部遞增滾動一輪到自己的位數;//
        for( byte j = 0; j<5; j++ )
        {
            for( byte k = i; k<mDigits; k++ )
            {
                mLedNum[k]++;
                if( mLedNum[k] > '9' )
                    mLedNum[k] = '0';
            }

            PrintString( mLedNum );
            delay( mRollTime );
        }

        //將最高位數滾動到指定數字;//
        while( mLedNum[i] != mNewNum[i] )
        {
            for( byte k = i; k<mDigits; k++ )
            {
                mLedNum[k]++;
                if( mLedNum[k] > '9' )
                    mLedNum[k] = '0';
            }
            PrintString( mLedNum );
            delay( mRollTime );
        }
    }

    //記錄新數字;//
    mCurrentNum = Num;
}

void MySevenSegment::EffectPlusOne( unsigned long Num )
{
    //先判斷數字是否有改變;//
    if( mCurrentNum == Num )
        return;

    //重新設定數字字串陣列;//
    ResetNumToArray( Num );

    for( short i = 0; i<mDigits; i++ )
    {
        //最高位數先滾動一輪;//
        for( short j=0; j<10; j++ )
        {
            mLedNum[i]++;
            if( mLedNum[i] > '9' )
                mLedNum[i] = '0';  

            PrintString( mLedNum );
            delay( mRollTime );
        }

        //在滾動到指定位數;//
        while( mLedNum[i] != mNewNum[i] )
        {
            mLedNum[i]++;     
            if( mLedNum[i] > '9' )
                mLedNum[i] = '0';   

            PrintString( mLedNum );
            delay( mRollTime );
        }
    }
    
    //記錄新數字;//
    mCurrentNum = Num;
}

void MySevenSegment::EffectRandom( unsigned long Num )
{
    //先判斷數字是否有改變;//
    if( mCurrentNum == Num )
        return;

    //重新設定數字字串陣列;//
    ResetNumToArray( Num );
}

void MySevenSegment::EffectAround( int Times )
{ 
    short RollTime = 20;
    mLed.clearDisplay(0);
    
    for( byte i=0; i<Times; i++ )
    {   
        for( short j = mDigits-1; j>=0; j-- )
        {
            mLed.setLed( 0, j, 1, true );
            if( j < mDigits - 1 )
                mLed.setLed( 0, j+1, 1, false );
            delay( RollTime );
        }

        mLed.setLed( 0, 0, 2, true );
        mLed.setLed( 0, 0, 1, false );
        delay( RollTime );   
  
        mLed.setLed( 0, 0, 3, true );
        mLed.setLed( 0, 0, 2, false );
        delay( RollTime );
        mLed.setLed( 0, 0, 3, false );

        for( short j = 0; j<mDigits; j++ )
        {
            mLed.setLed( 0, j, 4, true );
            if( j>0 )
              mLed.setLed( 0, j-1, 4, false );
            delay( RollTime );
        }
        mLed.setLed( 0, mDigits-1, 5, true );
        mLed.setLed( 0, mDigits-1, 4, false );
        delay( RollTime );

        mLed.setLed( 0, mDigits-1, 6, true );
        mLed.setLed( 0, mDigits-1, 5, false );
        delay( RollTime );

        mLed.setLed( 0, mDigits-1, 6, false );
    }    
}

void MySevenSegment::EffectServer()
{
    if( mServerWaitTime == 3500 )
    {
        if( mServerWaitCount == 0 )
        {
            PrintString(F("SEr __"));
        }
        else if( mServerWaitCount == 1 )
        {
            PrintString(F("SEr_ _"));
        }
        else
        {
            PrintString(F("SEr__ "));
        }
        mServerWaitCount = (mServerWaitCount+1) % 3;
        mServerWaitTime = mServerWaitTime%3500;
    }
    
    mServerWaitTime++;
}

void MySevenSegment::ResetNumToArray( unsigned long NewNum )
{
    ltoa( NewNum, mNewNum, 10 );
    ArrangeArrary( mNewNum, mDigits );

    ltoa( mCurrentNum, mLedNum, 10 );
    ArrangeArrary( mLedNum, mDigits );
}

void MySevenSegment::ArrangeArrary( char *NumArray, byte Digit )
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

