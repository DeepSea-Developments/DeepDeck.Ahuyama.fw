// Copyright 2020 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>
#include "esp_compiler.h"
#include "esp_log.h"
#include "menu.h"

#define true 1
#define false 0

#define MY_BORDER_SIZE 1
//#define U8G2_REF_MAN_PIC
//static const char *TAG = "menu";

volatile menu_event_t menu_event;






// typedef struct menu_item_str_t{
//     char * description; //String of the menu item
//     menu_item_action action;        //Action if the item. It can take to a new menu or run a function
//     menu_t * next_menu;               // Pointer of the next menu
//     void (*function_pointer)(void); // Pointer of a function
	
// } menu_item_t;

// struct menu_str_t{
//     char * title;
//     char * subtitle;
// 	menu_item_t * menu_item_array;
// };



enum {
    NONE = -1,
    MAIN_MENU = 0,
    BLUETOOTH_MENU,
    LED_MODE_MENU,
    menu_num
	
} menu_list;

// Titles array

char menu_titles[menu_num][MENU_CHAR_NUM] =
{
    "Main Menu",
    "Bluetooth",
    "LED modes"
};

char menu_subtitles[menu_num][MENU_CHAR_NUM] =
{
    "DeepSea",
    "DeepDeck",
    "DeepDeck"
};

// --------------------Main Menu!-------------------------------
char menu_main_description[5][MENU_CHAR_NUM] =
{
    "Bluetooth",
    "LED configuration",
    "DeepDeck Configuration",
    "Go to Sleep",
    "Exit"
};
menu_item_t m_main_array[] =
{
    //Descripción                 //Acción             //Siguiente menu      ó     //Función
    {menu_main_description[0],    MA_MENU,                BLUETOOTH_MENU,             0},
    {menu_main_description[1],    MA_MENU,                LED_MODE_MENU,              0},
    {menu_main_description[2],    MA_FUNCTION,            NONE,                       &splashScreen},
    {menu_main_description[3],    MA_FUNCTION,            NONE,                       &menu_goto_sleep},
    {menu_main_description[4],    MA_FUNCTION,            NONE,                       &menu_exit},
    {0,                           MA_END,                 0,                          0}
};
// ------------------Bluetooth Menu-------------------------------
char menu_bt_description[2][MENU_CHAR_NUM] =
{
    "Bluetooth 1",
    "Bluetooth 2"
};
menu_item_t m_bluetooth_array[] = 
{
    //Descripción                 //Acción             //Siguiente menu      ó     //Función
    {menu_bt_description[0],      MA_FUNCTION,            NONE,                       &splashScreen},
    {menu_bt_description[1],      MA_FUNCTION,            NONE,                       &splashScreen},
    {0,                           MA_END,                 0,                          0}
};

// ------------------LED modes -------------------------------
char menu_led_mode[5][MENU_CHAR_NUM] =
{
    "Off",
    "Pulsating",
    "Progressive",
    "Rainbow",
    "Solid"
};
menu_item_t m_led_array[] = 
{
    //Descripción                 //Acción             //Siguiente menu      ó     //Función
    {menu_led_mode[0],      MA_FUNCTION,               NONE,                       &menu_rgb_mode_0},
    {menu_led_mode[1],      MA_FUNCTION,               NONE,                       &menu_rgb_mode_1},
    {menu_led_mode[2],      MA_FUNCTION,               NONE,                       &menu_rgb_mode_2},
    {menu_led_mode[3],      MA_FUNCTION,               NONE,                       &menu_rgb_mode_3},
    {menu_led_mode[4],      MA_FUNCTION,               NONE,                       &menu_rgb_mode_4},
    {0,                           MA_END,                 0,                          0}
};

// ----------------------------------- Menu Array ------------------------------------------

menu_t menu_array[menu_num] = 
 {
    // Title                      //Subtitle                      //Item array
    {menu_titles[MAIN_MENU],      menu_subtitles[MAIN_MENU],      &m_main_array},
    {menu_titles[BLUETOOTH_MENU], menu_subtitles[BLUETOOTH_MENU], &m_bluetooth_array},
    {menu_titles[LED_MODE_MENU],  menu_subtitles[LED_MODE_MENU],  &m_led_array},
 };


