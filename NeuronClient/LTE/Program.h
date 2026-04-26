#ifndef LTE_Program_h__
#define LTE_Program_h__

#include "Window.h"

struct Program {
  Window window;
  bool deleted;

  Program();
  virtual ~Program();

  void Delete();
  void Execute();

  virtual void OnInitialize() = 0;
  virtual void OnUpdate() = 0;
};

bool Program_InStaticSection();

#endif
