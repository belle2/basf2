#ifdef __MAKECINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::Packet+;
#pragma link C++ class Belle2::EventHeaderPacket+;
#pragma link C++ class Belle2::EventWaveformPacket+;
#pragma link C++ class Belle2::topFileMetaData+;
#pragma link C++ class Belle2::CamacData+;
#pragma link C++ class Belle2::TopConfigurations+;

#pragma link C++ class TopElectronicModule+;
#pragma link C++ class TopPixel+;

#pragma link C++ class std::pair<std::string, unsigned int>+;

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
