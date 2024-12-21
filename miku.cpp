#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <cstdlib>
#include <cmath>


cv::Mat img;
//cv::Mat crosshair;

cv::Point cursorA, cursorB;
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
    //cv::putText(img, "boop", cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 1, cv::LINE_AA);
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

  cv::Mat buffer = img.clone();
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
  cv::drawMarker(buffer, cv::Point(x, y), cv::Scalar(0, 0, 255), cv::MARKER_CROSS);
  std::cout << "Click!" << x << "," << y << "; event=" << event << std::endl;
  cv::imshow("display", buffer);
}

//void loadResources(){
//   crosshair = cv::imread("resources/crosshair1.png", cv::IMREAD_COLOR);
// }


int main() {
  int value;
  img = cv::imread("./jetto.jpg", cv::IMREAD_COLOR);
  cv::namedWindow("display", cv::WINDOW_AUTOSIZE | cv::WINDOW_GUI_NORMAL);
  cv::setMouseCallback("display", mouseHandler, NULL);
  if(img.empty()) {
    std::cout << "Could not read the image " << std::endl;
    return 1;
  }
 
  cv::imshow("display", img);
  
  cv::waitKey(0);
  cv::destroyAllWindows();
  return 0;
}
