#include <msp430.h>

//TM1638模块引脚定义
#define DIO BIT5
#define CLK BIT4
#define STB BIT3
#define IO_OUT P1OUT
//#define DQ BIT0 //采样口为 P1.0




//共阴数码管显示代码
unsigned char tab[]={
0x3F,0x06,0x5B,0x4F,
0x66,0x6D,0x7D,0x07,
0x7F,0x6F,0x40,0x7C,
0x39,0x5E,0x00,0x3e};

unsigned int count=0,jud=0,ws=1;
int loc=0;
unsigned int  s,q,w,e,r,u,p;
long int efv,max,min;
unsigned int table_Valu[64];


unsigned char num[8];                //各个数码管显示的值
unsigned int show=270,dub=150;       //show是温度报警值2.7,   dub是电压报警值1.5v
void Write_DATA(unsigned char add,unsigned char DATA);
void init_TM1638();
void temperature();
void Write_allLED(unsigned char LED_flag);
int temper();
int DS18B20_INIT(void);
void DS18B20_Write();
int DS18B20_Read();
void TM1638_Write(unsigned char DATA);
void Write_COM(unsigned char cmd);
unsigned char TM1638_Read();
unsigned char Read_key();

void sin();
int c;
int x=0,y=0,z=0,t=0;
//unsigned char alert_T=300;
//unsigned int return_flag;

