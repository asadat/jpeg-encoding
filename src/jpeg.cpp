
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
CVD::Image<CVD::Rgb<CVD::byte> > orgImg_;
CVD::Image<CVD::Rgb<CVD::byte> > orgImg;

Matrix<8,8,double> T8;

Matrix<8,8,double> quantizers = Data (
            16, 11, 10, 16, 24, 40, 51, 61,
             12, 12, 14, 19, 26, 58, 60, 55,
             14, 13, 16, 24, 40, 57, 69, 56,
             14, 17, 22, 29, 51, 87, 80, 62,
             18, 22, 37, 56, 68, 109, 103, 77,
             24, 35, 55, 64, 81, 104, 113, 92,
             49, 64, 78, 87, 103, 121, 120, 101,
             72, 92, 95, 98, 112, 100, 103, 99);

Vector<3> RGB2YUV(CVD::Rgb<CVD::byte> rgb)
{
    Vector<3> yuv;
    Vector<3> rgbv = makeVector(rgb.red-128, rgb.green-128, rgb.blue-128);
    Matrix<3> m = Data(0.299, 0.587, 0.114,
                       -0.14713, -0.28886, 0.436,
                       0.615, -0.51499, -0.10001);
    return m*rgbv;
}

int cut(int a, int b)
{
    if(a<0)
        return 0;
    else
        return a;

    //return a<b?a:b;
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
     rgb.red = cut(yuvv[0]+128,255);
     rgb.green = cut(yuvv[1]+128,255);
     rgb.blue = cut(yuvv[2]+128,255);

     return rgb;
}

void quantize(Matrix<8,8,double> &m)
{
    for(int i=0; i<8; i++)
        for(int j=0; j<8; j++)
            m[i][j] = ((m[i][j]/ (quantizers[i][j])));
}

void scale(Matrix<8,8,double> &m)
{
    for(int i=0; i<8; i++)
        for(int j=0; j<8; j++)
            m[i][j] = m[i][j] * quantizers[i][j];
}

void filter()
{
    int size = tempImg.size().x;
    Matrix<512,512,int> img[3];

    for(int i=0; i< 512; i++)
        for(int j=0; j< 512; j++)
        {
            orgImg[i][j] = tempImg[511-i][j];
        }

    //convert rgb to yuv
    for(int i=0; i< 512; i++)
        for(int j=0; j< 512; j++)
        {
            Vector<3> yuvv =  RGB2YUV(tempImg[i][j]);
            img[0][511-i][j] = yuvv[0];
            img[1][511-i][j] = yuvv[1];
            img[2][511-i][j] = yuvv[2];
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
            Matrix<8,8,double> img8x8[3];
            img8x8[0] = T8 * (img[0].slice(i*8,j*8,8,8 ) * T8.T());
            img8x8[1] = T8 * (img[1].slice(i*8,j*8,8,8 ) * T8.T());
            img8x8[2] = T8 * (img[2].slice(i*8,j*8,8,8 ) * T8.T());

            quantize(img8x8[0]);
            quantize(img8x8[1]);
            quantize(img8x8[2]);

            img[0].slice(i*8,j*8,8,8) = img8x8[0];
            img[1].slice(i*8,j*8,8,8) = img8x8[1];
            img[2].slice(i*8,j*8,8,8) = img8x8[2];
        }

    for(int i=0; i<size/8; i++)
        for(int j=0; j<size/8; j++)
        {
            Matrix<8,8,double> img8x8[3];
            img8x8[0] = img[0].slice(i*8,j*8,8,8 );
            img8x8[1] = img[1].slice(i*8,j*8,8,8 );
            img8x8[2] = img[2].slice(i*8,j*8,8,8 );

            scale(img8x8[0]);
            scale(img8x8[1]);
            scale(img8x8[2]);

            img[0].slice(i*8,j*8,8,8) = T8.T() * (img8x8[0] * T8);
            img[1].slice(i*8,j*8,8,8) = T8.T() * (img8x8[1] * T8);
            img[2].slice(i*8,j*8,8,8) = T8.T() * (img8x8[2] * T8);
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

  glRasterPos2f(-1,-1);
  glDrawPixels(orgImg);

  glRasterPos2f(0,-1);
  glDrawPixels(tempImg);

  glutSwapBuffers();
}

int main(int argc, char** argv) {


    if(argc < 2)
    {
        printf("please specify an image file!\n");
        return 0;
    }

    CVD::img_load(tempImg,  argv[1] );
    CVD::img_load(orgImg_,  argv[1] );
    CVD::img_load(orgImg,   argv[1] );

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(2*orgImg.size().x, orgImg.size().y);
    glutCreateWindow("OpenGL glDrawPixels demo");


    glutDisplayFunc(display);
    //glutReshapeFunc(reshape);
    //glutMouseFunc(mouse_button);
    //glutMotionFunc(mouse_motion);
    //glutKeyboardFunc(keyboard);
    //glutIdleFunc(idle);

    glEnable(GL_DEPTH_TEST);
    //glClearColor(0.0, 0.0, 0.0, 1.0);
    //glPointSize(2);

    filter();
    glutMainLoop();
}
