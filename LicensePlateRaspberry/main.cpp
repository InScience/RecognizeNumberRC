#include <iostream>
#include <fstream>
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/StreamCopier.h"


#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Object.h"
#include <Poco/Path.h>
#include <Poco/URI.h>

#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/StringPartSource.h"

#include "Poco/Base64Encoder.h"
#include <sstream>
#include <iomanip>
#include <string>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>



int LCDAddr = 0x27;
int BLEN = 1;
int fd;

void write_word(int data)
{
    int temp = data;
    if ( BLEN == 1 )
        temp |= 0x08;
    else
        temp &= 0xF7;
    wiringPiI2CWrite(fd, temp);
}
void send_command(int comm)
{
    int buf;
// Send bit7-4 firstly
    buf = comm & 0xF0;
    buf |= 0x04; // RS = 0, RW = 0, EN = 1
    write_word(buf);
    delay(2);
    buf &= 0xFB; // Make EN = 0
    write_word(buf);
// Send bit3-0 secondly
    buf = (comm & 0x0F) << 4;
    buf |= 0x04; // RS = 0, RW = 0, EN = 1
    write_word(buf);
    delay(2);
    buf &= 0xFB; // Make EN = 0
    write_word(buf);
}
void send_data(int data)
{
    int buf;
// Send bit7-4 firstly
    buf = data & 0xF0;
    buf |= 0x05; // RS = 1, RW = 0, EN = 1
    write_word(buf);
    delay(2);
    buf &= 0xFB; // Make EN = 0
    write_word(buf);
// Send bit3-0 secondly
    buf = (data & 0x0F) << 4;
    buf |= 0x05; // RS = 1, RW = 0, EN = 1
    write_word(buf);
    delay(2);
    buf &= 0xFB; // Make EN = 0
    write_word(buf);
}
void init()
{
    send_command(0x33); // Must initialize to 8-line mode at first
    delay(5);
    send_command(0x32); // Then initialize to 4-line mode
    delay(5);
    send_command(0x28); // 2 Lines & 5*7 dots
    delay(5);
    send_command(0x0C); // Enable display without cursor
    delay(5);
    send_command(0x01); // Clear Screen
    wiringPiI2CWrite(fd, 0x08);
}
void clear()
{
    send_command(0x01); //clear Screen
}
void write(int x, int y, char data[])
{
    int addr, i;
    int tmp;
    if (x < 0) x = 0;
    if (x > 15) x = 15;
    if (y < 0) y = 0;
    if (y > 1) y = 1;
// Move cursor
    addr = 0x80 + 0x40 * y + x;
    send_command(addr);

    tmp = strlen(data);
    for (i = 0; i < tmp; i++)
    {
        send_data(data[i]);
    }
}


void takePicture()
{

CvCapture* capture = cvCaptureFromCAM( CV_CAP_ANY );
cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 352);
cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 288);

 // Get one frame
IplImage* frame;

for (int i = 0; i < 25; i++)
{
frame = cvQueryFrame( capture );
}


printf( "Image captured \n" );
//IplImage* RGB_frame = frame;
//cvCvtColor(frame,RGB_frame,CV_YCrCb2BGR);
//cvWaitKey(1000);
cvSaveImage("test.jpg" ,frame);
//cvSaveImage("cam.jpg" ,RGB_frame);

printf( "Image Saved \n" );

//cvWaitKey(10);

// Release the capture device housekeeping
cvReleaseCapture( &capture );
//cvDestroyWindow( "mywindow" );

}



using namespace std;

string Recognize()
{


 std::ifstream infile ("test.jpg",std::ios::binary | std::ios::in | std::ios::ate);
  std::ofstream outfile ("new.txt",std::ofstream::binary);
  std::string sb;
  std::string s64;

  // get size of file
  infile.seekg (0,infile.end);
  long size = infile.tellg();
  infile.seekg (0);

  // allocate memory for file content
  char* buffer = new char[size];

  // read content of infile
  infile.read (buffer,size);

  sb.reserve(infile.tellg());
  infile.seekg(0,std::ios::beg);
  while (infile){

  sb+=(char)(infile.get());

  }

 stringstream ss;

 Poco::Base64Encoder b64enc(ss);
 b64enc << sb;


 s64 = ss.str();
 Poco::Net::HTTPClientSession s("192.168.43.140",15570);
 Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, "/RestServiceImpl.svc/UploadPic");
 request.setContentType("application/json");

 //std::string body = "{\"Pavarde\":\"" + Pavarde + "\", \"Vardas\":\"Jonas\", \"id\":\"1\"}";

 std::string body;

 body = "{\"PicByt\":\"" + s64 +"\"}";
 body.erase(std::remove(body.begin(), body.end(), '\n'), body.end());
 body.erase(std::remove(body.begin(), body.end(), '\r'), body.end());
 body.erase(std::remove(body.begin(), body.end(), '\t'), body.end());
 //std::string body = "{\"PicByt\":\"fjhvjg\"}";

 request.setContentLength(body.length());

 s.sendRequest(request) << body;


 Poco::Net::HTTPResponse response;
 std::istream& rs = s.receiveResponse(response);
 std::ostringstream os;

 Poco::StreamCopier::copyStream(rs,os);
 std::string str = os.str();

return str;
}

void RecognizeMonitor(string numberPlate){

fd = wiringPiI2CSetup(LCDAddr);
init();

std::cout<< "JSON string: " << numberPlate << endl;

char tab2[1024];
strncpy(tab2, numberPlate.c_str(), sizeof(tab2));
tab2[sizeof(tab2) - 1] = 0;

 write(0, 0, "Atpazintas:");
 write(0, 1, tab2);
 delay(10000);
 clear();

}

int main()
{

string str;
int choice;
bool app = true;
while(app!=false){
cout << "Pi klientas" << endl;
cout << "Pasirinkti:" << endl;
cout << "1 - Fotografuoti ir atpazinti" << endl;
cout << "2 - Uzdaryti programa" << endl;
cin >> choice;


switch(choice)
{
    case 1:
     takePicture();
     str = Recognize();
     RecognizeMonitor(str);
    break;
    case 2:

    app = false;
    break;
    default:
    cout << "Pasirinkimas neteisingas, rinkites dar karta" << endl;

    break;
}
}



    return 0;
}
