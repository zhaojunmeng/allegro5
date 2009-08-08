/*         ______   ___    ___
 *        /\  _  \ /\_ \  /\_ \
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      New keyboard API.
 *
 *      By Peter Wang.
 *
 *      See readme.txt for copyright information.
 */

/* Title: Keyboard routines
 */


#define ALLEGRO_NO_COMPATIBILITY

#include "allegro5/allegro5.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_events.h"
#include "allegro5/internal/aintern_keyboard.h"
#include "allegro5/internal/aintern_system.h"



/* the active keyboard driver */
static ALLEGRO_KEYBOARD_DRIVER *new_keyboard_driver = NULL;

/* mode flags */
/* TODO: use the config system for these */
bool _al_three_finger_flag = true;
bool _al_key_led_flag = true;



/* Provide a default naming for the most common keys. Keys whose
 * mapping changes dependind on the layout aren't listed - it's up to
 * the keyboard driver to do that.  All keyboard drivers should
 * provide their own implementation though, especially if they use
 * positional mapping.
 */
const char *_al_keyboard_common_names[/* leave empty */] =
{
   "(none)",     "A",          "B",          "C",
   "D",          "E",          "F",          "G",
   "H",          "I",          "J",          "K",
   "L",          "M",          "N",          "O",
   "P",          "Q",          "R",          "S",
   "T",          "U",          "V",          "W",
   "X",          "Y",          "Z",          "0",
   "1",          "2",          "3",          "4",
   "5",          "6",          "7",          "8",
   "9",          "PAD 0",      "PAD 1",      "PAD 2",
   "PAD 3",      "PAD 4",      "PAD 5",      "PAD 6",
   "PAD 7",      "PAD 8",      "PAD 9",      "F1",
   "F2",         "F3",         "F4",         "F5",
   "F6",         "F7",         "F8",         "F9",
   "F10",        "F11",        "F12",        "ESCAPE",
   "KEY60",      "KEY61",      "KEY62",      "BACKSPACE",
   "TAB",        "KEY65",      "KEY66",      "ENTER",
   "KEY68",      "KEY69",      "BACKSLASH",  "KEY71",
   "KEY72",      "KEY73",      "KEY74",      "SPACE",
   "INSERT",     "DELETE",     "HOME",       "END",
   "PGUP",       "PGDN",       "LEFT",       "RIGHT",
   "UP",         "DOWN",       "PAD /",      "PAD *",
   "PAD -",      "PAD +",      "PAD DELETE", "PAD ENTER",
   "PRINTSCREEN","PAUSE",      "KEY94",      "KEY95",
   "KEY96",      "KEY97",      "KEY98",      "KEY99",
   "KEY100",     "KEY101",     "KEY102",     "PAD =",
   "KEY104",     "KEY105",     "KEY106",     "KEY107",
   "KEY108",     "KEY109",     "KEY110",     "KEY111",
   "KEY112",     "KEY113",     "KEY114",     "KEY115",
   "KEY116",     "KEY117",     "KEY118",     "KEY119",
   "KEY120",     "KEY121",     "KEY122",     "KEY123",
   "KEY124",     "KEY125",     "KEY126",     "KEY127",
   "KEY128",     "KEY129",     "KEY130",     "KEY131",
   "KEY132",     "KEY133",     "KEY134",     "KEY135",
   "KEY136",     "KEY137",     "KEY138",     "KEY139",
   "KEY140",     "KEY141",     "KEY142",     "KEY143",
   "KEY144",     "KEY145",     "KEY146",     "KEY147",
   "KEY148",     "KEY149",     "KEY150",     "KEY151",
   "KEY152",     "KEY153",     "KEY154",     "KEY155",
   "KEY156",     "KEY157",     "KEY158",     "KEY159",
   "KEY160",     "KEY161",     "KEY162",     "KEY163",
   "KEY164",     "KEY165",     "KEY166",     "KEY167",
   "KEY168",     "KEY169",     "KEY170",     "KEY171",
   "KEY172",     "KEY173",     "KEY174",     "KEY175",
   "KEY176",     "KEY177",     "KEY178",     "KEY179",
   "KEY180",     "KEY181",     "KEY182",     "KEY183",
   "KEY184",     "KEY185",     "KEY186",     "KEY187",
   "KEY188",     "KEY189",     "KEY190",     "KEY191",
   "KEY192",     "KEY193",     "KEY194",     "KEY195",
   "KEY196",     "KEY197",     "KEY198",     "KEY199",
   "KEY200",     "KEY201",     "KEY202",     "KEY203",
   "KEY204",     "KEY205",     "KEY206",     "KEY207",
   "KEY208",     "KEY209",     "KEY210",     "KEY211",
   "KEY212",     "KEY213",     "KEY214",     "LSHIFT",
   "RSHIFT",     "LCTRL",      "RCTRL",      "ALT",
   "ALTGR",      "LWIN",       "RWIN",       "MENU",
   "SCROLLLOCK", "NUMLOCK",    "CAPSLOCK"
};

