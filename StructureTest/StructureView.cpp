/*

  StructureView.cpp is part of Program StrucTest
    Copyright (C) 2015  Michael Salay,      Mike.Salay at gmail

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


  OpenGL visualization adopted from an OpenGL 2D box rotation example:
    http://www.badprog.com/c-opengl-using-glrotatef-to-animate-shapes
  
 */

#include "StructureView.h"


using namespace std;

   GLUquadricObj *obj = gluNewQuadric(); // required to draw cylinders,spheres

  extern "C" {
    void structurecalc_(int *d ,int *np ,int *s , float x[], float v[]);
  };


  const float PI=2*acos(0); // define PI
  const float deg2rad= PI/180; // conversion factor from degrees to radians


 /* Default Calculation */

  int  D=2;                     // dimension (2D or 3D)
  int np=4;                     // number of parts
  int  s=3;                     // size of array
//  vector <float> pos (24);      // position and velocity:  vx,vy,w
  vector <float> x (12);        // position and rotation
  vector <float> v (12);        // velocity

  vector <float> color 
    {1., 0., 0.,
    1.0, 1.0, 1.0,
    0.7, 1.0, 0.3,
    0.0, 0.0, 1.0};

  float k=100,b=1,m=1,q=0.333;  // spring, damp, mass, and inertial parameters


  float w=1, h=w/2.;            // default object dimensions 
  float xo=-3, yo=1, r=45;      // default initial object position and rotation

  vector<int> id;               // Type ID
  vector<string> did;           // Draw ID 





// StructureView::StructureView(int *ac, char *av[]) {
StructureView::StructureView(int *ac, char *av[]) {

  /* Set Default Positions */
  float rr=45*deg2rad; // initial rotation in radians
  for (int i = 0; i < np; i++)
  {
     x[i*3]=xo+i*w*cos(rr);         // xposition
     x[i*3+1]=yo+i*w*sin(rr);       // yposition
     x[i*3+2]=rr;                   // rotation
  }

  /* Obtain and Process Input */
  if (*ac == 2) 
  {
    string fileName=av[1];
    StrucParams p(fileName);
//    p.Summary(); // summarize/display all input // uncomment to view processed input
//    cout << endl;

    D=p.Dimension;
    np=p.typeID.size();
    s=p.Size; 

    x.clear();x=p.Position;
    v.clear();v=p.Velocity;

    k=p.Spring;
    b=p.Damp;
    
//   m masses


    
  } /* Done with Input Processing*/


// Uncomment to view initial positions and velocities 
  cout << "x: ";
  for (auto c : x) 
      cout << ' ' << c;
  cout << endl;
  cout << "v: ";
  for (auto c : v) 
      cout << ' ' << c;
  cout << endl;


  Anim(ac, av);
  
}

 
void StructureView::Anim(int *ac, char *av[]) {
    glutInit(ac, av);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutInitWindowPosition(SCREEN_POSITION_X, SCREEN_POSITION_Y);
    glutCreateWindow(SCREEN_TITLE);
//    this->
    init(ac, av);
    glutDisplayFunc(&managerDisplay);
    glutReshapeFunc(&managerResize);
    glutMouseFunc(&managerMouse);
    glutKeyboardFunc(&managerKeyboard);
    glutIdleFunc(&managerIdle);
    glutMainLoop();
}

/*
 * Deleting
 */
StructureView::~StructureView() {}

/*
 * Init
 */
void StructureView::init(int *ac, char *av[])
{
  glClearColor(0.0, 0.0, 0.0, 0.0); // already defaults
//  glShadeModel(GL_FLAT);            // already defaults

  if (D == 3)
  {
    glEnable(GL_DEPTH_TEST);

    GLfloat light_diffuse[] = {0.5, 0.5, 1.0, 1.0};  
    GLfloat light_position[] = {-5.0, 5.0, 5.0, 0.0};  /* Direction. */
    /* Enable a single OpenGL light. */
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    glEnable(GL_COLOR_MATERIAL); // adopts fcolor as material property
  }
}

/*
 * Displaying 2D objects on the screen with positions controlled by fortran
 * Initial positions, rotations, and velocities (all zeros) set manually for now
 */
