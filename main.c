/*****************************************************
Project : Skripsi (Water Level Control System)
Version : 1.0
Date    : 02/09/2016
Author  : Sandy Pratama Destiana Putra
Company : Teknik Informatika Unsika
Comments: 
Water Level Control System with Notification
using SMS

Chip type               : ATmega8535
Program type            : Application
AVR Core Clock frequency: 8.000000 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 128
*****************************************************/

#include <mega8535.h>
#include <delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#asm
        .equ __lcd_port=0x15 ;PORTC
#endasm
#include <alcd.h>

#define enter putchar(13)
#define TRIG PORTA.0
#define ECHO PINA.1
#define motor_on PORTA.2=1
#define motor_off PORTA.2=0
#define ALARM_ON PORTA.4

// Deklarasi variabel global
unsigned int i, count, batas, level, tampung=0, waktu=0;
unsigned long hitung;
unsigned char detik, menit, jam, take;
unsigned char tampil[16], tampil2[16], temp[16], ambil[30],tampil_no[16];
unsigned char alarm=0, status=1;

unsigned int batas_atas;
unsigned int batas_bawah;

eeprom int tinggi;
eeprom int batas_waktu;
eeprom char nomor_hp[16];

void inisialisasi()
{
    //fungsi untuk inisialisasi
    // inisialisasi PORT
    PORTA=0x20;DDRA=0x15;
    PORTB=0x00;DDRB=0x00;
    PORTC=0x00;DDRC=0x00;
    PORTD=0x00;DDRD=0x00;
    
    // inisialisasi untuk komunikasi melalui UASART
    UCSRA=0x00;UCSRB=0x18;UCSRC=0x86;
    UBRRH=0x00;UBRRL=0x33;
    ACSR=0x80;SFIOR=0x00;
}

void mulai()
{
    unsigned char loading;
    lcd_init(16);
    lcd_clear();
    lcd_gotoxy(0,0);
    lcd_putsf("Water Level");
    lcd_gotoxy(0,1);
    lcd_putsf("Control System");                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
    delay_ms(1000);
    
    lcd_clear();
    for (i=0; i<16; i++){
        lcd_gotoxy(0,0);
        loading = (i+1) * 6.25;
        sprintf(temp, "Loading %d", loading);
        lcd_puts(temp);
        lcd_gotoxy(11,0);
        lcd_putsf("%");
        lcd_gotoxy(i,1);
        lcd_putchar('>');
        delay_ms(100);
    }
    delay_ms(500);
}

void kirim_sms(){
    lcd_clear();
    lcd_gotoxy(0,0);
    lcd_putsf("Error Detected");
    delay_ms(1000);  
    printf("AT+CMGS=");
    putchar('"');
    puts(tampil_no);
    putchar('"');
    putchar(13);
    delay_ms(1000);
    printf("Masalah Terdeteksi. Mohon periksa keadaan sistem");
    delay_ms(1000);
    putchar(26);
    delay_ms(1000);
    lcd_clear();
    lcd_gotoxy(0,0);
    lcd_putsf("Message Sent");
    delay_ms(1000);
}

void hapus_data_tampung()
{
    for(i = 0; i < 16; i++){
        temp[i] = '';
        tampil2[i] = '';
    }
}

void baca_nomor()
{
    for(i = 0; i < 13; i++){
        tampil_no[i] = nomor_hp[i];
    }
}

void atur_tinggi()
{
    i=1;
    while (ambil[i] !='x'){
        temp[i-1] = ambil[i];
        i++;
    }
    tinggi = atoi(temp);
    lcd_clear();
    lcd_gotoxy(0,0);
    lcd_putsf("Height Set");
    lcd_gotoxy(0,1);
    sprintf(tampil2, "%d", tinggi);
    lcd_puts(tampil2);
    delay_ms(1000);
    hapus_data_tampung();
}

void atur_waktu()
{
    i=1;
    while (ambil[i] !='x'){
        temp[i-1] = ambil[i];
        i++;
    }
    batas_waktu = atoi(temp);
    lcd_clear();
    lcd_gotoxy(0,0);
    lcd_putsf("Time Limit Set");
    lcd_gotoxy(0,1);
    sprintf(tampil2, "%d", batas_waktu);
    lcd_puts(tampil2);
    delay_ms(1000);
    hapus_data_tampung();
}

