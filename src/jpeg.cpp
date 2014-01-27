
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>
#include <cvd/image_io.h>
#include <cvd/gl_helpers.h>
#include <cvd/image.h>
#include <TooN/TooN.h>

#include <iostream>
#include <sstream>
#include "math.h"

using namespace TooN;

CVD::Image<CVD::Rgb<CVD::byte> > tempImg;
//CVD::Image<CVD::Rgb<CVD::byte> > orgImg_;
CVD::Image<CVD::Rgb<CVD::byte> > orgImg;
CVD::Image<CVD::Rgb<CVD::byte> > cobmined2Draw;

int image_width;
int image_height;

int qual=40;

Matrix<8,8,double> T8;

Matrix<8,8,double> lumin_quantizers = Data (
            16, 11, 10, 16, 24, 40, 51, 61,
             12, 12, 14, 19, 26, 58, 60, 55,
             14, 13, 16, 24, 40, 57, 69, 56,
             14, 17, 22, 29, 51, 87, 80, 62,
             18, 22, 37, 56, 68, 109, 103, 77,
             24, 35, 55, 64, 81, 104, 113, 92,
             49, 64, 78, 87, 103, 121, 120, 101,
             72, 92, 95, 98, 112, 100, 103, 99);

Matrix<8,8,double> chrom_quantizers = Data (
            17, 18, 24, 47, 99, 99, 99, 99,
            18, 21, 6 , 66, 99, 99, 99, 99,
            24, 26, 56, 99, 99, 99, 99, 99,
            47, 66, 99, 99, 99, 99, 99, 99,
            99, 99, 99, 99, 99, 99, 99, 99,
            99, 99, 99, 99, 99, 99, 99, 99,
            99, 99, 99, 99, 99, 99, 99, 99,
            99, 99, 99, 99, 99, 99, 99, 99);

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

void quantize(Matrix<8,8,double> &m, bool lum)
{

    for(int i=0; i<8; i++)
        for(int j=0; j<8; j++)
        {
            double q = (lum)? lumin_quantizers[i][j] : chrom_quantizers[i][j];
            q += qual;
            m[i][j] = m[i][j]/q;
        }
}

void scale(Matrix<8,8,double> &m, bool lum)
{
    for(int i=0; i<8; i++)
        for(int j=0; j<8; j++)
        {
            double q = (lum)? lumin_quantizers[i][j] : chrom_quantizers[i][j];
            q += qual;
            m[i][j] = m[i][j] * q;
        }
}
void Encode(Matrix<Dynamic,Dynamic,int> &input,Matrix<Dynamic,Dynamic,int> &output, bool luminance)
{
    int w = input.num_cols();
    int h = input.num_rows();

    for(int i=0; i<h/8; i++)
        for(int j=0; j<w/8; j++)
        {
            Matrix<8,8,double> img8x8[3];
            img8x8[0] = T8 * (input.slice(i*8,j*8,8,8 ) * T8.T());
            quantize(img8x8[0], luminance);
            output.slice(i*8,j*8,8,8) = img8x8[0];
        }
}

void Decode(Matrix<Dynamic,Dynamic,int> &input,Matrix<Dynamic,Dynamic,int> &output, bool luminance)
{
    int w = input.num_cols();
    int h = input.num_rows();

    for(int i=0; i<h/8; i++)
        for(int j=0; j<w/8; j++)
        {
            Matrix<8,8,double> img8x8[3];
            img8x8[0] = input.slice(i*8,j*8,8,8 );
            scale(img8x8[0], luminance);
            output.slice(i*8,j*8,8,8) = T8.T() * (img8x8[0] * T8);
        }
}

