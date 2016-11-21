#define __18F97J60
#define __SDCC__
#define THIS_INCLUDES_THE_MAIN_FUNCTION
#include "Include/HardwareProfile.h"

#include <string.h>
#include <stdlib.h>

#include "Include/LCDBlocking.h"
#include "Include/TCPIP_Stack/Delay.h"

#define LOW(a)     (a & 0xFF)
#define HIGH(a)    ((a>>8) & 0xFF)

void DisplayString(BYTE pos, char* text);
void DisplayWORD(BYTE pos, WORD w);
void DisplayIPValue(DWORD IPdw);
size_t strlcpy(char *dst, const char *src, size_t siz);
char* current_time_string(void);
void display_time(void);
void blink_time(void);
void debug_display_time(void);
void display_state(void);
void delay_ms(unsigned long times);


/*************************************************
 Function DisplayWORD:
 writes a WORD in hexa on the position indicated by
 pos.
 - pos=0 -> 1st line of the LCD
 - pos=16 -> 2nd line of the LCD

 __SDCC__ only: for debugging
*************************************************/
#if defined(__SDCC__)
void DisplayWORD(BYTE pos, WORD w) //WORD is a 16 bits unsigned
{
  BYTE WDigit[6]; //enough for a  number < 65636: 5 digits + \0
  BYTE j;
  BYTE LCDPos = 0; //write on first line of LCD
  unsigned radix = 10; //type expected by sdcc's ultoa()

  LCDPos = pos;
  ultoa(w, WDigit, radix);
  for (j = 0; j < strlen((char*)WDigit); j++)
  {
    LCDText[LCDPos++] = WDigit[j];
  }
  if (LCDPos < 32u)
    LCDText[LCDPos] = 0;
  LCDUpdate();
}

// /*************************************************
//  Function DisplayString:
//  Writes string to the LCD display starting at pos
//  since strlcopy writes the final \0, only 31 characters
//  are really usable on the LCD
//  *************************************************/
// void DisplayString(BYTE pos, char* text)
// {
//   BYTE l= strlen(text)+1;/* l must include the finam \0, so, it is strlen+1*/
//    BYTE max= 32-pos;
//    strlcpy((char*)&LCDText[pos], text,(l<max)?l:max );
//    LCDUpdate();
//
// }

/*************************************************
 Function DisplayString:
 Writes the first characters of the string in the remaining
 space of the 32 positions LCD, starting at pos
 (does not use strlcopy, so can use up to the 32th place)
*************************************************/
void DisplayString(BYTE pos, char* text)
{
  BYTE        l = strlen(text);/*number of actual chars in the string*/
  BYTE      max = 32 - pos;  /*available space on the lcd*/
  char       *d = (char*)&LCDText[pos];
  const char *s = text;
  size_t      n = (l < max) ? l : max;
  /* Copy as many bytes as will fit */
  if (n != 0)
    while (n-- != 0)*d++ = *s++;
  LCDUpdate();

}

#endif

const char *state2str[] = 
{
  "STARTUP",
  "WAIT_FOR_RELEASE",
  "WAIT_HOURS",
  "WAIT_MINS",
  "WAIT_SECS",
  "SET_TIME",
  "INC_HOURS",
  "INC_MINS",
  "INC_SECS",
  "INC_SECS_2",
  "INC_MINS_2",
  "INC_HOURS_2",
  "RESET",
  "INC_HOURS_WAIT",
  "INC_MINS_WAIT",
  "INC_SECS_WAIT",
  "DEBOUNCE"
};

typedef enum
{
    STARTUP,
    WAIT_FOR_RELEASE,
    WAIT_HOURS,
    WAIT_MINS,
    WAIT_SECS,
    SET_TIME,
    INC_HOURS,
    INC_MINS,
    INC_SECS,
    INC_SECS_2,
    INC_MINS_2,
    INC_HOURS_2,
    RESET,
    INC_HOURS_WAIT,
    INC_MINS_WAIT,
    INC_SECS_WAIT,
    DEBOUNCE
} fsm_state;

fsm_state state;

/*-------------------------------------------------------------------------
 *
 * strlcpy.c
 *    strncpy done right
 *
 * This file was taken from OpenBSD and is used on platforms that don't
 * provide strlcpy().  The OpenBSD copyright terms follow.
 *-------------------------------------------------------------------------
 */

/*  $OpenBSD: strlcpy.c,v 1.11 2006/05/05 15:27:38 millert Exp $    */

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 * Function creation history:  http://www.gratisoft.us/todd/papers/strlcpy.html
 */
size_t
strlcpy(char *dst, const char *src, size_t siz)
{
  char       *d = dst;
  const char *s = src;
  size_t      n = siz;

  /* Copy as many bytes as will fit */
  if (n != 0)
  {
    while (--n != 0)
    {
      if ((*d++ = *s++) == '\0')
        break;
    }
  }

  /* Not enough room in dst, add NUL and traverse rest of src */
  if (n == 0)
  {
    if (siz != 0)
      *d = '\0';          /* NUL-terminate dst */
    while (*s++)
      ;
  }

  return (s - src - 1);       /* count does not include NUL */
}