int main(void)
{   WDTCTL = WDTPW + WDTHOLD;

P1DIR|= DIO + CLK + STB;
unsigned char i;
unsigned long int sum;    //温度
unsigned int temp[3];     //温度的三个位
init_TM1638();                             //初始化TM1638
for(i=0;i<8;i++)
    Write_DATA(i<<1,tab[0]);                  //初始化寄存器
//sin();
P1DIR&=~BIT7;

P2DIR |= BIT5;
/*1.2口，温度采样
1.7口电压采样
2.5 蜂鸣器
*/
P2OUT&=~BIT5;  //关闭蜂鸣器初始化

while(1)
{
    s=Read_key();                          //读按键值0~7对应s1`s8
    while(Read_key()==s);            //消抖和去干扰，后面也会用到

    if(s==0)
    {   count++;
    }
    jud=count%3;
    while(jud==1)//按下一次
    {
        sum=DS18B20_Conert();
        sum=sum*0.625;
        temp[0]=(sum)%10;
        temp[1]=((sum)/10)%10;
        temp[2]=((sum)/100)%10;
        //温度显示
        Write_DATA(4*2,tab[12]);   //"C"
        Write_DATA(7*2,tab[temp[0]]);   //0.1位
        Write_DATA(6*2,tab[temp[1]]|0x80);   //1.位
        Write_DATA(5*2,tab[temp[2]]);   //10位
        ADC10_init();
        ADC10_WaveSample();

        efv = (max)*100/266;  //efv电压有效值，下面是输出

        Write_DATA(0*2,tab[15]);   //"U"
        Write_DATA(1*2,tab[efv/100]|0x80);
        Write_DATA(2*2,tab[(efv/10)%10]);
        Write_DATA(3*2,tab[efv%10]);

        if(sum>show||efv>dub)
            P2OUT|=BIT5;
        else
            P2OUT&=~BIT5;
        Write_allLED(0); //上面所有二极管LED熄灭
        s=Read_key();
        if(s==0)
            break;
    }

    if(jud==2)   //按下第二次S1
    {
//报警值的设定
        Write_DATA(0*2,tab[14]);  //熄灭
        Write_DATA(1*2,tab[12]);    //‘C’
        Write_DATA(2*2,tab[show/100]);  // show的各位数
        Write_DATA(3*2,tab[(show/10)%10]);
        Write_DATA(4*2,tab[14]);
        Write_DATA(5*2,tab[15]);       //'U'
        Write_DATA(6*2,tab[dub/100]|BIT7);
        Write_DATA(7*2,tab[(dub/10)%10]);


        while(1)
        {
            if(sum>show||efv>dub)
                        P2OUT|=BIT5;
                    else
                        P2OUT&=~BIT5;       //2.5报警

            s=Read_key();                          //读按键值
            while(Read_key()==s);
            if(s==0)
                {   count++;
                jud=count%3;
                break;
                }
            if(s==5)    //复位键s6
            {
             show=270;
             dub=150;
             Write_DATA(0,tab[14]);
             Write_DATA(2,tab[12]);
             Write_DATA(4,tab[show/100]);
             Write_DATA(6,tab[(show/10)%10]);
             Write_DATA(8,tab[14]);
             Write_DATA(10,tab[15]);
             Write_DATA(12,tab[dub/100]|BIT7);
             Write_DATA(14,tab[(dub/10)%10]);
            }
            if(s==2)       //右移
            {
                loc ++;     // loc是led的位码
                ws=2*ws;   //第ws个LED亮

                Write_allLED(ws);  //
                //s = Read_key();
                //while(Read_key()==s);
            }

            if(s==1)        //左移
            {
                loc --;
                ws=ws/2;
                Write_allLED(ws);
                //s = Read_key();
                //while(Read_key()==s);
            }
            if(loc>7)  {
            loc=7;
            ws=0b10000000;
            }
            if(loc<0)  {
            loc=0;
            ws=0b00000001;
            }
            if(s==4)         //减   loc只能是  2  3    6  7 对应位码
            {
                if(loc==2)
                {
                    if((show/100)==0)
                        {show=show+900;
                        Write_DATA(2*loc,tab[9]);}
                    else
                        {show=show-100;
                        Write_DATA(2*loc,tab[show/100]);}
                }
                if(loc==3)
                {
                    unsigned int wy1;
                    wy1=(show/10)%10;
                    if(wy1==0)
                    {show=show+90;
                        Write_DATA(2*loc,tab[9]);} // 9
                    else
                    {show=show-10;
                    Write_DATA(2*loc,tab[((show/10)%10)]);}
                }
                if(loc==6)
                {

                    if((dub/100)==0)
                    {dub=dub+900;
                        Write_DATA(2*loc,tab[9]|BIT7);}
                    else
                    {dub=dub-100;
                        Write_DATA(2*loc,tab[dub/100]|BIT7);}

                }
                if(loc==7)
                {
                    unsigned int wy2;
                    wy2=(dub/100)%10;
                    if(wy2==0)
                    {dub=dub+90;
                        Write_DATA(2*loc,tab[9]);}
                    else
                    {dub=dub-10;
                    Write_DATA(2*loc,tab[(dub/10)%10]);}

                }


            }

            if(s==3)         //加
            {
                if(loc==2)
                {
                    if((show/100)==9)
                    {show=show-900;
                        Write_DATA(2*loc,tab[0]);}
                    else
                    {show=show+100;
                        Write_DATA(2*loc,tab[show/100]);}

                }
                if(loc==3)
                {
                    unsigned int wy3;
                    wy3=(show/10)%10;
                    if(wy3==9)
                    {show=show-90;
                        Write_DATA(2*loc,tab[0]);}
                    else
                    {show=show+10;
                        Write_DATA(2*loc,tab[(show/10)%10]);}

                }
                if(loc==6)
                {
                    if(dub/100==9)
                    {dub=dub-900;
                        Write_DATA(2*loc,tab[0]|BIT7);}
                    else
                        {dub=dub+100;
                        Write_DATA(2*loc,tab[dub/100]|BIT7);}

                }
                if(loc==7)
                {
                    unsigned int wy4;
                    wy4=(dub/10)%10;
                    if(wy4==9)
                    {dub=dub-90;
                        Write_DATA(2*loc,tab[0]);}
                    else
                    {dub=dub+10;
                        Write_DATA(2*loc,tab[(dub/10)%10]);}

                }


            }
        }

    }


    if(jud==0)
    {
        Write_allLED(0);
    Write_DATA(0,tab[14]);
    Write_DATA(2,tab[6]);
    Write_DATA(4,tab[0]);
    Write_DATA(6,tab[5]);
    Write_DATA(8,tab[10]);
    Write_DATA(10,tab[0]);
    Write_DATA(12,tab[1]);
    Write_DATA(14,tab[14]);
    }

}
}


