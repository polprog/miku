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
enum CalState {
  CAL_RUN = 0,   // normal state
  CAL_CURSOR, // cal mode - mark with cursors
  CAL_ASKDIST, // cal mode - ask for distance
  CAL_UNCAL // uncalibrated, show flag
};
enum CalState cal = CAL_UNCAL;
bool user_entering = false;
std::string userentry_q;

std::string statusline = "Ready";
float umperpx = 0.0;


float pointDistance(cv::Point &a, cv::Point &b){
  int dx = abs(a.x - b.x);
  int dy = abs(a.y - b.y);
  return sqrt(pow(dx, 2) + pow(dy, 2));
}

std::string pointToMeasResult(cv::Point &a, cv::Point &b, float scale){
  int dx = abs(a.x - b.x);
  int dy = abs(a.y - b.y);
  float distance = sqrt(pow(dx, 2) + pow(dy, 2));
  std::stringstream ss;
  if(scale != 0.0){
    ss.setf(std::ios::fixed);
    ss.precision(3);
    ss << "dx: " << dx*scale<< "um  dy: " << dy*scale << "um  dist: " << distance*scale << "um";
  } else{
    ss << "dx: " << dx << "px  dy: " << dy << "px  dist: " << distance << "px";
  }
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
      //statusline = pointToMeasResult(cursorA, cursorB, umperpx);

    } else if(cs == HAS_B){
      cs = NONE;
    }
  }
  mouse = cv::Point(x, y);
  
}

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
  int status_timeout = 0;
  int key;
  cv::Mat buffer(PREVIEW_HEIGHT, VIDEO_WIDTH, CV_8UC(3), cv::Scalar(0, 0, 0));
  cv::VideoCapture camera(2);
  cv::namedWindow("MIKU", cv::WINDOW_AUTOSIZE | cv::WINDOW_GUI_NORMAL);

  std::string userentry;
  
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
    key = cv::waitKey(10);


    std::cout << "stt=" << status_timeout << std::endl;
    std::cout << "fps=" << camera.get(cv::CAP_PROP_FPS) <<std::endl;
    //Generate the status line text
    if(status_timeout > 0) status_timeout--;
    if(status_timeout <= 0) {
      std::stringstream ss;
      ss << "MIKU " << getTimeString();
      if(cal == CAL_UNCAL) ss << " UNCAL";
      else ss << "      ";
      if(cs == HAS_B) ss << "  " << pointToMeasResult(cursorA, cursorB, umperpx);
      statusline = ss.str();
    }
    

    // User clicked the second cursor in cal mode
    if(cal == CAL_CURSOR && cs == HAS_B) {
      cal = CAL_ASKDIST;
      user_entering = true;
      status_timeout = -1; // do not hide the temporary status line
      userentry_q = "Dist [um]?";
      std::cout << "user entry mode" << std::endl;
    }

    //User entered the cal value
    if(cal == CAL_ASKDIST && !user_entering) {
      umperpx = atof(userentry.c_str())/pointDistance(cursorA, cursorB);
      userentry = "";
      cal = CAL_RUN;
      std::cout << "we have " << umperpx << "um/px" << std::endl;
    }


    
    // There is no text field in openCV, we have to do this the old way...
    if(user_entering){
      std::cout << "ue mode" << std::endl;
      if(key >= 32 and key <= 127){
	userentry += key;
      } else if(key == 8) { //backspace
	if(userentry.size() > 0) userentry.resize(userentry.size() - 1);
      } else if(key == 13){ //enter/ret
	user_entering = false;
	std::cout << "ue mode exit!" << std::endl;
      } 
      //XXX: add backspace support
      statusline = userentry_q + " " + userentry;
      std::cout << "key=" << key << std::endl;
    }
    
    
    drawUI(buffer);
    cv::imshow("MIKU", buffer);

    if(user_entering) continue;
    
    if (key == 27 || key == 'q')
      break;
    else if(key == 's'){
      std::stringstream ss;
      ss << "frame_" << time(NULL) << ".png";
      cv::imwrite(ss.str(), buffer);
      statusline = "WROTE " + ss.str();
      //status_timeout = camera.get(cv::CAP_PROP_FPS)/2; // show the message only for 1/2 sec
      status_timeout = 10;
    } else if(key == 'c'){
      statusline = "Mark distance";
      status_timeout = camera.get(cv::CAP_PROP_FPS)*2;
      cal = CAL_CURSOR;
    }
    

    if(cv::getWindowProperty("MIKU", cv::WND_PROP_VISIBLE) < 1) break;
  }
  
  cv::destroyAllWindows();
  return 0;

  
 
  
}
