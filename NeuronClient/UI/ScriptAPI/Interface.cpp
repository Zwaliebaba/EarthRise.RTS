#include "LTE/Function.h"
#include "UI/Interface.h"
#include "UI/Widget.h"

TypeAlias(Reference<InterfaceT>, Interface);

VoidFreeFunction(Interface_Add, "Add 'widget' to 'interface'", Interface, interf, Widget, widget) { interf->Add(widget); }
FunctionAlias(Interface_Add, Add);

VoidFreeFunction(Interface_Clear, "Clear all widgets from 'interface'", Interface, interf) { interf->Clear(); }
FunctionAlias(Interface_Clear, Clear);

VoidFreeFunction(Interface_Draw, "Draw all widgets in 'interface' to the screen", Interface, interf) { interf->Draw(); }
FunctionAlias(Interface_Draw, Draw);

VoidFreeFunction(Interface_Update, "Update all widgets in 'interface'", Interface, interf) { interf->Update(); }
FunctionAlias(Interface_Update, Update);
