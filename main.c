#include <msp430.h>
//#include <TM1638.h>
//TM1638ģ�����Ŷ���
#define DIO BIT3
#define CLK BIT4
#define STB BIT5
#define IO_OUT P1OUT
//#define DQ BIT0 //������Ϊ P1.0

#define DATA_COMMAND    0X40
#define DISP_COMMAND    0x80
#define ADDR_COMMAND    0XC0


//�����������ʾ����
unsigned char tab[]={
0x3F,0x06,0x5B,0x4F,
0x66,0x6D,0x7D,0x07,
0x7F,0x6F,0x40,0x7C,
0x39,0x5E,0x00,0x3e};

unsigned int k=0,MAX=0x00,MIN=0xFFFF,f=0,count=0,jud=0,left=1,right=2,add=3,mix=4,ws=1;
int loc=0;
unsigned int  efv,max,min,s,q,w,e,r,u,p;
unsigned char table_Valu[64];

unsigned int temp[2];
unsigned int T;
unsigned int get_one_temperature(void);
//#include <temp_1.h>

unsigned char num[8];                //�����������ʾ��ֵ
unsigned int show=270,dub=150;       //show���¶ȱ���ֵ2.7,   dub�ǵ�ѹ����ֵ1.5
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
unsigned long int sum;
unsigned int temp[3];
init_TM1638();                             //��ʼ��TM1638
for(i=0;i<8;i++)
    Write_DATA(i<<1,tab[0]);                  //��ʼ���Ĵ���
//sin();
P1DIR&=~BIT7;
P2DIR |= BIT5;
P2OUT&=~BIT5;  //�رշ�������ʼ��

