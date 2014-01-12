
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>
#include <cvd/image_io.h>
#include <cvd/gl_helpers.h>

#include <iostream>
#include <sstream>
#include "math.h"

CVD::Image<CVD::Rgb<CVD::byte> > tempImg;

void display()
{

  glEnable(GL_TEXTURE_2D);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glDrawPixels(tempImg);



  glutSwapBuffers();
}

void filter()
{
    int xsize = tempImg.size().x;
    int ysize = tempImg.size().y;

    CVD::Rgb<CVD::byte>  rgb;

    //tempImg.fill(rgb);
    //return;
    for(int i=0; i<ysize; i++)
        for(int j=0; j<xsize; j++)
        {
            //rgb = tempImg[i][j];
            tempImg[i][j].blue= 0;
            tempImg[i][j].red = 0;
            //printf("%d \n",tempImg[i][j].blue);
        }
}

int main(int argc, char** argv) {
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(1024,1024);
  glutCreateWindow("OpenGL glDrawPixels demo");

  CVD::img_load(tempImg,  "marbles.jpg" );

  filter();
  glutDisplayFunc(display);
  //glutReshapeFunc(reshape);
  //glutMouseFunc(mouse_button);
  //glutMotionFunc(mouse_motion);
  //glutKeyboardFunc(keyboard);
  //glutIdleFunc(idle);

  glEnable(GL_DEPTH_TEST);
  //glClearColor(0.0, 0.0, 0.0, 1.0);
  //glPointSize(2);

  glutMainLoop();
}
