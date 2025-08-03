#include "camera.hpp"
#include "face.hpp"
#include "faceDB.hpp"

int main(){
    // Mat frame;
    // Camera caper(VideoCapture(0));
    // faceDetect face;
    // while (true) {
    //     face.Get_ROI_face(caper.getCameraFrame(frame));
    //     if (waitKey(1) == 27) break;  // °´ESC¼üÍË³ö
    // }
    faceDB db("facedata.db");  // Create a database object
    db.openDB();  // Open the database
    db.createTable();  // Create a table in the database
    db.closeDB();  // Close the database
}