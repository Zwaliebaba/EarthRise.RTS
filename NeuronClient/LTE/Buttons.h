#ifndef LTE_Buttons_h__
#define LTE_Buttons_h__

#include "Button.h"

namespace LTE {
  Button Button_And(Button const& a, Button const& b);

  Button Button_Axis(Axis const& axis, float sign);

  Button Button_Capture();

  Button Button_Inverted(Button const& button);

  Button Button_Joy(uint joyIndex, uint buttonIndex);

  Button Button_Key(Key key);

  Button Button_Mouse(MouseButton mouseButton);

  Button Button_Or(Button const& a, Button const& b);

  inline Button Button_AndAlt(Button const& button) {
    return Button_And(button,
      Button_Or(
        Button_Key(Key_LAlt),
        Button_Key(Key_RAlt)));
  }

  inline Button Button_AndControl(Button const& button) {
    return Button_And(button,
      Button_Or(
        Button_Key(Key_LControl),
        Button_Key(Key_RControl)));
  }

  inline Button Button_AndShift(Button const& button) {
    return Button_And(button,
      Button_Or(
        Button_Key(Key_LShift),
        Button_Key(Key_RShift)));
  }

  inline Button Button_LeftStick() {
#ifdef LIBLT_WINDOWS
    return Button_Joy(0, 8);
#else
    return Button_Joy(0, 9);
#endif
  }

  inline Button Button_RightStick() {
#ifdef LIBLT_WINDOWS
    return Button_Joy(0, 9);
#else
    return Button_Joy(0, 10);
#endif
  }
}

#endif