void delay_ms(unsigned long times)
{
  unsigned long i = 25000*times;
  while(i-- > 0);
}

unsigned char current_hours = 0;
unsigned char current_minutes = 0;
unsigned char current_seconds = 0;

char* current_time_string(void)
{
    char string[16];
    unsigned char i = 0;
    for (;i<8;i++)
        string[i] = '0';


    if (current_hours > 9)
    {
        string[1] += current_hours % 10;
        string[0] += current_hours/10;
    }
    else
      string[1] += current_hours;

    string[2] = ':';

    if (current_minutes > 9)
    {
        string[4] += current_minutes % 10;
        string[3] += current_minutes/10;
    }
    else
      string[4] += current_minutes;

    string[5] = '.';

    if (current_seconds > 9)
    {
        string[7] += current_seconds % 10;
        string[6] += current_seconds/10;
    }
    else
      string[7] += current_seconds;

    return string;
}

void debug_display_time(void)
{
  LED_PUT(0x00);
  //DelayMs(40);
  display_time();
  LED_PUT(0x01);
}

/* Displays current time on LCD display */
void display_time(void)
{
  DisplayString(16, current_time_string());
}

void display_state(void)
{
  DisplayString(0,"                ");
  DisplayString(0,state2str[state]);
}

/* Blinks current time on display */
void blink_time(void)
{
  DisplayString(16,"");
  DelayMs(100);
  display_time();
}