/*
  Draw a string at x,y
  Center string within w (left adjust if w < pixel len of s)
  
  Side effects:
    u8g2_SetFontDirection(u8g2, 0);
    u8g2_SetFontPosBaseline(u8g2);

*/
void u8g2_DrawUTF8Line(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, const char *s, uint8_t border_size, uint8_t is_invert)
{
  u8g2_uint_t d, str_width;
  u8g2_uint_t fx, fy, fw, fh;

  /* only horizontal strings are supported, so force this here */
  u8g2_SetFontDirection(u8g2, 0);

  /* revert y position back to baseline ref */
  y += u8g2->font_calc_vref(u8g2);   

  /* calculate the width of the string in pixel */
  str_width = u8g2_GetUTF8Width(u8g2, s);

  /* calculate delta d within the box */
  d = 0;
  if ( str_width < w )
  {
    d = w;
    d -=str_width;
    d /= 2;
  }
  else
  {
    w = str_width;
  }

  /* caluclate text box */
  fx = x;
  fy = y - u8g2_GetAscent(u8g2) ;
  fw = w;
  fh = u8g2_GetAscent(u8g2) - u8g2_GetDescent(u8g2) ;

  /* draw the box, if inverted */
  u8g2_SetDrawColor(u8g2, 1);
  if ( is_invert )
  {
    u8g2_DrawBox(u8g2, fx, fy, fw, fh);
  }

  /* draw the frame */
  while( border_size > 0 )
  {
    fx--;
    fy--;
    fw +=2;
    fh +=2;
    u8g2_DrawFrame(u8g2, fx, fy, fw, fh );
    border_size--;
  }

  if ( is_invert )
  {
    u8g2_SetDrawColor(u8g2, 0);
  }
  else
  {
    u8g2_SetDrawColor(u8g2, 1);
  }

  /* draw the text */
  u8g2_DrawUTF8(u8g2, x+d, y, s);

  /* revert draw color */
  u8g2_SetDrawColor(u8g2, 1);

}

/*
  draw several lines at position x,y.
  lines are stored in s and must be separated with '\n'.
  lines can be centered with respect to "w"
  if s == NULL nothing is drawn and 0 is returned
  returns the number of lines in s multiplied with line_height
*/
u8g2_uint_t menu_DrawUTF8Lines(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t line_height, const char *s)
{
  uint8_t i;
  uint8_t cnt;
  u8g2_uint_t yy = 0;
  cnt = u8x8_GetStringLineCnt(s);
  //printf("str=%s\n", s);
  //printf("cnt=%d, y=%d, line_height=%d\n", cnt, y, line_height);
  for( i = 0; i < cnt; i++ )
  {
    //printf("  i=%d, y=%d, line_height=%d\n", i, y, line_height);
    u8g2_DrawUTF8Line(u8g2, x, y, w, u8x8_GetStringLineStart(i, s), 0, 0);
    y+=line_height;
    yy+=line_height;
  }
  return yy;
}

/*
  selection list with string line
  returns line height
*/
// static u8g2_uint_t u8g2_draw_selection_list_line(u8g2_t *u8g2, u8sl_t *u8sl, u8g2_uint_t y, uint8_t idx, const char *s) U8G2_NOINLINE;

static u8g2_uint_t menu_draw_selection_list_line(u8g2_t *u8g2, u8sl_t *u8sl, u8g2_uint_t y, uint8_t idx, menu_t current_menu)
{
  u8g2_uint_t yy;
  uint8_t border_size = 0;
  uint8_t is_invert = 0;
	
  u8g2_uint_t line_height = u8g2_GetAscent(u8g2) - u8g2_GetDescent(u8g2)+MY_BORDER_SIZE;

  /* calculate offset from display upper border */
  yy = idx;
  yy -= u8sl->first_pos;
  yy *= line_height;
  yy += y;

  /* check whether this is the current cursor line */
  if ( idx == u8sl->current_pos )
  {
    border_size = MY_BORDER_SIZE;
    is_invert = 1;
  }

  /* get the line from the array */
  //s = u8x8_GetStringLineStart(idx, s);
  char * s = current_menu.menu_item_array[idx].description;

  /* draw the line */
  if ( s == NULL )
    s = "";
  u8g2_DrawUTF8Line(u8g2, MY_BORDER_SIZE, y, u8g2_GetDisplayWidth(u8g2)-2*MY_BORDER_SIZE, s, border_size, is_invert);
  return line_height;
}

