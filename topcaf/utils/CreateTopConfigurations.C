/* This root macro will create a TFile that contains the data for the topcaf/dataobjects/TopConfigurations dataobject. */
/* Formulae are taken from https://belle2.cc.kek.jp/~twiki/bin/view/Archive/Belle2note0026 version 1. */
/* Note that this only seems to work if compiled in ROOT interactive mode. 
i.e.
root [0] .x CreateTopConfigurations.C+
*/

#include <iostream>
#include <map>
#include <string>

#include "TFile.h"
#include "iTopUtils.h"   // Basic functions for number mapping are here.
#include "iTopTypedef.h" // Type definitions.


void CreateTopConfigurations(void){

  const unsigned int NumberOfPixels = 512;

  std::string outputFile = "../data/TopConfigurations.root";

  vector<TopElectronicConstructionName> TopModuleElectronicConstructions;

  // specific to CRT November 2014, see https://belle2.cc.kek.jp/~twiki/bin/view/Detector/TOP/CRTSetup2014#CRT_PMT_and_board_stack_configur
  TopElectronicConstructionName CRTHybridConstruction("CRTHybridConstruction1");
  TopModuleElectronicConstructions.push_back(CRTHybridConstruction);

  TopElectronicConstructionName June2013LepsConstruction("June2013LEPSConstruction1");
  TopModuleElectronicConstructions.push_back(June2013LepsConstruction);

  TopElectronicConstructionName ModuleC01Construction("ModuleC01");
  TopModuleElectronicConstructions.push_back(ModuleC01Construction);

  TopElectronicConstructionName ModuleC02Construction("ModuleC02");
  TopModuleElectronicConstructions.push_back(ModuleC02Construction);

  TopElectronicConstructionName ModuleC03Construction("ModuleC03");
  TopModuleElectronicConstructions.push_back(ModuleC03Construction);

  TopElectronicConstructionName ModuleC04Construction("ModuleC04");
  TopModuleElectronicConstructions.push_back(ModuleC04Construction);

  TopElectronicConstructionName ModuleC05Construction("ModuleC05");
  TopModuleElectronicConstructions.push_back(ModuleC05Construction);

  TopElectronicConstructionName ModuleC06Construction("ModuleC06");
  TopModuleElectronicConstructions.push_back(ModuleC06Construction);

  TopElectronicConstructionName ModuleC07Construction("ModuleC07");
  TopModuleElectronicConstructions.push_back(ModuleC07Construction);

  TopElectronicConstructionName ModuleC08Construction("ModuleC08");
  TopModuleElectronicConstructions.push_back(ModuleC08Construction);

  TopElectronicConstructionName ModuleC09Construction("ModuleC09");
  TopModuleElectronicConstructions.push_back(ModuleC09Construction);

  TopElectronicConstructionName ModuleC10Construction("ModuleC10");
  TopModuleElectronicConstructions.push_back(ModuleC10Construction);

  TopElectronicConstructionName ModuleC11Construction("ModuleC11");
  TopModuleElectronicConstructions.push_back(ModuleC11Construction);

  TopElectronicConstructionName ModuleC12Construction("ModuleC12");
  TopModuleElectronicConstructions.push_back(ModuleC12Construction);

  TopElectronicConstructionName ModuleC13Construction("ModuleC13");
  TopModuleElectronicConstructions.push_back(ModuleC13Construction);

  TopElectronicConstructionName ModuleC14Construction("ModuleC14");
  TopModuleElectronicConstructions.push_back(ModuleC14Construction);

  TopElectronicConstructionName ModuleC15Construction("ModuleC15");
  TopModuleElectronicConstructions.push_back(ModuleC15Construction);


  TopElectronicRefMap ElectronicModuletoScrod;
  TopElectronicRetMap ScrodtoElectronicModule;
  TopUnsignedMap   ScrodtoElectronicModuleNumber;
  for(vector<TopElectronicConstructionName>::const_iterator i = TopModuleElectronicConstructions.begin();
      i!=TopModuleElectronicConstructions.end();
      ++i){

    if( (*i) == CRTHybridConstruction ){
      ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 67;
      ElectronicModuletoScrod[TopElectronicModule(*i,1)] = 65;
      ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 66;
      ElectronicModuletoScrod[TopElectronicModule(*i,3)] = 59;
    }

    if( (*i) == June2013LepsConstruction ){
      ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 36;
      ElectronicModuletoScrod[TopElectronicModule(*i,1)] = 35;
      ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 32;
      ElectronicModuletoScrod[TopElectronicModule(*i,3)] = 37;
    }

    //if ( (*i) == ModuleC01Construction ) {
    //ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 14;
    //ElectronicModuletoScrod[TopElectronicModule(*i,1)] = 15;
    //ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 16;
    //ElectronicModuletoScrod[TopElectronicModule(*i,3)] = 13;
    //}
    if ( (*i) == ModuleC01Construction ) {
	ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 41;
      	ElectronicModuletoScrod[TopElectronicModule(*i,1)] = 103;
      	ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 74;
      	ElectronicModuletoScrod[TopElectronicModule(*i,3)] = 88;
    }
    if ( (*i) == ModuleC02Construction ) {
	ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 32;
      	ElectronicModuletoScrod[TopElectronicModule(*i,1)] = 29;
      	ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 96;
      	ElectronicModuletoScrod[TopElectronicModule(*i,3)] = 31;
    }
    /*
    if ( (*i) == ModuleC02Construction ) {
	ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 32;
      	ElectronicModuletoScrod[TopElectronicModule(*i,1)] = 29;
      	ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 31;
      	ElectronicModuletoScrod[TopElectronicModule(*i,3)] = 28;
    }
    */
    if ( (*i) == ModuleC03Construction ) {
	ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 42;
      	ElectronicModuletoScrod[TopElectronicModule(*i,1)] = 26;
      	ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 34;
      	ElectronicModuletoScrod[TopElectronicModule(*i,3)] = 25;
    }
    if ( (*i) == ModuleC04Construction ) {
	ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 45;
      	ElectronicModuletoScrod[TopElectronicModule(*i,1)] = 50;
      	ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 48;
      	ElectronicModuletoScrod[TopElectronicModule(*i,3)] = 46;
    }
    if ( (*i) == ModuleC05Construction ) {
	ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 72;
      	ElectronicModuletoScrod[TopElectronicModule(*i,1)] = 95;
      	ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 70;
      	ElectronicModuletoScrod[TopElectronicModule(*i,3)] = 69;
    }
    if ( (*i) == ModuleC06Construction ) {
	ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 62;
      	ElectronicModuletoScrod[TopElectronicModule(*i,1)] = 54;
      	ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 63;
      	ElectronicModuletoScrod[TopElectronicModule(*i,3)] = 60;
    }
    if ( (*i) == ModuleC07Construction ) {
	ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 38;
      	ElectronicModuletoScrod[TopElectronicModule(*i,1)] = 24;
      	ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 39;
      	ElectronicModuletoScrod[TopElectronicModule(*i,3)] = 97;
    }
    if ( (*i) == ModuleC08Construction ) {
	ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 64;
      	ElectronicModuletoScrod[TopElectronicModule(*i,1)] = 65;
      	ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 66;
      	ElectronicModuletoScrod[TopElectronicModule(*i,3)] = 67;
    }
    if ( (*i) == ModuleC09Construction ) {
	ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 51;
      	ElectronicModuletoScrod[TopElectronicModule(*i,1)] = 53;
//      ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 56;
      	ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 99;
      	ElectronicModuletoScrod[TopElectronicModule(*i,3)] = 59;
    }
    //if ( (*i) == ModuleC10Construction ) {
    //ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 35;
    //ElectronicModuletoScrod[TopElectronicModule(*i,1)] = 87;
    //ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 57;
    //ElectronicModuletoScrod[TopElectronicModule(*i,3)] = 68;
    //}
    if ( (*i) == ModuleC10Construction ) {
	ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 35;
      	ElectronicModuletoScrod[TopElectronicModule(*i,1)] = 23;
      	ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 44;
      	ElectronicModuletoScrod[TopElectronicModule(*i,3)] = 102;
    }
    if ( (*i) == ModuleC11Construction ) {
	ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 79;
      	ElectronicModuletoScrod[TopElectronicModule(*i,1)] = 80;
      	ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 82;
      	ElectronicModuletoScrod[TopElectronicModule(*i,3)] = 94;
    }
    if ( (*i) == ModuleC12Construction ) {
	ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 75;
      	ElectronicModuletoScrod[TopElectronicModule(*i,1)] = 76;
      	ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 77;
      	ElectronicModuletoScrod[TopElectronicModule(*i,3)] = 81;
    }
    if ( (*i) == ModuleC13Construction ) {
//	ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 87;
	ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 83;
      	ElectronicModuletoScrod[TopElectronicModule(*i,1)] = 98;
      	ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 84;
      	ElectronicModuletoScrod[TopElectronicModule(*i,3)] = 36;
    }
    if ( (*i) == ModuleC14Construction ) {
	ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 37;
      	ElectronicModuletoScrod[TopElectronicModule(*i,1)] = 55;
//      ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 57;
      	ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 21;
      	ElectronicModuletoScrod[TopElectronicModule(*i,3)] = 71;
    }
    if ( (*i) == ModuleC15Construction ) {
	ElectronicModuletoScrod[TopElectronicModule(*i,0)] = 85;
      	ElectronicModuletoScrod[TopElectronicModule(*i,1)] = 90;
      	ElectronicModuletoScrod[TopElectronicModule(*i,2)] = 91;
      	ElectronicModuletoScrod[TopElectronicModule(*i,3)] = 92;
    }

    
    ScrodtoElectronicModuleNumber[ElectronicModuletoScrod[TopElectronicModule(*i,0)]] = 0;
    ScrodtoElectronicModuleNumber[ElectronicModuletoScrod[TopElectronicModule(*i,1)]] = 1;
    ScrodtoElectronicModuleNumber[ElectronicModuletoScrod[TopElectronicModule(*i,2)]] = 2;
    ScrodtoElectronicModuleNumber[ElectronicModuletoScrod[TopElectronicModule(*i,3)]] = 3;

    ScrodtoElectronicModule[ElectronicModuletoScrod[TopElectronicModule(*i,0)]] = TopElectronicModule(*i,0);
    ScrodtoElectronicModule[ElectronicModuletoScrod[TopElectronicModule(*i,1)]] = TopElectronicModule(*i,1);
    ScrodtoElectronicModule[ElectronicModuletoScrod[TopElectronicModule(*i,2)]] = TopElectronicModule(*i,2);
    ScrodtoElectronicModule[ElectronicModuletoScrod[TopElectronicModule(*i,3)]] = TopElectronicModule(*i,3);
    
  }


  TopPixelRefMap PixeltoRow;
  TopPixelRefMap PixeltoColumn;
  TopPixelRefMap PixeltoPMT;
  TopPixelRefMap PixeltoPMTChannel;
  TopPixelRefMap PixeltoScrod;
  TopPixelRefMap PixeltoAsicRow;
  TopPixelRefMap PixeltoAsicColumn;
  TopPixelRefMap PixeltoAsicChannel;

  TopPixelRetMap HardwareIDtoPixel;

  TopPixelRefElectronicRetMap PixeltoElectronicModule;

  /* Map the pixel to various items. */
  for(vector<std::string>::const_iterator i = TopModuleElectronicConstructions.begin();
      i!=TopModuleElectronicConstructions.end();
      ++i){
    for( TopPixelNumber c=1 ; c <= NumberOfPixels ; c++ ){
      TopPixel myPixel(TopElectronicModule(*i,c));

      PixeltoRow[myPixel] = pixelNumber_to_pixelRow(c);
      PixeltoColumn[myPixel] = pixelNumber_to_pixelColumn(c);
      PixeltoPMT[myPixel] = pixelNumber_to_PMTNumber(c);
      PixeltoPMTChannel[myPixel] = pixelNumber_to_channelNumber(c);
      PixeltoElectronicModule[myPixel] = TopElectronicModule(*i,pixel_to_electronicsModuleNumber(c));
      PixeltoAsicRow[myPixel] = pixel_to_asicRow(c);
      PixeltoAsicColumn[myPixel] = pixel_to_asicColumn(c);
      PixeltoAsicChannel[myPixel] = pixel_to_asicChannel(c);      
      PixeltoScrod[myPixel] = ElectronicModuletoScrod[PixeltoElectronicModule[myPixel]];
      
      unsigned int HardwareID = PixeltoScrod[myPixel] * 1E8
	+ PixeltoAsicRow[myPixel] * 1E6
	+ PixeltoAsicColumn[myPixel] * 1E4
	+ PixeltoAsicChannel[myPixel] * 1E2;
      


      HardwareIDtoPixel[HardwareID] = myPixel;
      
    } 
  }

  TopParameter TDCUnit_ns(.001,"ns"); // lets use picoseconds for now.


  TFile f(outputFile.c_str(), "recreate");
  f.WriteObject(&HardwareIDtoPixel,"HardwareIDtoPixel");
  f.WriteObject(&ScrodtoElectronicModule,"ScrodtoElectronicModule");
  f.WriteObject(&ScrodtoElectronicModuleNumber,"ScrodtoElectronicModuleNumber");
  f.WriteObject(&ElectronicModuletoScrod,"ElectronicModuletoScrod");
  f.WriteObject(&PixeltoRow,"PixeltoRow");
  f.WriteObject(&PixeltoColumn,"PixeltoColumn");
  f.WriteObject(&PixeltoPMT,"PixeltoPMT");
  f.WriteObject(&PixeltoPMTChannel,"PixeltoPMTChannel");
  f.WriteObject(&PixeltoAsicRow,"PixeltoAsicRow");
  f.WriteObject(&PixeltoAsicColumn,"PixeltoAsicColumn");
  f.WriteObject(&PixeltoAsicChannel,"PixeltoAsicChannel");
  f.WriteObject(&PixeltoScrod,"PixeltoScrod");
  f.WriteObject(&PixeltoElectronicModule,"PixeltoElectronicModule");
  f.WriteObject(&TopModuleElectronicConstructions,"TopModuleElectronicConstructions");
  f.WriteObject(&TDCUnit_ns,"TDCUnit_ns");
  

  f.Close();

  unsigned int testID = 3500000100;
  std::cout<<"hardware id["<<testID<<"]: "<<HardwareIDtoPixel[testID].second<<std::endl;


}