void StructureView::managerDisplay(void)
{
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


  for( int i = 0; i < np ; i +=1 )
  {
    glPushMatrix();
    switch (D)
    {
    case 3: /* cube */  
      glTranslatef(x[s*i], x[s*i+1], 0.0);
//      glRotatef(45., 0.0, 0.0,1.0);
      glRotatef(x[s*i+5]/deg2rad, 0.0, 0.0,-1.0);
      glRotatef(90., 1.0, 0.0,0.0);  // additional cyl rotation to make vert
      glColor3f(color[3*i], color[3*i+1],color[3*i+2]);
//      glutSolidCube(w);  
      gluCylinder(obj, h, h, w, 8, 1);
    break;
    case 2: /* rect */  
      glTranslatef(x[s*i], x[s*i+1], 0.0);
      glRotatef(x[3*i+2]/deg2rad, 0.0, 0.0,-1.0);
      glColor3f(color[3*i], color[3*i+1],color[3*i+2]);
      glRectf(-h, -h, h, h);
    break;
    }
    glPopMatrix();

  }


/*
// Other test drawings 
      glPushMatrix();
//      glTranslatef(x[s*i], x[3*i+1], 0.0);
        glRotatef(90, 1.0, 1.0,2.);
        glColor3f(0.8, 0.5,0.8);
//      glutWireCube(1.);  
        glutSolidCube(1.);  
      glPopMatrix();


      glPushMatrix();
        glTranslatef(-2, -2, -2.);
        glRotatef(90, 1.0, 1.0,1.0);
        glColor3f(0.5, 0.5,0.85);
        GLUquadricObj *obj = gluNewQuadric();
//      gluCylinder(	GLUquadric*  	quad,
// 	GLdouble  	base,
// 	GLdouble  	top,
// 	GLdouble  	height,
// 	GLint  	slices,
// 	GLint  	stacks);
//      glutSolidCube(3.);  

        gluCylinder(obj, 1., 1., 2., 8, 1);
      glPopMatrix();


      glPushMatrix();
        glTranslatef(2, 2, -2.);
//      glRotatef(90, -1.0, 1.0,1.0);
        glColor3f(0.5, 0.85,0.5);
//      GLUquadricObj *obj = gluNewQuadric();

        gluSphere(obj, 1., 16, 16);
      glPopMatrix();
*/

  glutSwapBuffers();
}

/*
 * The animation
 */
void StructureView::managerIdle(void)
{
  usleep(10000);

/*  Calculate positions with a Fortran subroutine */

  structurecalc_(&D,&np,&s,&x[0],&v[0]);      


//  cout << "xv:";
  cout << endl;
  for( int i = 0; i < np ; i +=1 )
  {
    for( int j = 0; j < s ; j +=1 )
      cout << " " << x[i*s+j];
    for( int j = 0; j < s ; j +=1 )
      cout << " " << v[i*s+j];
//    for( int j = 0; j < s ; j +=1 )
//      cout << ' ' << v(j);
    cout << endl;
  }

    
    
    glutPostRedisplay();

}

/*
 * Allow the resizing of the window, preserving perspective.
 */
void StructureView::managerResize(int w, int h)
{
   glViewport(0, 0, (GLsizei)h, (GLsizei)h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
//   glOrtho(-5.,5.,-5.,5.,-5.,5.);     // 2D REndering (use sizeh and sizew)
/* note that perspective z is relative to camera position */
   gluPerspective(90.,1.,.01,100.);  // camera must be closer than far clip
//   glFrustum(-5,+5,-5,+5,-100,100);  // 3D REndering "complex"
   gluLookAt(0.,0.,5., // must be after model view
             0.,0.,0.,
             0,1,0);
   glMatrixMode(GL_MODELVIEW);
/* gluLookAt must be called after Model View */
   glLoadIdentity();
}



/*
 * Manage the mouse, if the left button is clicked, we revive the animation.
 * If the right button is clicked, we stop the animation.
 */
void StructureView::managerMouse(int button, int state, int x, int y)
{
    switch (button) {
        case GLUT_LEFT_BUTTON:
            if (state == GLUT_DOWN)
                glutIdleFunc(&managerIdle);
        break;
        case GLUT_RIGHT_BUTTON:
            if (state == GLUT_DOWN)
                glutIdleFunc(NULL);
        break;
        default:
        break;
    }
    (void)(x);
    (void)(y);
}

/*
 * Manage the keyboard, 27 = ESC key.
 */
void StructureView::managerKeyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27:
        {
            exit(0);
        }
        break;
    }
    (void)(x);
    (void)(y);
}