void menu_DrawSelectionList(u8g2_t *u8g2, u8sl_t *u8sl, u8g2_uint_t y, menu_t current_menu)
{
  uint8_t i;
  for( i = 0; i < u8sl->visible; i++ )
  {
    y += menu_draw_selection_list_line(u8g2, u8sl, y, i+u8sl->first_pos, current_menu);
  }
}





uint8_t menu_selection2(u8g2_t *u8g2, menu_t current_menu )
{
  u8sl_t u8sl;
  u8g2_uint_t yy;
  uint8_t start_pos = 0;

  u8g2_uint_t line_height = u8g2_GetAscent(u8g2) - u8g2_GetDescent(u8g2)+MY_BORDER_SIZE;

  uint8_t title_lines = 2;
  uint8_t display_lines;

  
  if ( start_pos > 0 )	
    start_pos--;		


  if ( title_lines > 0 )
  {
    display_lines = (u8g2_GetDisplayHeight(u8g2)-3) / line_height;
    u8sl.visible = display_lines;
    u8sl.visible -= title_lines;
  }
  else
  {
    display_lines = u8g2_GetDisplayHeight(u8g2) / line_height;
    u8sl.visible = display_lines;
  }

  uint8_t num_items = 0;
  while(current_menu.menu_item_array[num_items].action != MA_END)
  {
    num_items++;
  }
  ESP_LOGI("MENU_SELECTION2", "%d", num_items);

  u8sl.total = num_items;
  u8sl.first_pos = 0;
  u8sl.current_pos = start_pos;

  if ( u8sl.current_pos >= u8sl.total )
    u8sl.current_pos = u8sl.total-1;
  if ( u8sl.first_pos+u8sl.visible <= u8sl.current_pos )
    u8sl.first_pos = u8sl.current_pos-u8sl.visible+1;

  u8g2_SetFontPosBaseline(u8g2);
  
  menu_event = MENU_NONE;
  for(;;)
  {
      u8g2_FirstPage(u8g2);
      do
      {
        yy = u8g2_GetAscent(u8g2);
        if ( title_lines > 0 )
        {
          yy += menu_DrawUTF8Lines(u8g2, 0, yy, u8g2_GetDisplayWidth(u8g2), line_height, current_menu.title);
          yy += menu_DrawUTF8Lines(u8g2, 0, yy, u8g2_GetDisplayWidth(u8g2), line_height, current_menu.subtitle);
		
	  u8g2_DrawHLine(u8g2, 0, yy-line_height- u8g2_GetDescent(u8g2) + 1, u8g2_GetDisplayWidth(u8g2));
		
	  yy += 3;
        }
        
        menu_DrawSelectionList(u8g2, &u8sl, yy, current_menu);
      } while( u8g2_NextPage(u8g2) );

      for(;;)
      {
        if ( menu_event == MENU_SELECT )
        {
            ESP_LOGI("MENU","select");
            return u8sl.current_pos+1;

        }		
        else if ( menu_event == MENU_HOME )
        {
            ESP_LOGI("MENU","return");
            return 0;	
        }	
        else if ( menu_event == MENU_NEXT || menu_event == MENU_DOWN )
        {
            ESP_LOGI("MENU","next");
            u8sl_Next(&u8sl);
            break;
        }
        else if ( menu_event == MENU_PREV || menu_event == MENU_UP )
        {
            ESP_LOGI("MENU","Previous");
            u8sl_Prev(&u8sl);
            break;
        }
      }
      menu_event = MENU_NONE;
  }
}




