#ifndef iTopTypedef_H
#define iTopTypedef_H

#include <string>
#include <utility>
#include <map>
#include <vector>




//typedef unsigned int packet_word_t;
typedef std::string TopElectronicConstructionName;
typedef unsigned int TopPixelNumber;
typedef unsigned int TopElectronicModuleNumber;
typedef double TopParameterValue;
typedef std::string TopParameterUnit;

// Pairs
typedef std::pair<TopElectronicConstructionName, TopElectronicModuleNumber> TopElectronicModule;
typedef std::pair<TopElectronicConstructionName, TopPixelNumber> TopPixel;
typedef std::pair<TopParameterValue, TopParameterUnit> TopParameter;

// Maps
typedef std::map<TopPixel, unsigned int> TopPixelRefMap;
typedef std::map<unsigned int, TopPixel> TopPixelRetMap;
typedef std::map<TopPixel, TopElectronicModule> TopPixelRefElectronicRetMap;
typedef std::map<unsigned int, unsigned int> TopUnsignedMap;
typedef std::map<TopElectronicModule, unsigned int> TopElectronicRefMap;
typedef std::map<unsigned int, TopElectronicModule> TopElectronicRetMap;





#ifdef __MAKECINT__

#pragma link C++ class TopElectronicModule+;
#pragma link C++ class TopPixel+;

//#pragma link C++ class std::pair<std::string, unsigned int>;

#pragma link C++ class TopUnsignedMap+;
#pragma link C++ class std::pair<unsigned int, unsigned int>+;

#pragma link C++ class TopPixelRefMap+;
#pragma link C++ class std::pair<TopPixel,unsigned int>+;

#pragma link C++ class TopPixelRetMap+;
#pragma link C++ class std::pair<unsigned int, TopPixel>+;

#pragma link C++ class TopPixelRefElectronicRetMap+;
#pragma link C++ class std::pair<TopPixel, TopElectronicModule>+;

#pragma link C++ class TopElectronicRefMap+;
#pragma link C++ class std::pair<TopElectronicModule, unsigned int>+;

#pragma link C++ class TopElectronicRetMap+;
#pragma link C++ class std::pair<unsigned int, TopElectronicRetMap>+;

#pragma link C++ class TopParameter+;

#endif


#endif
