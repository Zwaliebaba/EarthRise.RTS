#ifndef Module_Settings_h__
#define Module_Settings_h__

#include "LTE/Common.h"
#include "LTE/DeclareFunction.h"
#include "LTE/Generic.h"
#include "LTE/String.h"
#include "UI/Common.h"
#include "UI/Widget.h"

GenericAxis Settings_Axis(
  String const& name,
  Axis const& defValue);

GenericBool Settings_Bool(
  String const& name,
  bool defValue);

GenericButton Settings_Button(
  String const& name,
  Button const& defValue);

GenericColor Settings_Color(
  String const& name,
  Color const& defValue);

GenericFloat Settings_Float(
  String const& name,
  float minimum,
  float maximum,
  float defValue);

DeclareFunctionNoParams(Widget_Settings, Widget)

GenericColor Settings_PrimaryColor();

GenericColor Settings_SecondaryColor();

#endif