ALLEGRO_STATIC_ASSERT(
   sizeof(_al_keyboard_common_names) / sizeof(_al_keyboard_common_names[0])
   == ALLEGRO_KEY_MAX);



/* Function: al_is_keyboard_installed
 */
bool al_is_keyboard_installed(void)
{
   return (new_keyboard_driver ? true : false);
}



/* Function: al_install_keyboard
 */
bool al_install_keyboard(void)
{
   if (new_keyboard_driver)
      return true;

   //FIXME: seems A4/A5 driver list stuff doesn't quite agree right now
   if (al_system_driver()->vt->get_keyboard_driver) {
       new_keyboard_driver = al_system_driver()->vt->get_keyboard_driver();
       if (!new_keyboard_driver->init_keyboard()) {
          new_keyboard_driver = NULL;
          return false;
       }
       _al_add_exit_func(al_uninstall_keyboard, "al_uninstall_keyboard");
       return true;
   }
   
   return false;

   /*
   if (system_driver->keyboard_drivers)
      driver_list = system_driver->keyboard_drivers();
   else
      driver_list = _al_keyboard_driver_list;

   for (i=0; driver_list[i].driver; i++) {
      new_keyboard_driver = driver_list[i].driver;
      name = get_config_text(new_keyboard_driver->keydrv_ascii_name);
      new_keyboard_driver->keydrv_name = name;
      new_keyboard_driver->keydrv_desc = name;
      if (new_keyboard_driver->init_keyboard())
	 break;
   }

   if (!driver_list[i].driver) {
      new_keyboard_driver = NULL;
      return false;
   }

   //set_leds(-1);

   _al_add_exit_func(al_uninstall_keyboard, "al_uninstall_keyboard");


   return true;
   */
}



/* Function: al_uninstall_keyboard
 */
void al_uninstall_keyboard(void)
{
   if (!new_keyboard_driver)
      return;

   //set_leds(-1);

   new_keyboard_driver->exit_keyboard();
   new_keyboard_driver = NULL;
}



/* This was in the public API but its only purpose is now served by
 * al_get_keyboard_event_source().
 */
ALLEGRO_KEYBOARD *al_get_keyboard(void)
{
   ASSERT(new_keyboard_driver);
   {
      ALLEGRO_KEYBOARD *kbd = new_keyboard_driver->get_keyboard();
      ASSERT(kbd);

      return kbd;
   }
}



/* Function: al_set_keyboard_leds
 */
bool al_set_keyboard_leds(int leds)
{
   ASSERT(new_keyboard_driver);

   if (new_keyboard_driver->set_keyboard_leds)
      return new_keyboard_driver->set_keyboard_leds(leds);

   return false;
}



/* Function: al_keycode_to_name
 */
const char *al_keycode_to_name(int keycode)
{
   const char *name = NULL;

   ASSERT(new_keyboard_driver);
   ASSERT((keycode >= 0) && (keycode < ALLEGRO_KEY_MAX));

   if (new_keyboard_driver->keycode_to_name)
      name = new_keyboard_driver->keycode_to_name(keycode);

   if (!name)
      name = _al_keyboard_common_names[keycode];

   ASSERT(name);

   return name;
}



/* Function: al_get_keyboard_state
 */
void al_get_keyboard_state(ALLEGRO_KEYBOARD_STATE *ret_state)
{
   ASSERT(new_keyboard_driver);
   ASSERT(ret_state);

   new_keyboard_driver->get_keyboard_state(ret_state);
}



/* Function: al_key_down
 */
bool al_key_down(const ALLEGRO_KEYBOARD_STATE *state, int keycode)
{
   return _AL_KEYBOARD_STATE_KEY_DOWN(*state, keycode);
}



/* Function: al_get_keyboard_event_source
 */
ALLEGRO_EVENT_SOURCE *al_get_keyboard_event_source(void)
{
   ALLEGRO_KEYBOARD *keyboard = al_get_keyboard();

   return (keyboard) ? &keyboard->es : NULL;
}


/*
 * Local Variables:
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
/* vim: set sts=3 sw=3 et: */
