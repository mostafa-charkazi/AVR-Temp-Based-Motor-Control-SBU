/*******************************************************
This program was created by the
CodeWizardAVR V3.14 Advanced
Automatic Program Generator
© Copyright 1998-2014 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : Micro AVR Course
Version : 1.0.0
Date    : 6/3/2025
Author  : Mostafa Charkazi
Company : SBU
Comments: Control Motor using temprature sensor (LM32)


Chip type               : ATmega32
Program type            : Application
AVR Core Clock frequency: 2.000000 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 512
*******************************************************/
#include <mega32.h>
#include <delay.h>
#include <alcd.h>
#include <stdio.h>
#include <string.h>
#define ADC_VREF_TYPE ((1<<REFS1) | (1<<REFS0) | (0<<ADLAR)) // Voltage Reference: Int., cap. on AREF
#define led PORTD.6
#define DA PIND.2
#define btn PINB.0

int i = 0;
int j = 0;
int k = 0; // LOG cursor

char str[20];
int menu = 0;
float temperature = 0;
int temp_int;
int temp_frac;
unsigned int hour = 2, minute = 12, second = 35;
unsigned int overflow_count = 0;
unsigned char key = 0;
char last_key = 0;
unsigned int key_hold_counter = 0; // Increasing it in timer (Not interupt)
unsigned int step = 1;

char input[17] = "";
char username[17] = "";
char password[17] = "";
char tries = 0; // Number Of Tries
char stage = 0; // USERNAME input OR PASSWORD input
char login = 0;

char correct_user[] = "12345";
char correct_pass[] = "11111";
char menu_shown = 0;

int threshold_temperature = 27;
int thre_temp;

unsigned int mot_speed = 25;
int temp_mot_speed = 25;
int warm_start = -1; // zaman shoru garm boodan hava
unsigned int mot_speed_current = 0;
int elapsed; // bara mohasebe zaman 15s

unsigned int temp_sec;
unsigned int temp_min;
unsigned int temp_hour;

unsigned int digit[4]; // 7-Segment
char current_digit = 0;
char segment_on = 0;

int submenu = 0;
unsigned int hour_not = 0, minute_not = 0, second_not = 0;
unsigned int hour_login = 0, minute_login = 0, second_login = 0;

unsigned int hour_thre = 2, minute_thre = 12, second_thre = 35;
int old_thre = 0;
int new_thre = 0;

unsigned int hour_mot = 2, minute_mot = 12, second_mot = 35;
int old_mot = 0;
int new_mot = 0;

unsigned int hour_clock = 2, minute_clock = 12, second_clock = 35;
int new_clock_hour = 0, new_clock_clock = 0, new_clock_second = 0;
// Functions
void init();
char key_to_char(unsigned char);
void update_digits();
void check_pb();

// External Interrupt 0 service routine
interrupt [EXT_INT0] void ext_int0_isr(void) {
    delay_ms(200); // Denounce   

    if (DA == 0) {
        key = PIND & 0x1B;
        last_key = key_to_char(key);
        key_hold_counter = 0;
    }
    else {
        last_key = 0;
    }
}

// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void) {
    overflow_count++;              
    // 2 MHz : overflow_count >= 8
    // 8 MHz : overflow_count >= 31
    if (overflow_count >= 8) {
        overflow_count = 0;
        second++;
        if (second >= 60){
            second = 0;
            minute++;
            if (minute >= 60) {
                minute = 0;
                hour++;
                if (hour >= 12)
                    hour = 0;
            }
        }
    }
    
    if (DA) {
        key_hold_counter++;
    }
}

// Timer1 overflow interrupt service routine
interrupt [TIM1_OVF] void timer1_ovf_isr(void) {
    if (segment_on){
        PORTC |= 0xF0;

        PORTC = (PORTC & 0xF0) | (digit[current_digit] & 0x0F);

        PORTC &= ~(1 << (4 + current_digit));

        // Decimal Point
        if (current_digit == 1)
            PORTD |= (1 << 5);
        else
            PORTD &= ~(1 << 5);

        current_digit = (current_digit + 1) % 4;
    }
    else {
        PORTC = 0xF0;
    }
}