void menu_screen()
{
    uint8_t current_menu = MAIN_MENU;
    uint8_t prev_menu = MAIN_MENU;
    uint8_t selection;

	//selection = menu_selection(&u8g2, menu_main.title, 1, menu_main.subtitle);

    ESP_LOGI("MENU_SCREEN", "Enter");
    menu_ret ret;

    while(true)
    {
        selection = menu_selection2(&u8g2, menu_array[current_menu]);
        if(selection == 0)
        {
            current_menu = prev_menu;
        }
        else
        {
            selection--;
            ESP_LOGI("MENU_SCREEN", "selection %d", selection );
        
            switch( menu_array[current_menu].menu_item_array[selection].action )
            {
                case MA_MENU:
                    prev_menu = current_menu;
                    current_menu = menu_array[current_menu].menu_item_array[selection].next_menu;
                    ESP_LOGI("MENU_SCREEN", "Next menu %d", current_menu );
                break;
                case MA_FUNCTION:
                    ret = menu_array[current_menu].menu_item_array[selection].function_pointer();
                    switch(ret)
                    {
                      case mret_exit:
                        return;
                      break;
                      case mret_goto_main_menu:
                        current_menu = MAIN_MENU;
                        prev_menu = MAIN_MENU;
                      break;
                      default:
                      break;
                    }
                break;
                default:
                break;
            }
        }
    }
    ESP_LOGI("MENU_SCREEN", "Exit");

}

void menu_command(encoder_state_t encoder_action)
{
    switch (encoder_action)
    {
    case ENC_UP:
        menu_event = MENU_UP;
        ESP_LOGI("MENU_COMMAND","UP");
    break;
    case ENC_DOWN:
        menu_event = MENU_DOWN;
        ESP_LOGI("MENU_COMMAND","DOWN");
    break;
    case ENC_BUT_SHORT_PRESS:
        menu_event = MENU_SELECT;
        ESP_LOGI("MENU_COMMAND","SELECT");
    break;
    case ENC_BUT_LONG_PRESS:
        menu_event = MENU_HOME;
        ESP_LOGI("MENU_COMMAND","HOME");
    break;
    
    default:
    break;
    }
}


void menu_init(void)
{
    // char titles[5][110] = {"DeepDeck\nMainMenu", "Bluetooth Options\nLED test mode\nSleep Mode\netc\netc1\netc2"};
    // ESP_LOGI("MENU", "%s",titles[0]);
    // ESP_LOGI("MENU", "%s",titles[1]);
    // menu_main.title = titles[0]; 
    // menu_main.subtitle = titles[1]; 
    // menu_main.menu_item_array = &menu_bluetooth; 


}

uint8_t goto_sleep = false;
menu_ret menu_goto_sleep(void)
{
  ESP_LOGI("menu_goto_sleep","True!");
  goto_sleep = true;
  while(true); // To avoid the function to try to render anything on the screen. it works. believe me. If you dont, write me to nick@dsd.dev :)
  return mret_none;
}

menu_ret menu_exit(void)
{
  return mret_exit;
}

uint8_t menu_get_goto_sleep(void)
{
  if (goto_sleep)
  {
    goto_sleep = false;
    ESP_LOGI("menu_get_goto_sleep","True!");
    return true;
  }
  return goto_sleep;
}

uint8_t menu_send_rgb_mode(uint8_t mode)
{
  int mode_t = mode;
  xQueueSend(keyled_q, &mode_t, 0);

  return mret_none; 
}

// ToDo: Optimize this
uint8_t menu_rgb_mode_0(void)
{
  return menu_send_rgb_mode(0);
}

uint8_t menu_rgb_mode_1(void)
{
  return menu_send_rgb_mode(1);
}

uint8_t menu_rgb_mode_2(void)
{
  return menu_send_rgb_mode(2);
}

uint8_t menu_rgb_mode_3(void)
{
  return menu_send_rgb_mode(3);
}

uint8_t menu_rgb_mode_4(void)
{
  return menu_send_rgb_mode(4);
}