void TM1638_Write(unsigned char DATA)           //写数据函数
{
    P1DIR |= DIO;
    unsigned char i;
    for(i=0;i<8;i++)
    {
        P1OUT &=~CLK;
        if(DATA&0X01)
        {P1OUT |= DIO;}
        else
        {P1OUT &= ~DIO;}
        DATA>>=1;
        P1OUT |= CLK;
    }
}
unsigned char TM1638_Read(void)                 //读数据函数
{
    P1DIR &= ~DIO;
    unsigned char i;
    unsigned char temp=0;
    P1DIR &= ~DIO ;  //设置为输入
    for(i=0;i<8;i++)
    {
        temp>>=1;
        P1OUT &= ~CLK;
        if(P1IN & DIO)
            temp|=0x80;
        P1OUT |=CLK;
    }
    return temp;
}
void Write_COM(unsigned char cmd)       //发送命令字
{
    P1OUT &= ~STB;
    TM1638_Write(cmd);
    P1OUT |= STB;
}
unsigned char Read_key(void)
{
    P1DIR &= ~DIO;
    unsigned char c[4],i,key_value=0;
    IO_OUT&=~STB;
    TM1638_Write(0x42);                //读键扫数据 命令
    for(i=0;i<4;i++)
        c[i]=TM1638_Read();
    IO_OUT|=STB;                             //4个字节数据合成一个字节
    for(i=0;i<4;i++)
        key_value|=c[i]<<i;
    for(i=0;i<8;i++)
        if((0x01<<i)==key_value)
            break;
    return i;
}
void Write_DATA(unsigned char add,unsigned char DATA)       //指定地址写入数据
{
    Write_COM(0x44);
    P1OUT &=~STB;
    TM1638_Write(0xc0|add);
    TM1638_Write(DATA);
    P1OUT |= STB;
}

void Write_allLED(unsigned char LED_flag)                   //控制全部LED函数，LED_flag表示各个LED状态
{

    unsigned char i;
    for(i=0;i<8;i++)
    {
        if(LED_flag&(1<<i))
            //Write_DATA(2*i+1,3);
            Write_DATA(2*i+1,1);
        else
            Write_DATA(2*i+1,0);
    }
}

//TM1638初始化函数
void init_TM1638(void)
{
    unsigned char i;
    Write_COM(0x8a);       //亮度 (0x88-0x8f)8级亮度可调
    Write_COM(0x40);       //采用地址自动加1
    P1OUT &=~STB;                 //
    TM1638_Write(0xc0);    //设置起始地址

    for(i=0;i<16;i++)      //传送16个字节的数据
        TM1638_Write(0x00);
    P1OUT |=STB;
}
void DS18B20_init();                              //DS18B20初始化
unsigned char DS18B20_Reset();                    //DS18B20复位
void DS18B20_WriteData(unsigned char);            //写数据到DS18B20
unsigned char DS18B20_ReadData();                 //读数据
extern unsigned int DS18B20_Conert();                    //转换温度

#define DS18B20_DIR P1DIR
#define DS18B20_OUT P1OUT
#define DS18B20_IN P1IN
#define DS18B20_DQ BIT2

#define DS18B20_H   DS18B20_OUT|=DS18B20_DQ        //DQ置位
#define DS18B20_L   DS18B20_OUT&=~DS18B20_DQ       //DQ复位

#define DQ_IN       DS18B20_DIR &= ~DS18B20_DQ     //设置DQ为输入
#define DQ_OUT      DS18B20_DIR |= DS18B20_DQ      //设置DQ为输出
#define READ_DQ     (DS18B20_IN&DS18B20_DQ)        //读DQ电平

#define CPU_F ((double)1000000)
#define delay_us(x) __delay_cycles((long)(CPU_F*(double)x/1000000.0))
#define delay_ms(x) __delay_cycles((long)(CPU_F*(double)x/1000.0))

/**********************************************************
 *DS18B20初始化
 *函数名称:DS18B20_Init()
 *说明：本初始化程序可以不要，因为18B20在出厂时就被配置为12位精度了
 **********************************************************/
void DS18B20_Init()
{

    DS18B20_Reset();
    DS18B20_WriteData(0xCC);  // 跳过ROM
    DS18B20_WriteData(0x4E);  // 写暂存器
    DS18B20_WriteData(0x64);  // 往暂存器的第三字节中写上限值100℃
    DS18B20_WriteData(0x00);  // 往暂存器的第四字节中写下限值0℃
    DS18B20_WriteData(0x7F);  // 将配置寄存器配置为12位精度
    DS18B20_Reset();
}
/**********************************************************
 *复位DS18B20(通过存在脉冲可以判断DS1820是否损坏)
 *函数名称:DS18B20_Reset()
 **********************************************************/
