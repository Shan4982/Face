#include "camera.hpp"
#include "face.hpp"

int main(){
    Mat frame;
    Camera caper(VideoCapture(0));
    faceDetect face;
    while (true) {
        face.Get_ROI_face(caper.getCameraFrame(frame));
        if (waitKey(1) == 27) break;  // °´ESC¼üÍË³ö
    }
}