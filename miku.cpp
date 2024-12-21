#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <time.h>

#define VIDEO_WIDTH   800
#define VIDEO_HEIGHT  600
#define STATUS_HEIGHT 40
#define PREVIEW_HEIGHT (VIDEO_HEIGHT + STATUS_HEIGHT)

cv::Mat img;
//cv::Mat crosshair;

cv::Point cursorA, cursorB, mouse;
enum CursorState {
  NONE = 0,
  HAS_A,
  HAS_B
};
enum CursorState cs = NONE;

std::string statusline = "Ready";


std::string pointToMeasResult(cv::Point &a, cv::Point &b){
  int dx = abs(a.x - b.x);
  int dy = abs(a.y - b.y);
  float distance = sqrt(pow(dx, 2) + pow(dy, 2));
  std::stringstream ss;
   
  ss << "dx: " << dx << "px  dy: " << dy << "px  dist: " << distance << "px";
  return ss.str();
}

std::string getTimeString(){
  char timeString[std::size("yy-bbb-DD hh:mm:ss")];
  time_t rawtime;
  struct tm *info;
  time( &rawtime );
  info = localtime( &rawtime );
  strftime(timeString,std::size(timeString),"%y-%b-%d %H:%M:%S", info);
  std::stringstream ss;
  ss << timeString;
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
      statusline = pointToMeasResult(cursorA, cursorB);

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

  }
  
  //Draw mouse
  cv::drawMarker(buffer, mouse, cv::Scalar(0, 0, 255), cv::MARKER_CROSS);

  //Draw status line
  cv::putText(buffer, statusline , cv::Point(0, PREVIEW_HEIGHT-10), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);

}




int main() {
  int value;
  int i = 0;
  cv::Mat buffer(PREVIEW_HEIGHT, VIDEO_WIDTH, CV_8UC(3), cv::Scalar(0, 0, 0));
  cv::VideoCapture camera(-1);
  cv::namedWindow("MIKU", cv::WINDOW_AUTOSIZE | cv::WINDOW_GUI_NORMAL);
  
  
  cv::setMouseCallback("MIKU", mouseHandler, NULL);
  camera.set(cv::CAP_PROP_FRAME_WIDTH, VIDEO_WIDTH);
  camera.set(cv::CAP_PROP_FRAME_HEIGHT, VIDEO_HEIGHT);
  if (!camera.isOpened()) {
    std::cerr << "ERROR: Could not open camera" << std::endl;
    return 1;
  }
  while (1) {
    // capture the next frame from the webcam
    camera >> img;
    cv::Rect to(cv::Point(0, 0), img.size());
    buffer = 0;
    img.copyTo(buffer(to));


    
    //Generate the status line text
    if(i > 0) i--;
    if(i == 0) {
      std::stringstream ss;
      ss << "MIKU " << getTimeString();
      if(cs == HAS_B) ss << "  " << pointToMeasResult(cursorA, cursorB);
      statusline = ss.str();
    }
    
    drawUI(buffer);
    cv::imshow("MIKU", buffer);
    int key = cv::waitKey(10);
    // wait (10ms) for esc key to be pressed to stop
    if (key == 27 || key == 'q')
      break;
    else if(key == 's'){
      std::stringstream ss;
      ss << "frame_" << time(NULL) << ".png";
      cv::imwrite(ss.str(), buffer);
      statusline = "WROTE " + ss.str();
      i = camera.get(cv::CAP_PROP_FPS)/2; // show the message only for 1/2 sec
    }
    

    if(cv::getWindowProperty("MIKU", cv::WND_PROP_VISIBLE) < 1) break;
  }
  
  cv::destroyAllWindows();
  return 0;

  
 
  
}