void main(void)
{
  //unsigned int i;
  //WORD w;
  int set_time = 0;
  unsigned char hms = 0;
  LED0_TRIS = 0; //configure 1st led pin as output (yellow)
  LED1_TRIS = 0; //configure 2nd led pin as output (red)
  LED2_TRIS = 0; //configure 3rd led pin as output (red)

  BUTTON0_TRIS = 1; //configure button0 as input
  BUTTON1_TRIS = 1; //configure button1 as input

  LCDInit();
  DelayMs(100);
  LED_PUT(0x00);

  /* Here we write a few chars in the external buffer of the ethernet
     interface of this PIC microcomputer */
  // w = 5;
  // EWRPTL = LOW(w);
  // EWRPTH = HIGH(w);
  // EDATA = '1';
  // EDATA = '2';
  // EDATA = '3';
  // EDATA = '4';
  // EDATA = '5';
  // EDATA = 0;

  // DisplayString (0, "Test of Ethernet buffer"); //first arg is start position
  // // on 32 positions LCD

  //DisplayString (16, "      Push But1");

  /* Here we copy what we wrote in the buffer on the LCD */
  // for (i = 16; i < 21; i++)
  // {
  //   LCDText[i] =  EDATA;
  // }
  // LCDUpdate();

  /* main loop of this toy program: turn leds on if button 1 is pressed */
  //char[8] times = "00:00.00";

   // ERDPTL = 5;
  // ERDPTH = 0;
  //DisplayString(16, current_time_string());
  state = STARTUP;
  while(1)
  {
    display_state();
    display_time();
    if(state == STARTUP)
    {
      current_hours = 0;
      current_minutes = 0;
      current_seconds = 0;
      hms = 0;
      
      if (BUTTON0_IO == 0u && BUTTON1_IO == 0u) state = WAIT_FOR_RELEASE;
    }
    else if (state == WAIT_FOR_RELEASE)
    {
      if (BUTTON0_IO == 1u && BUTTON1_IO == 1u)
      {
        if(hms == 0)
        {

        }
        hms++;
        state = SET_TIME;
      }
      if(hms > 3)
      {
          hms = 0;
          state = INC_SECS_2;
      }
    }
    else if (state == SET_TIME)
    {
      if (BUTTON0_IO == 0u && BUTTON1_IO == 0u) state = WAIT_FOR_RELEASE;
      else if(BUTTON0_IO == 0u && BUTTON1_IO == 1u)
      {
        if(hms==1)
          state = INC_HOURS;
        else if(hms==2)
          state = INC_MINS;
        else if(hms==3)
          state = INC_SECS;
      }
    }
    else if (state == INC_HOURS)
    {
      current_hours = (current_hours < 23? current_hours+1:0);
      state = INC_HOURS_WAIT;
    }
    else if (state == INC_HOURS_WAIT)
    {
      if(BUTTON0_IO == 1u)
        state = SET_TIME;
    }
    else if (state == INC_MINS)
    {
      current_minutes = (current_minutes < 59? current_minutes+1:0);
      state = INC_MINS_WAIT;
    }
    else if (state == INC_MINS_WAIT)
    {
      if(BUTTON0_IO == 1u)
        state = SET_TIME;
    }
    else if (state == INC_SECS)
    {
      current_seconds = (current_seconds < 59? current_seconds+1:0);
      state = INC_SECS_WAIT;
    }
    else if (state == INC_SECS_WAIT)
    {
      if(BUTTON0_IO == 1u)
        state = SET_TIME;
    }

    else if (state == INC_SECS_2)
    {
      DelayMs(100);
      current_seconds++;
      if(current_seconds==60)
        state = INC_MINS_2;
      else if (BUTTON0_IO == 0u && BUTTON1_IO == 0u) state = WAIT_FOR_RELEASE;
    }
    else if (state == INC_MINS_2)
    {
      current_minutes++;
      current_seconds=0;
      state = (current_minutes == 60? INC_HOURS_2:INC_SECS_2);
    }
    else if (state == INC_HOURS_2)
    {
      current_hours++;
      current_minutes=0;
      state = (current_hours == 24? RESET:INC_SECS_2);
    }
    else if (state == RESET)
    {
      current_hours = 0;
      current_minutes = 0;
      current_seconds = 0;
      state = INC_SECS_2;
    }
    else
    {
      state = STARTUP;
    }
  }

  // while(1)
  // {
  //   switch(state)
  //   {
  //       case STARTUP:
  //           //LED_PUT(0x00);
  //           //blink_time();
  //           display_time();
  //           LED_PUT(0x07);  //turn on the 3 red leds
  //           current_hours = 0;
  //           current_minutes = 0;
  //           current_seconds = 0;
  //           hms = 0;
            
  //           if(BUTTON0_IO == 0u && BUTTON1_IO == 0u)
  //               state = WAIT_FOR_RELEASE;
  //           break;

  //       case WAIT_FOR_RELEASE:
  //           blink_time();
  //           if (BUTTON0_IO == 1u && BUTTON1_IO == 1u)
  //               state = SET_TIME;
  //           else if(hms > 2)
  //           {
  //               hms = 0;
  //               state = INC_SECS_2;
  //           }
  //           break;

  //       case SET_TIME:
  //           blink_time();
  //           if (BUTTON0_IO == 0u)
  //           {
  //               if (BUTTON1_IO == 0u)
  //               {
  //                   hms++;
  //                   state = WAIT_FOR_RELEASE;
  //               }
  //               else if (hms == 0)
  //                   state = INC_HOURS;
  //               else if(hms == 1)
  //                   state = INC_MINS;
  //               else if(hms == 2)
  //                 state = INC_SECS;
  //           }
  //           break;

  //       case INC_HOURS:
  //           current_hours = (current_hours < 23? current_hours+1:0);
  //           state = INC_HOURS_WAIT;
  //           break;
  //       case INC_HOURS_WAIT:
  //           if(BUTTON1_IO == 1u)
  //             state = SET_TIME;
  //           break;

  //       case INC_MINS:
  //           current_minutes = (current_minutes < 59? current_minutes+1:0);
  //           state = INC_MINS_WAIT;
  //           break;
  //       case INC_MINS_WAIT:
  //           if(BUTTON1_IO == 1u)
  //             state = SET_TIME;
  //           break;

  //       case INC_SECS:
  //           current_seconds = (current_seconds < 59? current_seconds+1:0);
  //           state = INC_SECS_WAIT;
  //           break;
  //       case INC_SECS_WAIT:
  //           if(BUTTON1_IO == 1u)
  //             state = SET_TIME;
  //           break; 

  //       case INC_SECS_2:
  //           for(i=0;i<10;i++) DelayMs(100);
  //           current_seconds++;
  //           display_time();
  //           if(current_seconds==60)
  //             state = INC_MINS_2;
  //           break;
  //       case INC_MINS_2:
  //           current_minutes++;
  //           state = (current_minutes == 60? INC_HOURS_2:INC_SECS_2);
  //           break;

  //       case INC_HOURS_2:
  //           current_hours++;
  //           state = (current_hours == 24? RESET:INC_SECS_2);
  //           break;

  //       case RESET:
  //           current_hours = 0;
  //           current_minutes = 0;
  //           current_seconds = 0;
  //           state = INC_SECS_2;
  //           break;

  //   } //end case
  // }   //end while

  
  // set_time = 0;

  //   while (1)
  //   {
  //   //ERDPTL = 5;
  //   //ERDPTH = 0;

  //       if (BUTTON0_IO == 0u && BUTTON1_IO == 0u)
  //           set_time = 1;

  //       if (set_time == 1 && (BUTTON0_IO == 1u && BUTTON1_IO == 1u))
  //           break;
  //   }
    // while(1)
    // {
    //     set_time = 0;
    //     if (BUTTON0_IO == 0u) //If Button 0 is pressed
    //     {
    //         current_hours++;
    //       //  DisplayString(16, "00:00.01        ");
    //       LED_PUT(0x07);  //turn on the 3 red leds
    //     }
    //     else
    //       LED_PUT(0x00);  //turn them off

    //     DisplayString(16, current_time_string());
    //     DelayMs(100);
    //     //for (i = 0; i < 1000; i++);

    // }

}