#include <stdio.h>
#include <stdarg.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/text.h>
#include <types.h>

#include "genC.h"
#include "ri.h"
#include "makefile.h"
#include "database.h"

#include "misc.h"
#include "pipsdbm.h"

#include "wpips.h"

#define DECALAGE_STATUS 100

static Panel_item directory_name, program_name, module_name,
memory_name, message, window_number;


void window_number_notify(Panel_item item, int value, Event *event)
{
  number_of_wpips_windows = (int) xv_get(item, PANEL_VALUE);
}


void show_directory()
{
    xv_set(directory_name, PANEL_VALUE, get_cwd(), 0);
}



void
show_program()
{
   static char *none = "(* none *)";
   char *name = db_get_current_program_name();

   if (name == NULL)
      name = none;

   xv_set(program_name, PANEL_VALUE, name, 0);
}



void
show_module()
{
   static char *none = "(* none *)";
   char *module_name_content = db_get_current_module_name();

   if (module_name_content == NULL)
      module_name_content = none;

   xv_set(module_name, PANEL_VALUE, module_name_content, 0);
}



/*VARARGS0*/
/*
void show_message(va_alist)
va_dcl
{
    static char message_buffer[SMALL_BUFFER_LENGTH];
    va_list args;
    char *fmt;

    va_start(args, char *);

    fmt = va_arg(args, char *);

    (void) vsprintf(message_buffer, fmt, args);

    va_end(args);

    xv_set(message, PANEL_VALUE, message_buffer, 0);
}
*/

void show_message(string message_buffer /*, ...*/)
{
   /* va_list some_arguments;
   static char message_buffer[SMALL_BUFFER_LENGTH]; */

   /* va_start(some_arguments, a_printf_format); */

   /* (void) vsprintf(message_buffer, a_printf_format, some_arguments);*/

   xv_set(message, PANEL_VALUE, message_buffer, 0);
}


void create_status_subwindow()
{
  /* Maintenant on n'utilise plus qu'un seul panel pour la fene^tre
     principale.  En effet, sinon il y a des proble`mes de retrac,age
     sur e'cran couleur. RK, 15/03/1994. */
  
  message = 
    xv_create(main_panel, PANEL_TEXT, 
	      PANEL_VALUE_X, DECALAGE_STATUS,
	      PANEL_VALUE_Y, xv_rows(main_panel, 1),
	      PANEL_LABEL_STRING, "Message:",
	      PANEL_READ_ONLY, TRUE,
	      PANEL_VALUE_DISPLAY_LENGTH, 64,
	      NULL);

  directory_name = 
    xv_create(main_panel, PANEL_TEXT, 
	      PANEL_VALUE_X, DECALAGE_STATUS,
	      PANEL_VALUE_Y, xv_rows(main_panel, 2),
	      PANEL_LABEL_STRING, "Directory:",
	      PANEL_READ_ONLY, TRUE,
	      PANEL_VALUE_DISPLAY_LENGTH, 64,
	      NULL);

  program_name = 
    xv_create(main_panel, PANEL_TEXT,
	      PANEL_VALUE_X, DECALAGE_STATUS,
	      PANEL_VALUE_Y, xv_rows(main_panel, 3),
	      PANEL_LABEL_STRING, "Workspace:",
	      PANEL_VALUE_DISPLAY_LENGTH, 20,
	      PANEL_READ_ONLY, TRUE, 
	      NULL);

  module_name = 
    xv_create(main_panel, PANEL_TEXT, 
	      PANEL_VALUE_X, DECALAGE_STATUS,
	      PANEL_VALUE_Y, xv_rows(main_panel, 4),
	      PANEL_LABEL_STRING, "Module:",
	      PANEL_READ_ONLY, TRUE,
	      PANEL_VALUE_DISPLAY_LENGTH, 20,
	      NULL);

  memory_name = 
    xv_create(main_panel, PANEL_TEXT,
	      PANEL_ITEM_X_GAP, DECALAGE_STATUS,
	      PANEL_VALUE_Y, xv_rows(main_panel, 4),
	      PANEL_LABEL_STRING, "Memory:",
	      PANEL_READ_ONLY, TRUE,
	      PANEL_VALUE_DISPLAY_LENGTH, 10,
	      NULL);

 window_number = 
    xv_create(main_panel, PANEL_NUMERIC_TEXT,
	      PANEL_ITEM_X_GAP, DECALAGE_STATUS,
	      PANEL_VALUE_Y, xv_rows(main_panel, 4),
	      PANEL_LABEL_STRING, "# windows:",
	      PANEL_MIN_VALUE, 1,
	      PANEL_MAX_VALUE, MAX_NUMBER_OF_WPIPS_WINDOWS,
	      PANEL_VALUE, number_of_wpips_windows,
	      PANEL_VALUE_DISPLAY_LENGTH, 2,
	      PANEL_NOTIFY_PROC, window_number_notify,
	      NULL);

  window_fit(main_panel);
  window_fit(main_frame);

  show_directory();
  show_program();
  show_module();
}
