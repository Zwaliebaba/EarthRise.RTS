#ifndef LTE_Keyboard_h__
#define LTE_Keyboard_h__

#include "Common.h"

namespace LTE {
  const char* Key_Name(Key key);

  void Keyboard_AddDown(Key key);

  void Keyboard_AddText(uchar c);

  void Keyboard_Block();

  bool Keyboard_Down(Key key);

  Vector<Key> const& Keyboard_GetKeysPressed();

  bool Keyboard_IsBlocked();

  void Keyboard_ModifyString(String& str, int& cursor);

  bool Keyboard_Pressed(Key key);

  bool Keyboard_Released(Key key);

  void Keyboard_Update(bool hasFocus);

  inline bool Keyboard_Alt() {
    return Keyboard_Down(Key_LAlt) || Keyboard_Down(Key_RAlt);
  }

  inline bool Keyboard_Control() {
    return Keyboard_Down(Key_LControl) || Keyboard_Down(Key_RControl);
  }

  inline bool Keyboard_Shift() {
    return Keyboard_Down(Key_LShift) || Keyboard_Down(Key_RShift);
  }

  inline bool Keyboard_System() {
    return Keyboard_Down(Key_LSystem) || Keyboard_Down(Key_RSystem);
  }
}

#endif