// Read the AD conversion result
unsigned int read_adc(unsigned char adc_input) {
    ADMUX=adc_input | ADC_VREF_TYPE;
    // Delay needed for the stabilization of the ADC input voltage
    delay_us(10);
    // Start the AD conversion
    ADCSRA|=(1<<ADSC);
    // Wait for the AD conversion to complete
    while ((ADCSRA & (1<<ADIF))==0);
    ADCSRA|=(1<<ADIF);
    return ADCW;
}

void main(void) {
    init();

    lcd_clear();
    lcd_gotoxy(0, 0); lcd_puts("Group No. 3");
    lcd_gotoxy(0, 1); lcd_puts("Project No. 1");
    delay_ms(1000);
           
    lcd_clear();
    lcd_gotoxy(0, 0); lcd_puts("401249015");
    lcd_gotoxy(0, 1); lcd_puts("Mostafa");
    lcd_gotoxy(0, 2); lcd_puts("Charkazi");
    delay_ms(1000);

    lcd_clear();
    lcd_gotoxy(0, 0); lcd_puts("401249036");
    lcd_gotoxy(0, 1); lcd_puts("Meysam");
    lcd_gotoxy(0, 2); lcd_puts("Sadeghi");
    delay_ms(1000);

    lcd_clear();
    lcd_gotoxy(0, 0); lcd_puts("401249008");
    lcd_gotoxy(0, 1); lcd_puts("Ali Asghar");
    lcd_gotoxy(0, 2); lcd_puts("Bararjanpour");
    delay_ms(1000);

    while (1) {
        int t;
        temperature = (float) read_adc(0) * 0.25;
        temp_int = (int) temperature;
        temp_frac = (int)((temperature - temp_int) * 100);
        update_digits();
        check_pb();        
        
        // Key Hold Counter
        if (DA == 1) {
            step = key_hold_counter;
            
            if (menu == 4){
                if((PIND & 0x1B) == 27){  // key : +
                    t = temp_mot_speed + step;
                    t = (t >= 100)? 100: t;
                }
                else if ((PIND & 0x1B) == 26){ // key : -  
                    t = temp_mot_speed - step;
                    t = (t <= 0)? 0: t;
                }
                sprintf(str, "Speed: %d %%        ", t);
                lcd_gotoxy(0, 1); lcd_puts(str);
            }
            else if (menu == 3){
                if((PIND & 0x1B) == 27){  // key : +
                    t = thre_temp + step;
                    t = (t >= 100)? 100: t;
                }
                else if ((PIND & 0x1B) == 26){ // key : -  
                    t = thre_temp - step;
                    t = (t <= 1)? 1: t;
                }
            }    
        }
        else {
            key_hold_counter = 0;
        }
      
        // PWM and LED    
		if (temperature > threshold_temperature) {
			if (warm_start == -1) {
				warm_start = second;
				mot_speed_current = mot_speed;
			}
			
			elapsed = second - warm_start;
			if (elapsed < 0) elapsed += 60;


			i = elapsed / 15;
			mot_speed_current = mot_speed * (i + 1);
			if (mot_speed_current > 100) mot_speed_current = 100;

			led = 1;
			OCR2 = (unsigned int)((mot_speed_current / 100.0) * 255);
		}
		else {
			warm_start = -1; mot_speed_current = 0;
			led = 0;  OCR2 = 0x00;
		}
        
        
        // Menu 0 : Time and degree
        // Menu 1 : Login Menu
        // Menu 2 : Main menu
        // Menu 3 : Threshold
        // Menu 4 : Mot Speed
        // Menu 5 : Set Clock
        // Menu 6 : Logout
        // Menu 7 : Log ( when x pressed )
        if (menu == 0) {
            lcd_clear();  
            sprintf(str, "%d.%02d", temp_int, temp_frac);
            lcd_gotoxy(0, 0);
            lcd_putsf("Temp = ");
            lcd_gotoxy(7, 0);
            lcd_puts(str);
            lcd_putchar(0xDF); // Degree Char
            sprintf(str, "Time = %02d:%02d:%02d", hour, minute, second);
            lcd_gotoxy(0, 1); lcd_puts(str); 
            
            // Menu       
            if (last_key == 'C') {
                menu = 1; menu_shown = 0; last_key = 0;
            }  
            
        }
        else if (menu == 1){   
           // Login Menu
            if (login == 0){            
                if (!menu_shown) {
                    lcd_clear();
                    lcd_gotoxy(0, 0); lcd_puts("Please Login      "); 
                    lcd_gotoxy(0, 1); lcd_puts("User :            ");
                    lcd_gotoxy(0, 2); lcd_puts("Pass :            ");
                    lcd_gotoxy(0, 3); lcd_puts("NoT : 0           ");
                    menu_shown = 1;
                }
                
                if (last_key == 'C'){
                     menu = 0; menu_shown = 0; last_key = 0;
                }
                else if (last_key != 0){
                    char ch = last_key;
                    last_key = 0;
                    
                    if (ch == '='){
                        if (stage == 0){
                            strcpy(username, input);
                            input[0] = 0;
                            stage = 1;
                        }else if (stage == 1){
                            strcpy(password, input);
                            input[0] = 0;
                            stage = 0;
                            if (strcmp(username, correct_user) == 0 && strcmp(password, correct_pass) == 0) {
                                hour_login = hour;
                                minute_login = minute;
                                second_login = second;
                                
                                login = 1;
                                menu = 2;
                                menu_shown = 0;
                            } else {
                                tries++;
                                lcd_gotoxy(0,1); lcd_puts("User :           ");
                                lcd_gotoxy(0,2); lcd_puts("Pass :           ");
                                sprintf(str, "NoT : %d        ", tries);
                                lcd_gotoxy(0, 3); lcd_puts(str);
                                
                                // Changing in history
                                hour_not = hour;
                                minute_not = minute;
                                second_not = second;
                            }
                        }
                    }
                    else if (last_key == 'C'){
                        menu = 6;
                    }
                    else {
                        int len = strlen(input);
                        lcd_gotoxy(0,0); lcd_puts("Please Login   ");
                        if (len < 16) {
                            input[len] = ch;
                            input[len + 1] = 0;
                            
                            if (stage == 0) {
                                sprintf(str, "NoT : %d        ", tries);
                                lcd_gotoxy(0,1); lcd_puts("User :           ");
                                lcd_gotoxy(0,2); lcd_puts("Pass :           ");
                                lcd_gotoxy(0,3); lcd_puts(str);
                                lcd_gotoxy(7, 1); lcd_puts(input);
                            } 
                            else {
                                lcd_gotoxy(7, 2);
                                for (i = 0; i < len + 1; i++)
                                    lcd_putchar('*');
                            }
                        }
                    } 
                } 
            } 
            else {
                menu = 2; menu_shown = 0; last_key = 0;
            }
        }
        else if (menu == 2){
            // MENU (AS PROJECT SAID)
            if (!menu_shown) {
                lcd_clear();
                lcd_gotoxy(0, 0); lcd_puts("Set Threshold       ");
                lcd_gotoxy(0, 1); lcd_puts("Set MOT . SPD       ");
                lcd_gotoxy(0, 2); lcd_puts("Set Clock           ");
                lcd_gotoxy(0, 3); lcd_puts("Logout              ");
                menu_shown = 1;
                i = 0;
            }
            
            if (last_key != 0){
                lcd_gotoxy(0, 0); lcd_puts("Set Threshold       ");
                lcd_gotoxy(0, 1); lcd_puts("Set MOT . SPD       ");
                lcd_gotoxy(0, 2); lcd_puts("Set Clock           ");
                lcd_gotoxy(0, 3); lcd_puts("Logout              ");
                
                if (last_key == '+'){
                    i--;
                }
                else if (last_key == '-'){
                    i++;
                }
                else if (last_key == 'x' || last_key == '/'){
                    menu = 7; menu_shown = 0; last_key = 0;
                }
                else if (last_key == '='){
                    switch(i){
                        case 0: menu = 3; menu_shown = 0; break;
                        case 1: menu = 4; menu_shown = 0; break;
                        case 2: menu = 5; menu_shown = 0; break;
                        case 3: menu = 6; menu_shown = 0; break;
                    }
                }
                else if (last_key == 'C'){
                    menu = 0; menu_shown = 0; last_key = 0;
                }
                
                if (i == 4){
                    i = 0;
                }
                else if (i == -1){
                    i = 3;
                }
                
                last_key = 0;
            }
            else {
                lcd_gotoxy(17,i); lcd_puts("<=");
            }
        }
        else if (menu == 3){
            // Threshold
            if (!menu_shown) {
                thre_temp = threshold_temperature;                
                lcd_clear();
                lcd_gotoxy(0, 0); lcd_puts("Threshold");
                sprintf(str, "Temperature: %d", thre_temp);
                lcd_gotoxy(0, 1); lcd_puts(str);lcd_putchar(0xDF);lcd_puts("C    ");
                menu_shown = 1;
            }
            if (last_key != 0){
                if (last_key == 'C'){
                    menu = 2; menu_shown = 0; last_key = 0;
                }
                else if (last_key == '+'){ 
                    thre_temp += step;
                }
                else if (last_key == '-'){
                    thre_temp -= step;
                }
                else if (last_key == '='){                
                     // Changing in history
                     hour_thre = hour;
                     minute_thre = minute;
                     second_thre = second;
                     old_thre = threshold_temperature;
                     new_thre = thre_temp;
                     
                     // Submiting data
                     threshold_temperature = thre_temp;          
                }
                
                last_key = 0;
            }
            else {
                lcd_gotoxy(0, 0); lcd_puts("Threshold");
                
                sprintf(str, "Temperature: %d", thre_temp);
                lcd_gotoxy(0, 1); lcd_puts(str);lcd_putchar(0xDF);lcd_puts("C    ");               
            }   
        }
        else if (menu == 4){
            // Mot Speed  
            if (!menu_shown) {
                temp_mot_speed = mot_speed;
                lcd_clear();
                lcd_gotoxy(0, 0); lcd_puts("Set MOT . SPD       ");
                sprintf(str, "Speed: %d %%        ", temp_mot_speed);
                lcd_gotoxy(0, 1); lcd_puts(str);
                menu_shown = 1;
            }

            if (last_key != 0){
                if (last_key == 'C'){
                    menu = 2; menu_shown = 0; last_key = 0;
                }
                else if (last_key == '='){
                    // changing history
                    hour_mot = hour;
                    minute_mot = minute;
                    second_mot = second;
                    old_mot = mot_speed;
                    new_mot = temp_mot_speed;
                    
                    // Submiting data
                    mot_speed = temp_mot_speed; last_key = 0;                    
                }
                else if (last_key == '+' || last_key == '-') {
                    if (last_key == '+'){
                        temp_mot_speed += step;
                        if (temp_mot_speed > 100)
                            temp_mot_speed = 100;
                    }
                    else if (last_key == '-'){ 
                        temp_mot_speed -= step;
                        if (temp_mot_speed < 0)
                            temp_mot_speed = 0;
                    }
                     
                    sprintf(str, "Speed: %d %%        ", temp_mot_speed);
                    lcd_gotoxy(0, 1); lcd_puts(str);
                    last_key = 0;
                }
            }
        }
        else if (menu == 5){     
            // Set Clock
            if (!menu_shown) {
                temp_sec = second;
                temp_min = minute;
                temp_hour = hour;
                i = 0; j = 1;
                lcd_clear();
                lcd_gotoxy(0, 0); lcd_puts("Set Clock           ");
                sprintf(str, "%02d:%02d:%02d       ", temp_hour, temp_min, temp_sec);
                lcd_gotoxy(0, 1); lcd_puts(str);
                lcd_gotoxy(i, 2); lcd_puts("--");
                menu_shown = 1;
            } 

            if (last_key != 0){
                if (last_key == 'C'){
                    menu = 2; menu_shown = 0; 
                }
                else if (last_key == '+'){
                    if (i == 0){
                        temp_hour = (temp_hour + 1) % 12;
                    }
                    else if (i == 3){
                        temp_min = (temp_min + 1) % 60;
                    }
                    else if (i == 6){
                        temp_sec = (temp_sec + 1) % 60;
                    }
                }
                else if (last_key == '-'){
                    if (i == 0){
                        temp_hour = (temp_hour == 0) ? 11 : temp_hour - 1;
                    }
                    else if (i == 3){
                        temp_min = (temp_min == 0) ? 59 : temp_min - 1;
                    }
                    else if (i == 6){
                        temp_sec = (temp_sec == 0) ? 59 : temp_sec - 1;
                    }
                }
                else if (last_key == 'x'){
                    i += 3; j += 3;
                    if (i > 6){
                        i = 0; j = 1;
                    }
                    lcd_gotoxy(0, 2); lcd_puts("                    ");
                }
                else if (last_key == '/'){
                    i -= 3; j -= 3;
                    if (i < 0){
                        i = 6; j = 7;
                    }
                    lcd_gotoxy(0, 2); lcd_puts("                    ");
                }
                else if (last_key == '='){     
                    // Changing history
                    hour_clock = hour;
                    minute_clock = minute;
                    second_clock = second; 
                    
                    new_clock_hour = temp_hour;
                    new_clock_clock = temp_min;
                    new_clock_second = temp_sec;
                    
                    second = temp_sec;
                    minute = temp_min;
                    hour = temp_hour;
                    
                }


                lcd_gotoxy(0, 0); lcd_puts("Set Clock           ");
                sprintf(str, "%02d:%02d:%02d       ", temp_hour, temp_min, temp_sec);
                lcd_gotoxy(0, 1); lcd_puts(str);
                lcd_gotoxy(i, 2); lcd_puts("--");

                last_key = 0;
            }
        }
        else if (menu == 6){
            // Logout
            strcpy(input, "");
            strcpy(username, "");
            strcpy(password, "");
            tries = 0; // Number Of Tries
            stage = 0; // USERNAME input OR PASSWORD input
            login = 0;
            menu = 0;
            menu_shown = 0;         
            last_key = 0;
        }
        else if (menu == 7) {
            lcd_clear();
                        
            // submenu 0: main manu 
            // submenu 1: Last Login fail
            // submenu 2: Last Threshold Change
            // submenu 3: Last motor speed change
            // submenu 4: Last Clock Change
            if (submenu == 0){
                if (last_key == '+'){
                    k--;
                    if (k == -1)
                        k = 3;
                }
                else if (last_key == '-'){
                    k++;
                    if (k == 4)
                        k = 0;
                }
                else if (last_key == 'x' || last_key == '/'){
                    menu = 1; menu_shown = 0; last_key = 0; submenu = 0; k = 0; j = 0; i = 0;
                }
                
                lcd_gotoxy(0, 0); lcd_puts("L. Login Fail       ");
                lcd_gotoxy(0, 1); lcd_puts("L. Threshold Chng   ");
                lcd_gotoxy(0, 2); lcd_puts("L. Mot Speed Chng   ");
                lcd_gotoxy(0, 3); lcd_puts("L. Clock Chng       ");
                lcd_gotoxy(18, k); lcd_puts("<=");
                
                if (last_key == '=') {
                    switch(k){
                        case 0: submenu = 1; break;
                        case 1: submenu = 2; break;
                        case 2: submenu = 3; break;
                        case 3: submenu = 4; break;
                    }
                    lcd_clear();
                    k = 0;
                    last_key = 0;
                }
                
                if (last_key == 'x'){
                    menu = 1; menu_shown = 0; last_key = 0; submenu = 0; k = 0;
                }
            }
            else if (submenu == 1) { 
                lcd_gotoxy(0, 0); lcd_puts("L. Login           ");
                
                sprintf(str, "Last Login = %02d:%02d:%02d", hour_login, minute_login, second_login);
                lcd_gotoxy(0, 1); lcd_puts(str);
                
                sprintf(str, "Last Fail = %02d:%02d:%02d", hour_not, minute_not, second_not);
                lcd_gotoxy(0, 2); lcd_puts(str);
                
                sprintf(str, "Tries Number = %d", tries);
                lcd_gotoxy(0, 3); lcd_puts(str); 
                
                if (last_key == 'C') {
                    submenu = 0;
                    k = 0;
                    last_key = 0;
                }
            }
            else if (submenu == 2) { 
                lcd_gotoxy(0, 0); lcd_puts("L. Threshold Chng   ");
                sprintf(str, "Time = %02d:%02d:%02d", hour_thre, minute_thre, second_thre);
                lcd_gotoxy(0, 1); lcd_puts(str);
                sprintf(str, "Old Threshold = %d", old_thre);
                lcd_gotoxy(0, 2); lcd_puts(str);
                sprintf(str, "New Threshold = %d", new_thre);
                lcd_gotoxy(0, 3); lcd_puts(str);
                 
                
                if (last_key == 'C') {
                    submenu = 0;
                    k = 0;
                    last_key = 0;
                }
            }
            else if (submenu == 3) { 
                lcd_gotoxy(0, 0); lcd_puts("L. Mot Speed Chng   ");         
                sprintf(str, "Time = %02d:%02d:%02d", hour_mot, minute_mot, second_mot);
                lcd_gotoxy(0, 1); lcd_puts(str);  
                sprintf(str, "Old Mot. Speed = %d", old_mot);
                lcd_gotoxy(0, 2); lcd_puts(str);
                sprintf(str, "New Mot. Speed = %d", new_mot);
                lcd_gotoxy(0, 3); lcd_puts(str);
                
                if (last_key == 'C') {
                    submenu = 0;
                    k = 0;
                    last_key = 0;
                }
            }
            else if (submenu == 4) { 
                lcd_gotoxy(0, 0); lcd_puts("L. Clock Chng       ");         
                sprintf(str, "Old Time = %02d:%02d:%02d", hour_clock, minute_clock, second_clock);
                lcd_gotoxy(0, 1); lcd_puts(str);         
                sprintf(str, "New Time = %02d:%02d:%02d", new_clock_hour, new_clock_clock, new_clock_second);
                lcd_gotoxy(0, 2); lcd_puts(str);
                
                if (last_key == 'C') {
                    submenu = 0;
                    k = 0;
                    last_key = 0;
                }            
            }
            
            last_key = 0;
        }
    }
}

