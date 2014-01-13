
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>
#include <cvd/image_io.h>
#include <cvd/gl_helpers.h>
#include <TooN/TooN.h>

#include <iostream>
#include <sstream>
#include "math.h"

using namespace TooN;

CVD::Image<CVD::Rgb<CVD::byte> > tempImg;
CVD::Image<CVD::Rgb<CVD::byte> > orgImg;

Matrix<8> T8;

Vector<3,int> RGB2YUV(CVD::Rgb<CVD::byte> rgb)
{
    Vector<3> yuv;
    Vector<3> rgbv = makeVector(rgb.red, rgb.green, rgb.blue);
    Matrix<3> m = Data(0.299, 0.587, 0.114,
                       -0.14713, -0.28886, 0.436,
                       0.615, -0.51499, -0.10001);
    return m*rgbv;
}

CVD::Rgb<CVD::byte> YUV2RGB(Vector<3,double> yuv)
{
    CVD::Rgb<CVD::byte> rgb;
    //Vector<3,int> yuv;
    //Vector<3,int> rgbv = makeVector(rgb.red, rgb.green, rgb.blue);
    Matrix<3> m = Data(1, 0, 1.13983,
                       1, -0.39465, -0.5806,
                       1, 2.03211, 0);

     Vector<3> yuvv = m*yuv;
     rgb.red = yuvv[0];
     rgb.green = yuvv[1];
     rgb.blue = yuvv[2];

     return rgb;
}

void filter()
{
    int size = tempImg.size().x;
    Matrix<512> img[3];

    //convert rgb to yuv
    for(int i=0; i< 512; i++)
        for(int j=0; j< 512; j++)
        {
            Vector<3> yuvv =  RGB2YUV(tempImg[i][j]);
            img[0][i][j] = yuvv[0];
            img[1][i][j] = yuvv[1];
            img[2][i][j] = yuvv[2];
        }

    //make T8 matrix
    for(int i=0; i<8; i++)
        for(int j=0; j<8; j++)
        {
            if(i==0)
                T8[i][j] = 0.35355339059;
            else
                T8[i][j] = 0.5*cos((2*j+1)*i*3.14/16);
        }


    for(int i=0; i<size/8; i++)
        for(int j=0; j<size/8; j++)
        {
            //printf("coding: %d\t%d\n",i,j);
//            for(int k=0; k<8; k++)
//                for(int w=0; w<8; w++)
                {
                    //img[0][i*8+k][j*8+w] = T8 * (img[0][i*8+k][j*8+w] * T8.T());
                    img[0].slice(i*8,j*8,8,8) = T8 * (img[0].slice(i*8,j*8,8,8 ) * T8.T());
                    img[1].slice(i*8,j*8,8,8) = T8 * (img[1].slice(i*8,j*8,8,8 ) * T8.T());
                    img[2].slice(i*8,j*8,8,8) = T8 * (img[2].slice(i*8,j*8,8,8 ) * T8.T());

//                    img[0].slice(i*8+2,j*8+2,6,6) = Zeros(6,6);
//                    img[1].slice(i*8+2,j*8+2,6,6) = Zeros(6,6);
//                    img[2].slice(i*8+2,j*8+2,6,6) = Zeros(6,6);
                }
        }

    for(int i=0; i<size/8; i++)
        for(int j=0; j<size/8; j++)
        {
            //printf("encoding: %d\t%d\n",i,j);
//            for(int k=0; k<8; k++)
//                for(int w=0; w<8; w++)
                {
                    //img[0][i*8+k][j*8+w] = T8 * (img[0][i*8+k][j*8+w] * T8.T());
                    img[0].slice(i*8,j*8,8,8) = T8.T() * (img[0].slice(i*8,j*8,8,8 ) * T8);
                    img[1].slice(i*8,j*8,8,8) = T8.T() * (img[1].slice(i*8,j*8,8,8 ) * T8);
                    img[2].slice(i*8,j*8,8,8) = T8.T() * (img[2].slice(i*8,j*8,8,8 ) * T8);
                }
        }


    //convert yuv to rgb
    for(int i=0; i< 512; i++)
        for(int j=0; j< 512; j++)
        {
            Vector<3> yuv;
            yuv[0] = img[0][i][j];
            yuv[1] = img[1][i][j];
            yuv[2] = img[2][i][j];

            CVD::Rgb<CVD::byte> rgb = YUV2RGB(yuv);
            tempImg[i][j] = rgb;
        }

}

void display()
{
  glEnable(GL_TEXTURE_2D);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glRasterPos2f(-1,-0.5);
  glDrawPixels(orgImg);

  glRasterPos2f(0,-0.5);
  glDrawPixels(tempImg);

  glutSwapBuffers();
}

int main(int argc, char** argv) {
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(1024,1024);
  glutCreateWindow("OpenGL glDrawPixels demo");

  CVD::img_load(tempImg,  "marbles.jpg" );
  CVD::img_load(orgImg,  "marbles.jpg" );


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