while(1)
{
    s=Read_key();                          //������ֵS1
    while(Read_key()==s);            //������ȥ���ţ�����Ҳ���õ�
    if(s==0)
    {   count++;
    }
    jud=count%3;
    while(jud==1)//����һ��
    {
        sum=DS18B20_Conert();
        sum=sum*0.625;
        temp[0]=(sum)%10;
        temp[1]=((sum)/10)%10;
        temp[2]=((sum)/100)%10;
        //�¶���ʾ
        Write_DATA(4*2,tab[12]);   //"C"
        Write_DATA(7*2,tab[temp[0]]);   //0.1λ
        Write_DATA(6*2,tab[temp[1]]|0x80);   //1.λ
        Write_DATA(5*2,tab[temp[2]]);   //10λ

        efv = (max - min)*100/341;  //efv��ѹ��Чֵ�����������

        Write_DATA(0*2,tab[15]);   //"U"
        Write_DATA(1*2,tab[efv/100]|0x80);
        Write_DATA(2*2,tab[(efv/10)%10]);
        Write_DATA(3*2,tab[efv%10]);

        if(sum>show||efv>dub)
            P2OUT|=BIT5;
        else
            P2OUT&=~BIT5;
        Write_allLED(0); //�������ж�����LEDϨ��
        s=Read_key();
        if(s==0)
            break;
    }

    if(jud==2)   //���µڶ���S1
    {
//����ֵ���趨
        Write_DATA(0*2,tab[14]);  //Ϩ��
        Write_DATA(1*2,tab[12]);    //��C��
        Write_DATA(2*2,tab[show/100]);  // show�ĸ�λ��
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
                        P2OUT&=~BIT5;       //2.5����

            s=Read_key();                          //������ֵ
            while(Read_key()==s);
            if(s==0)
                {   count++;
                jud=count%3;
                break;
                }
            if(s==5)    //��λ��s6
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
            if(s==2)       //����
            {
                loc ++;     // loc������ܵ�λ��
                ws=2*ws;   //��ws��LED��

                Write_allLED(ws);  //
                //s = Read_key();
                //while(Read_key()==s);
            }

            if(s==1)        //����
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
            if(s==4)         //��   locֻ����  2  3    6  7 ��Ӧλ��
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

            if(s==3)         //��
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


void TM1638_Write(unsigned char DATA)           //д���ݺ���
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
unsigned char TM1638_Read(void)                 //�����ݺ���
{
    P1DIR &= ~DIO;
    unsigned char i;
    unsigned char temp=0;
    P1DIR &= ~DIO ;  //����Ϊ����
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
void Write_COM(unsigned char cmd)       //����������
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
    TM1638_Write(0x42);                //����ɨ���� ����
    for(i=0;i<4;i++)
        c[i]=TM1638_Read();
    IO_OUT|=STB;                             //4���ֽ����ݺϳ�һ���ֽ�
    for(i=0;i<4;i++)
        key_value|=c[i]<<i;
    for(i=0;i<8;i++)
        if((0x01<<i)==key_value)
            break;
    return i;
}
void Write_DATA(unsigned char add,unsigned char DATA)       //ָ����ַд������
{
    Write_COM(0x44);
    P1OUT &=~STB;
    TM1638_Write(0xc0|add);
    TM1638_Write(DATA);
    P1OUT |= STB;
}

void Write_allLED(unsigned char LED_flag)                   //����ȫ��LED������LED_flag��ʾ����LED״̬
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

//TM1638��ʼ������
void init_TM1638(void)
{
    unsigned char i;
    Write_COM(0x8a);       //���� (0x88-0x8f)8�����ȿɵ�
    Write_COM(0x40);       //���õ�ַ�Զ���1
    P1OUT &=~STB;                 //
    TM1638_Write(0xc0);    //������ʼ��ַ

    for(i=0;i<16;i++)      //����16���ֽڵ�����
        TM1638_Write(0x00);
    P1OUT |=STB;
}
void DS18B20_init();                              //DS18B20��ʼ��
unsigned char DS18B20_Reset();                    //DS18B20��λ
void DS18B20_WriteData(unsigned char);            //д���ݵ�DS18B20
unsigned char DS18B20_ReadData();                 //������
extern unsigned int DS18B20_Conert();                    //ת���¶�

#define DS18B20_DIR P1DIR
#define DS18B20_OUT P1OUT
#define DS18B20_IN P1IN
#define DS18B20_DQ BIT2

#define DS18B20_H   DS18B20_OUT|=DS18B20_DQ        //DQ��λ
#define DS18B20_L   DS18B20_OUT&=~DS18B20_DQ       //DQ��λ

#define DQ_IN       DS18B20_DIR &= ~DS18B20_DQ     //����DQΪ����
#define DQ_OUT      DS18B20_DIR |= DS18B20_DQ      //����DQΪ���
#define READ_DQ     (DS18B20_IN&DS18B20_DQ)        //��DQ��ƽ

#define CPU_F ((double)1000000)
#define delay_us(x) __delay_cycles((long)(CPU_F*(double)x/1000000.0))
#define delay_ms(x) __delay_cycles((long)(CPU_F*(double)x/1000.0))

/**********************************************************
 *DS18B20��ʼ��
 *��������:DS18B20_Init()
 *˵��������ʼ��������Բ�Ҫ����Ϊ18B20�ڳ���ʱ�ͱ�����Ϊ12λ������
 **********************************************************/
void DS18B20_Init()
{

    DS18B20_Reset();
    DS18B20_WriteData(0xCC);  // ����ROM
    DS18B20_WriteData(0x4E);  // д�ݴ���
    DS18B20_WriteData(0x64);  // ���ݴ����ĵ����ֽ���д����ֵ100��
    DS18B20_WriteData(0x00);  // ���ݴ����ĵ����ֽ���д����ֵ0��
    DS18B20_WriteData(0x7F);  // �����üĴ�������Ϊ12λ����
    DS18B20_Reset();
}
/**********************************************************
 *��λDS18B20(ͨ��������������ж�DS1820�Ƿ���)
 *��������:DS18B20_Reset()
 **********************************************************/
unsigned char DS18B20_Reset()
{
    unsigned char flag;
    DQ_OUT;                       //DQΪ���
    DS18B20_H;
    delay_us(8);                  //��ʱ8΢��
    DS18B20_L;                    //��������
    delay_us(500);                //��ʱ500΢��,������λ���壬�������480΢��
    DS18B20_H;
    delay_us(80);              //15~60us ����� 60-240us�Ĵ������壬�������60΢��
    DQ_IN;
    if(READ_DQ)flag=0;           //�ȴ��ӻ� DS18B20 Ӧ�𣨵͵�ƽ��Ч��
    else flag=1;
    DQ_OUT;
    delay_us(440);
    DS18B20_H;
    return(flag);
}
/**********************************************************
 *д���ݵ�DS18B20
 *��������:DS18B20_WriteData()
 **********************************************************/
void DS18B20_WriteData(unsigned char wData)
{
    unsigned char i;

    DQ_OUT;                     //DQΪ���
    for (i=8;i>0;i--)
    {
        DS18B20_L;                  //��������,����д�ź�
        delay_us(15);               //��ʱ��15us~30us
        if(wData&0x01)              //����1λ
        {DS18B20_H;}
        else {DS18B20_L;}
        delay_us(45);               //��ʱ15~60us
        DS18B20_H;                  //�ͷ�����,�ȴ����߻ָ�
        wData>>=1;                  //׼����һλ���ݵĴ���
    }
}
/**********************************************************
 *��DS18B20�ж�������
 *��������:DS18B20_ReadData()
 **********************************************************/
unsigned char DS18B20_ReadData()
{
    unsigned char i,TmepData;

    for(i=8;i>0;i--)
    {
        TmepData>>=1;             //��������
        DQ_OUT;                   //DQΪ���
        DS18B20_L;                //��������,�������ź�
        delay_us(6);
        DS18B20_H;                //�ͷ�����,׼��������
        delay_us(4);
        DQ_IN;                    //DQΪ����
        if(READ_DQ)
        {TmepData|=0x80;}
        delay_us(65);
    }
    return(TmepData);          //���ض���������
}
/**********************************************************
 *DS18B20ת���¶�
 *��������:DS18B20_Conert()
 **********************************************************/
unsigned int DS18B20_Conert()
{
    unsigned char TempData1,TempData2;

    DS18B20_Reset();
    DS18B20_WriteData(0xCC);       // ����ROM
    DS18B20_WriteData(0x44);       // ��ʼת��
    delay_us(500);                 //��ʱһ����500us���ܹ�С
    DS18B20_Reset();
    DS18B20_WriteData(0xCC);       // ����ROM
    DS18B20_WriteData(0xBE);       //��ȡ RAM

    TempData1=DS18B20_ReadData();  //���Ͱ�λ��LS Byte, RAM0
    TempData2=DS18B20_ReadData();  //���߰�λ��MS Byte, RAM1
    //   delay_ms(750);                 //��ʱ750~900ms����֤��������
    DS18B20_Reset();
    //return (((TempData2<<8)|TempData1)*0.625); //0.0625=xx, 0.625=xx.x, 6.25=xx.xx
    return (((TempData2<<8)+TempData1)); //0.0625=xx, 0.625=xx.x, 6.25=xx.xx
}



void ADC10_init(void) {
    ADC10CTL0 =SREF_1 + ADC10SHT_3 + REF2_5V + REFON + REFOUT + ADC10ON;
    //��׼VR+=VREF+,VR-=VSS,64��ADC10CLK��������ʱ��,2.5V��׼��ѹ,��׼�����
    ADC10CTL1 = INCH_6 + CONSEQ_0;//��ͨ������ת��
    ADC10AE0 |= BIT6;     //ѡ��P1.0/A0����ADC
}

void ADC10_WaveSample(void)
{
    unsigned int j = 0,i = 0;
    min = 1023;
    max = 0;
    for(j = 0; j < 64; j++)
    {  //����SIN_NUMA��
        ADC10CTL0 |= ENC + ADC10SC;  //��ʼ������ת��
        for(i = 0;i < 300; i++); //������ʱ������Ҫ����һ�����ڵ�����
        table_Valu[j] = ADC10MEM;
        if(table_Valu[j] > max)
        {
            max = table_Valu[j];
        } //�ҳ����ֵ
        if(table_Valu[j] < min)
        {
            min = table_Valu[j];
        } //�ҳ���Сֵ
    }
}