void init(){
    // DDR and PORTS
    DDRA=(0<<DDA7) | (0<<DDA6) | (0<<DDA5) | (0<<DDA4) | (0<<DDA3) | (0<<DDA2) | (0<<DDA1) | (0<<DDA0);
    PORTA=(0<<PORTA7) | (0<<PORTA6) | (0<<PORTA5) | (0<<PORTA4) | (0<<PORTA3) | (0<<PORTA2) | (0<<PORTA1) | (0<<PORTA0);

    DDRB=(0<<DDB7) | (0<<DDB6) | (0<<DDB5) | (0<<DDB4) | (0<<DDB3) | (0<<DDB2) | (0<<DDB1) | (0<<DDB0);
    PORTB=(0<<PORTB7) | (0<<PORTB6) | (0<<PORTB5) | (0<<PORTB4) | (0<<PORTB3) | (0<<PORTB2) | (0<<PORTB1) | (0<<PORTB0);

    DDRC=(1<<DDC7) | (1<<DDC6) | (1<<DDC5) | (1<<DDC4) | (1<<DDC3) | (1<<DDC2) | (1<<DDC1) | (1<<DDC0);
    PORTC=(1<<PORTC7) | (1<<PORTC6) | (1<<PORTC5) | (1<<PORTC4) | (1<<PORTC3) | (1<<PORTC2) | (1<<PORTC1) | (1<<PORTC0);

    DDRD=(1<<DDD7) | (1<<DDD6) | (1<<DDD5) | (0<<DDD4) | (0<<DDD3) | (0<<DDD2) | (0<<DDD1) | (0<<DDD0);
    PORTD=(0<<PORTD7) | (0<<PORTD6) | (0<<PORTD5) | (0<<PORTD4) | (0<<PORTD3) | (0<<PORTD2) | (0<<PORTD1) | (0<<PORTD0);

    // Timer/Counter 0 initialization -> Time (dd:dd:dd)
    TCCR0 = (1<<CS02) | (1<<CS00); // Prescaler = 1024
    TCNT0 = 0;
    OCR0=0x00;

    // Timer/Counter 1 initialization
    // Clock source: System Clock
    // Clock value: 1.953 kHz
    // Mode: Normal top=0xFFFF
    // OC1A output: Disconnected
    // OC1B output: Disconnected
    // Noise Canceler: Off
    // Input Capture on Falling Edge
    // Timer Period: 33.554 s
    // Timer1 Overflow Interrupt: On
    // Input Capture Interrupt: Off
    // Compare A Match Interrupt: Off
    // Compare B Match Interrupt: Off
    TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
    TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (0<<CS12) | (0<<CS11) | (1<<CS10);
    TCNT1H=0x00;
    TCNT1L=0x00;
    ICR1H=0x00;
    ICR1L=0x00;
    OCR1AH=0x00;
    OCR1AL=0x00;
    OCR1BH=0x00;
    OCR1BL=0x00;

    // Timer/Counter 2 initialization --> PWM
    ASSR=0<<AS2;
    TCCR2 = (1<<WGM21) | (1<<WGM20)    // Fast PWM mode
           | (1<<COM21)                // Non-inverting PWM on OC2 (PD7)
           | (0<<COM20)
           | (0<<CS22) | (1<<CS21) | (1<<CS20); // Prescaler = 64
    TCNT2=0x00;
    OCR2=0x00;

    // Timer(s)/Counter(s) Interrupt(s) initialization
    TIMSK=(0<<OCIE2) | (0<<TOIE2) | (0<<TICIE1) | (0<<OCIE1A) | (0<<OCIE1B) | (1<<TOIE1) | (0<<OCIE0) | (1<<TOIE0);

    // External Interrupt(s) initialization
    GICR|=(0<<INT1) | (1<<INT0) | (0<<INT2);
    MCUCR=(0<<ISC11) | (0<<ISC10) | (1<<ISC01) | (0<<ISC00);
    MCUCSR=(0<<ISC2);
    GIFR=(0<<INTF1) | (1<<INTF0) | (0<<INTF2);

    // USART initialization
    UCSRB=(0<<RXCIE) | (0<<TXCIE) | (0<<UDRIE) | (0<<RXEN) | (0<<TXEN) | (0<<UCSZ2) | (0<<RXB8) | (0<<TXB8);

    // Analog Comparator initialization
    ACSR=(1<<ACD) | (0<<ACBG) | (0<<ACO) | (0<<ACI) | (0<<ACIE) | (0<<ACIC) | (0<<ACIS1) | (0<<ACIS0);

    // ADC initialization
    ADMUX=ADC_VREF_TYPE;
    ADCSRA=(1<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (0<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
    SFIOR=(0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0);

    // SPI initialization
    SPCR=(0<<SPIE) | (0<<SPE) | (0<<DORD) | (0<<MSTR) | (0<<CPOL) | (0<<CPHA) | (0<<SPR1) | (0<<SPR0);

    // TWI initialization
    TWCR=(0<<TWEA) | (0<<TWSTA) | (0<<TWSTO) | (0<<TWEN) | (0<<TWIE);

    // Alphanumeric LCD initialization
    // Characters/line: 20
    lcd_init(20);

    // Global enable interrupts
    #asm("sei")
}

char key_to_char(unsigned char key_code) {
    switch (key_code) {
        case 11: return '0';
        case 2:  return '1';
        case 10: return '2';
        case 18: return '3';
        case 1:  return '4';
        case 9:  return '5';
        case 17: return '6';
        case 0:  return '7';
        case 8:  return '8';
        case 16: return '9';
        case 24: return '/';
        case 25: return 'x';
        case 26: return '-';
        case 27: return '+';
        case 19: return '=';
        case 3:  return 'C';
        default: return 0;
    }
}

void update_digits() {
    digit[3] = temp_frac % 10;
    digit[2] = temp_frac / 10;
    digit[1] = temp_int % 10;
    digit[0] = temp_int / 10;
}

void check_pb() {
    delay_ms(200); // Denounce
    segment_on = !btn;
}
