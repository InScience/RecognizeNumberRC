#include <algorithm>
#include <cstdio>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iterator>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "/home/pi/openalpr/src/tclap/CmdLine.h"
#include "/home/pi/openalpr/src/openalpr/support/filesystem.h"
#include "/home/pi/openalpr/src/openalpr/support/timing.h"
#include "/home/pi/openalpr/src/openalpr/support/platform.h"
#include "/home/pi/openalpr/src/openalpr/openalpr/src/video/videobuffer.h"
#include "/home/pi/openalpr/src/openalpr/motiondetector.h"
#include "/home/pi/openalpr/src/openalpr/alpr.h"
//#include "/home/pi/openalpr/src/bindings/go/openalpr/openalpr.go"

//using namespace alpr;
using namespace std;

int CompareText (vector<string> plates, string RecognizedText, float confidence){

int Result=0;


for (int i=0; i < plates.size(); i++){

if (plates[i]==RecognizedText){
Result = 1;
}
}

return Result;
}







int main()
{
int linecount=0;
string line;
std::vector<string> plates;
ifstream Platefile ("/home/pi/Desktop/Projects/alprtest/bin/Debug/Plates.txt");
if (Platefile.is_open())
{
 while (getline (Platefile,line))
 {
     linecount++;
 }
 Platefile.clear();
 Platefile.seekg(0, ios::beg);

//plates.resize(linecount);
int i =0;
 while(getline (Platefile, line)){
 plates.push_back(line);
 i++;
 }}
 Platefile.close();
cout << "Comparing against: " << endl;
for (int i=0; i < linecount; i++)
{
     cout << plates[i] << endl;
}

   // Initialize the library using United States style license plates.
  // You can use other countries/regions as well (for example: "eu", "au", or "kr")
  // alpr::Alpr openalpr("eu", "/path/to/openalpr.conf");
 alpr::Alpr openalpr("eu");
 // Optionally specify the top N possible plates to return (with confidences).  Default is 10
 openalpr.setTopN(20);

 // Optionally, provide the library with a region for pattern matching.  This improves accuracy by
 // comparing the plate text with the regional pattern.
 //openalpr.setDefaultRegion("md");

 // Make sure the library loaded before continuing.
 // For example, it could fail if the config/runtime_data is not found
 if (openalpr.isLoaded() == false)
 {
     std::cerr << "Error loading OpenALPR" << std::endl;
     return 1;
 }

 // Recognize an image file.  You could alternatively provide the image bytes in-memory.
 alpr::AlprResults results = openalpr.recognize("/home/pi/Desktop/Projects/LicensePlateRecognitorTest/bin/Debug/numeris3.jpg");
 int Compare;
 // Iterate through the results.  There may be multiple plates in an image,
 // and each plate return sthe top N candidates.
 for (int i = 0; i < results.plates.size(); i++)
 {
   alpr::AlprPlateResult plate = results.plates[i];
   std::cout << "plate" << i << ": " << plate.topNPlates.size() << " results" << std::endl;

     for (int k = 0; k < plate.topNPlates.size(); k++)
     {
       alpr::AlprPlate candidate = plate.topNPlates[k];
       std::cout << "    - " << candidate.characters << "\t confidence: " << candidate.overall_confidence;
       std::cout << "\t pattern_match: " << candidate.matches_template << std::endl;
       Compare = CompareText(plates, candidate.characters, candidate.overall_confidence);
       if (Compare = 1)
        {
          cout << "Vartai atidaryti" << endl;
          k = plate.topNPlates.size();
        }


     }
 }
    if (Compare = 0)
    cout << "Vartai uzdaryti" << endl;
    return 0;
}