unsigned char DS18B20_Reset()
{
    unsigned char flag;
    DQ_OUT;                       //DQ为输出
    DS18B20_H;
    delay_us(8);                  //延时8微秒
    DS18B20_L;                    //拉低总线
    delay_us(500);                //延时500微秒,产生复位脉冲，必须大于480微秒
    DS18B20_H;
    delay_us(80);              //15~60us 后接收 60-240us的存在脉冲，必须大于60微秒
    DQ_IN;
    if(READ_DQ)flag=0;           //等待从机 DS18B20 应答（低电平有效）
    else flag=1;
    DQ_OUT;
    delay_us(440);
    DS18B20_H;
    return(flag);
}
/**********************************************************
 *写数据到DS18B20
 *函数名称:DS18B20_WriteData()
 **********************************************************/
void DS18B20_WriteData(unsigned char wData)
{
    unsigned char i;

    DQ_OUT;                     //DQ为输出
    for (i=8;i>0;i--)
    {
        DS18B20_L;                  //拉低总线,产生写信号
        delay_us(15);               //延时在15us~30us
        if(wData&0x01)              //发送1位
        {DS18B20_H;}
        else {DS18B20_L;}
        delay_us(45);               //延时15~60us
        DS18B20_H;                  //释放总线,等待总线恢复
        wData>>=1;                  //准备下一位数据的传送
    }
}
/**********************************************************
 *从DS18B20中读出数据
 *函数名称:DS18B20_ReadData()
 **********************************************************/
unsigned char DS18B20_ReadData()
{
    unsigned char i,TmepData;

    for(i=8;i>0;i--)
    {
        TmepData>>=1;             //数据右移
        DQ_OUT;                   //DQ为输出
        DS18B20_L;                //拉低总线,产生读信号
        delay_us(6);
        DS18B20_H;                //释放总线,准备读数据
        delay_us(4);
        DQ_IN;                    //DQ为输入
        if(READ_DQ)
        {TmepData|=0x80;}
        delay_us(65);
    }
    return(TmepData);          //返回读到的数据
}
/**********************************************************
 *DS18B20转换温度
 *函数名称:DS18B20_Conert()
 **********************************************************/
unsigned int DS18B20_Conert()
{
    unsigned char TempData1,TempData2;

    DS18B20_Reset();
    DS18B20_WriteData(0xCC);       // 跳过ROM
    DS18B20_WriteData(0x44);       // 开始转换
    delay_us(500);                 //延时一般在500us不能过小
    DS18B20_Reset();
    DS18B20_WriteData(0xCC);       // 跳过ROM
    DS18B20_WriteData(0xBE);       //读取 RAM

    TempData1=DS18B20_ReadData();  //读低八位，LS Byte, RAM0
    TempData2=DS18B20_ReadData();  //读高八位，MS Byte, RAM1
    //   delay_ms(750);                 //延时750~900ms，保证工作周期
    DS18B20_Reset();
    //return (((TempData2<<8)|TempData1)*0.625); //0.0625=xx, 0.625=xx.x, 6.25=xx.xx
    return (((TempData2<<8)+TempData1)); //0.0625=xx, 0.625=xx.x, 6.25=xx.xx
}



void ADC10_init(void) {
    ADC10CTL0 =SREF_1 + ADC10SHT_3 + REF2_5V + REFON + REFOUT + ADC10ON;
    //基准VR+=VREF+,VR-=VSS,64×ADC10CLK采样保持时间,2.5V基准电压,基准输出打开
    ADC10CTL1 = INCH_7 + CONSEQ_0;//单通道单次转换
    ADC10AE0 |= BIT7;     //选择P1.7/A7来做ADC
}

void ADC10_WaveSample(void)
{
    unsigned int j = 0,i = 0;
    min = 1023;
    max = 0;
    for(j = 0; j < 64; j++)
    {  //采样SIN_NUMA次
        ADC10CTL0 |= ENC + ADC10SC;  //开始采样和转换
        for(i = 0;i < 500; i++); //采样延时，至少要采样一个周期的数据
        table_Valu[j] = ADC10MEM;
        if(table_Valu[j] > max)
        {
            max = table_Valu[j];
        } //找出最大值
        if(table_Valu[j] < min)
        {
            min = table_Valu[j];
        } //找出最小值
    }
}
