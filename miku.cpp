#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <time.h>

cv::Mat img;
//cv::Mat crosshair;

cv::Point cursorA, cursorB, mouse;
enum CursorState {
  NONE = 0,
  HAS_A,
  HAS_B
};
enum CursorState cs = NONE;

std::string statusline = "empty status";


std::string pointToMeasResult(cv::Point &a, cv::Point &b){
  int dx = abs(a.x - b.x);
  int dy = abs(a.y - b.y);
  float distance = sqrt(pow(dx, 2) + pow(dy, 2));
  std::stringstream ss;
  //ss << "Δx: " << dx << " Δy: " << dy << " dist: " << distance;
  ss << "dx: " << dx << " dy: " << dy << " dist: " << distance;
  return ss.str();
}

void mouseHandler(int event, int x, int y, int z, void* userdata){
  if(event == 1){
    if(cs == NONE){
      cursorA = cv::Point(x, y);
      cs = HAS_A;
    } else if(cs == HAS_A){
      cursorB = cv::Point(x, y);
      cs = HAS_B;
    } else if(cs == HAS_B){
      cs = NONE;
    }
  }
  mouse = cv::Point(x, y);
  
}

//void loadResources(){
//   crosshair = cv::imread("resources/crosshair1.png", cv::IMREAD_COLOR);
// }

void drawUI(cv::Mat &buffer){

  if(cs == HAS_A || cs == HAS_B) {
    cv::drawMarker(buffer, cursorA, cv::Scalar(0, 0, 255), cv::MARKER_TRIANGLE_UP);
    cv::drawMarker(buffer, cursorA, cv::Scalar(0, 0, 255), cv::MARKER_CROSS);
    
  }
  if(cs == HAS_B) {
    cv::drawMarker(buffer, cursorB, cv::Scalar(0, 0, 255), cv::MARKER_TRIANGLE_DOWN);
    cv::drawMarker(buffer, cursorB, cv::Scalar(0, 0, 255), cv::MARKER_CROSS);

    //Draw summary
    
    cv::putText(buffer, pointToMeasResult(cursorA, cursorB), cv::Point(0, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
  }
  
  //Draw mouse
  cv::drawMarker(buffer, mouse, cv::Scalar(0, 0, 255), cv::MARKER_CROSS);

}




int main() {
  int value;

  cv::VideoCapture camera(0);
  cv::namedWindow("MIKU", cv::WINDOW_AUTOSIZE | cv::WINDOW_GUI_NORMAL);
  cv::setMouseCallback("MIKU", mouseHandler, NULL);
  camera.set(cv::CAP_PROP_FRAME_WIDTH, 800);
  camera.set(cv::CAP_PROP_FRAME_HEIGHT, 600);
  if (!camera.isOpened()) {
    std::cerr << "ERROR: Could not open camera" << std::endl;
    return 1;
  }
  while (1) {
    // capture the next frame from the webcam
    camera >> img;
    
    drawUI(img);
    cv::imshow("MIKU", img);
    int key = cv::waitKey(10);
    // wait (10ms) for esc key to be pressed to stop
    if (key == 27 || key == 'q')
      break;
    else if(key == 's'){
      std::stringstream ss;
      ss << "frame_" << time(NULL) << ".png";
      cv::imwrite(ss.str(), img);
    }
  }
  
  cv::destroyAllWindows();
  return 0;

  
 
  
}
