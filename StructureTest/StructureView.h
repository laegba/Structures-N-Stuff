/*
 * StructureView.h
 




  OpenGL visualization adopted from an OpenGL 2D box rotation example:
    http://www.badprog.com/c-opengl-using-glrotatef-to-animate-shapes

 */



#ifndef STRUCTUREVIEW_H_
#define STRUCTUREVIEW_H_


#include "GL/freeglut.h"

/* These are alrready in StrucParams.h */
//#include <iostream>
//#include <cmath>
//#include <vector>

#include "StrucParams.h"  // would be better to just transfer the vectors vectors

#define SCREEN_TITLE         "Test Fortran Rotation and Translation"
#define SCREEN_WIDTH             500
#define SCREEN_HEIGHT            500
#define SCREEN_POSITION_X        650
#define SCREEN_POSITION_Y         25

class StructureView {
public:

  StructureView(int *ac, char *av[]);
  
  void Anim(int *ac, char *av[]);

  virtual ~StructureView();

  void init(int *ac, char *av[]);
  static void managerDisplay(void);
  static void managerIdle(void);
  static void managerResize(int, int);
  static void managerMouse(int, int, int, int);
  static void managerKeyboard(unsigned char, int, int);
};


#endif /* STRUCTUREVIEW_H_ */