void daftar_nomor()
{
    i=1;
    while (ambil[i] !='x'){
        nomor_hp[i-1] = ambil[i];
        tampil_no[i-1] = nomor_hp[i-1];
        i++;
    }
    lcd_clear();
    lcd_gotoxy(0,0);
    lcd_putsf("Reg Number");
    lcd_gotoxy(0,1);
    lcd_puts(tampil_no);
    delay_ms(1000);
    hapus_data_tampung();
}

void baca_data(){
    lcd_clear();
    lcd_gotoxy(0,0);
    lcd_putsf("Batas Waktu");
    lcd_gotoxy(0,1);
    sprintf(tampil2, "%d", batas_waktu);
    lcd_puts(tampil2);
    delay_ms(1500);
    lcd_clear();
    lcd_gotoxy(0,0);
    lcd_putsf("Tinggi");
    lcd_gotoxy(0,1);
    sprintf(tampil2, "%d", tinggi);
    lcd_puts(tampil2);
    delay_ms(1500);
    lcd_clear();
    lcd_gotoxy(0,0);
    lcd_putsf("Nomor Terdaftar");
    lcd_gotoxy(0,1);
    lcd_puts(tampil_no);
    delay_ms(1500);
    lcd_clear();
    lcd_gotoxy(0,0);
    lcd_putsf("Konfigurasi");
    delay_ms(2000);
    i = 0;
    take = getchar();
    while(take!='y'){
        ambil[i] = take;
        take = getchar();
        i++;
    }
    if (ambil[0] == 'a'){
        atur_tinggi();
    }else if (ambil[0] == 'b'){
        atur_waktu();
    }else if (ambil[0] == 'c'){
        daftar_nomor();
    }
}

void update_waktu()
{   //ubah rata-rata
    batas_waktu = (batas_waktu + waktu)/2;
    lcd_clear();
    lcd_gotoxy(0,0);
    lcd_putsf("Mengisi Selesai");
    delay_ms(1000);
}

int jarak()
{  
    TRIG = 1;
    delay_us(10);
    TRIG = 0;
    while(ECHO == 0){};
    while(ECHO == 1)
    {
        count++;
    };
    hitung = (float)count /4.49032258;
    level = tinggi - hitung;
    return level;
}

void konversi_waktu()
{
    jam = waktu / 3600;
    menit = (waktu - (3600*jam)) / 60;
    detik = (waktu - (3600*jam)) - (menit * 60);
    sprintf(tampil2,"mengisi=%d:%d:%d", jam,menit,detik);
}

void main(void)
{
    inisialisasi();
    mulai();
    motor_off;
    while (tinggi <=0 || batas_waktu <=0){
        baca_data();
    }
    baca_nomor();
    batas_atas = 0.95 * tinggi /10;
    batas_bawah = tinggi/200;
    while (alarm == 0)
    {   
        count = 0; hitung=0;
        tampung = jarak() /10;

        sprintf(tampil,"Level %d cm", tampung);
        sprintf(tampil2,"%d,%d", batas_bawah, batas_atas);  
        lcd_clear();
        if (tampung <= batas_bawah){
            status = 0;         //status = 0, ketika ketinggian dibawah LOW
        }  
        if (status == 0){               //keadaan low, pengisian dilakukan
            motor_on;
            batas = 1.5 * batas_waktu;   //toleransi 150%
            konversi_waktu();
            waktu++;
            if (waktu >= batas){
                alarm = 1;              // melebihi batas waktu, sistem berhenti  
                kirim_sms(); 
                motor_off;
            }
            
            lcd_clear();
            lcd_gotoxy(0,0);
            lcd_puts(tampil);
            lcd_gotoxy(0,1);
            lcd_puts(tampil2);
            delay_ms(1000);
            
            if (tampung >= batas_atas){        //lebih dari batas atas, pengisian berhenti
                update_waktu();
                status = 1;
            }
            
        
        }
        else if (status == 1){        //ketinggian tidak dibawah low
            motor_off;                
            waktu = 0;
            lcd_clear();
            lcd_gotoxy(0,0);
            lcd_puts(tampil);
            delay_ms(1000);
            if (PINA.5 == 0){
                baca_data();
            }
        } 
    }
    lcd_clear();
    lcd_gotoxy(0,0);
    lcd_putsf("Tekan Reset");
    lcd_gotoxy(0,1);
    lcd_putsf("untuk memulai");
    delay_ms(1000);
}
