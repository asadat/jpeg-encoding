#include <cvd/videosource.h>
#include <cvd/videodisplay.h>
#include <cvd/gl_helpers.h>

using namespace CVD;

int main()
{
    VideoBuffer<Rgb<byte> > * video_buffer = open_video_source<Rgb<byte> >("drop.avi");
    //VideoDisplay disp(video_buffer->size());

//    while(1)
//    {
//        VideoFrame<Rgb<byte> > *frame = video_buffer->get_frame();
//        glDrawPixels(*frame);
//        video_buffer->put_frame(frame);
//    }
}