void JpegEncoding()
{
    //int size = tempImg.size().x;
    int image_height_padded = ceil(image_height/8.0)*8;
    int image_width_padded  = ceil(image_width/8.0)*8;

    //printf("%d %d \n", image_height_padded, image_width_padded);
    Matrix<Dynamic,Dynamic,int> img_y(image_height_padded, image_width_padded);
    Matrix<Dynamic,Dynamic,int> img_u(image_height_padded, image_width_padded);
    Matrix<Dynamic,Dynamic,int> img_v(image_height_padded, image_width_padded);

    Matrix<Dynamic,Dynamic,int> img_y_encoded(image_height_padded, image_width_padded);
    Matrix<Dynamic,Dynamic,int> img_u_encoded(image_height_padded, image_width_padded);
    Matrix<Dynamic,Dynamic,int> img_v_encoded(image_height_padded, image_width_padded);

    //convert rgb to yuv
    for(int i=0; i< image_height_padded; i++)
        for(int j=0; j< image_width_padded; j++)
        {
            int jx = (j >= image_width) ? image_width-1 : j;
            int iy = (i >= image_height) ? image_height-1 : i;

            Vector<3> yuvv =  RGB2YUV(orgImg[iy][jx]);
            img_y[i][j] = yuvv[0];
            img_u[i][j] = yuvv[1];
            img_v[i][j] = yuvv[2];
        }


    // Start encoding
    Encode(img_y, img_y_encoded, true);
    Encode(img_u, img_u_encoded, false);
    Encode(img_v, img_v_encoded, false);


    // Start decoding
    Decode(img_y_encoded, img_y, true);
    Decode(img_u_encoded, img_u, false);
    Decode(img_v_encoded, img_v, false);

    tempImg.resize(CVD::ImageRef(image_width, image_height));
    //convert yuv to rgb
    for(int i=0; i< image_height; i++)
        for(int j=0; j< image_width; j++)
        {
            Vector<3> yuv;
            yuv[0] = img_y[i][j];
            yuv[1] = img_u[i][j];
            yuv[2] = img_v[i][j];

            CVD::Rgb<CVD::byte> rgb = YUV2RGB(yuv);
            tempImg[i][j] = rgb;
        }


    // preparing output image (combined high quality + jpeg image)
    cobmined2Draw.resize(CVD::ImageRef(2*image_width, image_height));
    for(int i=0; i< image_height; i++)
        for(int j=0; j< 2*image_width; j++)
        {
            if(j< image_width)
            {
                cobmined2Draw[i][j] = orgImg[image_height-i-1][j];
            }
            else
            {
                cobmined2Draw[i][j] = tempImg[image_height-i-1][j-image_width];
            }
        }
}

void display()
{
  glEnable(GL_TEXTURE_2D);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //glRasterPos2f(-1,-1);
  //glDrawPixels(orgImg);


  glRasterPos2f(-1,-1);
  CVD::glDrawPixels(cobmined2Draw);

  glutSwapBuffers();
}

bool update = true;
void keyboard_event(unsigned char key, int x, int y)
{
    if(update)
    {
        update = false;
        if(key == '=')
        {
            qual += 20;
            JpegEncoding();
            display();
        }
        else if(key == '-')
        {
            qual -= 20;
            JpegEncoding();
            display();
        }

        printf("Qual: %d\n", qual);
    }
}

void keyboard_event_Up(unsigned char key, int x, int y)
{
    update = true;
}

int main(int argc, char** argv) {


    if(argc < 2)
    {
        printf("please specify an image file!\n");
        return 0;
    }

    for(int i=0; i<8; i++)
        for(int j=0; j<8; j++)
        {
            if(i==0)
                T8[i][j] = 0.35355339059;
            else
                T8[i][j] = 0.5*cos((2*j+1)*i*3.14/16);
        }

    //CVD::img_load(tempImg,  argv[1] );
   // CVD::img_load(orgImg_,  argv[1] );
    CVD::img_load(orgImg,   argv[1] );

    image_width = orgImg.size().x;
    image_height = orgImg.size().y;

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(2*image_width, image_height);
    glutCreateWindow("OpenGL glDrawPixels demo");


    glutDisplayFunc(display);
    //glutReshapeFunc(reshape);
    //glutMouseFunc(mouse_button);
    //glutMotionFunc(mouse_motion);
    glutKeyboardFunc(keyboard_event);
    glutKeyboardUpFunc(keyboard_event_Up);

    //glutIdleFunc(idle);

    glEnable(GL_DEPTH_TEST);
    //glClearColor(0.0, 0.0, 0.0, 1.0);
    //glPointSize(2);

    JpegEncoding();
    glutMainLoop();
}
