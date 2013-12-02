#ifndef __CINT__
#include "trg/cdc/Fitter3DUtility.h"
#include "trg/cdc/Hough3DUtility.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#endif



using namespace std;

Hough3DFinder::Hough3DFinder(void){
  m_mode = 2;
  m_Trg_PI = 3.141592653589793;
  m_inputFileName = "GeoFinder.input";
}

Hough3DFinder::Hough3DFinder(int mode){
  m_mode = mode;
}

Hough3DFinder::~Hough3DFinder(void){
  destruct();
}

void Hough3DFinder::setMode(int mode){
  m_mode = mode;
}

int Hough3DFinder::getMode(void){
  return m_mode;
}

void Hough3DFinder::initialize(TVectorD &geometryVariables, vector<float > &initVariables){

  m_findRhoMax = -9999;
  m_findRhoIntMax = -9999;
  m_findPhi0Max = -9999;
  m_findPhi0IntMax = -9999;
  m_findArcCosMax = -9999;
  m_findArcCosIntMax = -9999;
  m_findPhiZMax = -9999;
  m_findPhiZIntMax = -9999;
  m_findRhoMin = 9999;
  m_findRhoIntMin = 9999;
  m_findPhi0Min = 9999;
  m_findPhi0IntMin = 9999;
  m_findArcCosMin = 9999;
  m_findArcCosIntMin = 9999;
  m_findPhiZMin = 9999;
  m_findPhiZIntMin = 9999;

  for(int iLayer=0; iLayer<4; iLayer++){
    m_bestTS[iLayer] = 999;
    m_bestTSIndex[iLayer] = 999;
  }
  for(int i=0; i<4; i++){
    m_rr[i] = geometryVariables[i];
    m_anglest[i] = geometryVariables[i+4];
    m_ztostraw[i] = geometryVariables[i+8];
    m_nWires[i] = (int)geometryVariables[i+12];
  }
  switch (m_mode){
    case 0: 
      break;
    case 1: 
      initVersion1(initVariables);
      break;
    case 2:
      initVersion2(initVariables);
      break;
    case 3:
      initVersion3(initVariables);
      break;
    default:
      cout<<"[Error] 3DFinder mode is not correct. Current mode is "<<m_mode<<"."<<endl;
      break;
  }
}

void Hough3DFinder::destruct(void){
  switch (m_mode){
    case 1: 
      destVersion1();
      break;
    case 2: 
      destVersion2();
      break;
    case 3: 
      destVersion3();
      break;
    default:
      break;
  }
}

void Hough3DFinder::runFinder(std::vector<double> &trackVariables, vector<vector<double> > &stTSs){

  // Clear best TS
  for(int iLayer=0; iLayer<4; iLayer++){
    m_bestTS[iLayer] = 999;
    m_bestTSIndex[iLayer] = 999;
  }

  // Set 2D parameters
  int charge = (int)trackVariables[0];
  double rho = trackVariables[1];
  double fitPhi0 = trackVariables[2];

  // Change TS candidates to arcS and z plane.
  vector<double > tsArcS;
  vector<vector<double> > tsZ;
  for(unsigned i=0; i<4; i++){
    tsArcS.push_back(calS(rho,m_rr[i]));
    tsZ.push_back(vector<double>());
    for(unsigned j=0; j<stTSs[i].size(); j++){
      //fitPhi0 = mcPhi0;
      tsZ[i].push_back(calZ(charge, m_anglest[i], m_ztostraw[i], m_rr[i], stTSs[i][j], rho, fitPhi0));
    }
  }

  switch (m_mode){
    // Hough 3D finder
    case 1: 
      runFinderVersion1(trackVariables, stTSs, tsArcS, tsZ);
      break;
    // Geo Finder
    case 2:
      runFinderVersion2(trackVariables, stTSs);
      break;
    // FPGA Geo Finder (For LUT generator. Doesn't use LUTs)
    case 3:
      runFinderVersion3(trackVariables, stTSs);
      break;
    default:
      break;
  }

}

void Hough3DFinder::initVersion1(vector<float > & initVariables){
  // Hough Variables.
  m_cotStart = (int)initVariables[0];
  m_cotEnd = (int)initVariables[1];
  m_z0Start = (int)initVariables[2];
  m_z0End = (int)initVariables[3];
  // Must be odd
  //m_nCotSteps = 101;
  //m_nZ0Steps = 501;
  m_nCotSteps = (int)initVariables[4];
  m_nZ0Steps = (int)initVariables[5];
  m_cotStepSize = m_cotEnd/((m_nCotSteps-1)/2);
  m_z0StepSize = m_z0End/((m_nZ0Steps-1)/2);
  // HoughMesh
  m_houghMeshLayerDiff = new float**[m_nCotSteps];
  m_houghMeshLayer = new bool**[m_nCotSteps];
  m_houghMesh = new int*[m_nCotSteps];
  m_houghMeshDiff = new float*[m_nCotSteps];
  for(int i=0; i<m_nCotSteps; i++){
    m_houghMeshLayerDiff[i] = new float*[m_nZ0Steps];
    m_houghMeshLayer[i] = new bool*[m_nZ0Steps];
    m_houghMesh[i] = new int[m_nZ0Steps];
    m_houghMeshDiff[i] = new float[m_nZ0Steps];
    for(int j=0; j<m_nZ0Steps; j++){
      m_houghMeshLayerDiff[i][j] = new float[4];
      m_houghMeshLayer[i][j] = new bool[4];
    }
  }
}

void Hough3DFinder::initVersion2(vector<float > & initVariables){
  if(1==2) cout<<initVariables.size()<<endl; // Removes warning when compiling

  // index values of candidates.
  m_geoCandidatesIndex = new vector<vector<int > >;
  for(int iLayer=0; iLayer<4; iLayer++) m_geoCandidatesIndex->push_back(vector<int> ());
  // phi values of candidates.
  m_geoCandidatesPhi = new vector<vector<double > >;
  for(int iLayer=0; iLayer<4; iLayer++) m_geoCandidatesPhi->push_back(vector<double> ());
  // diffStWire values of candidates.
  m_geoCandidatesDiffStWires = new vector<vector<double > >;
  for(int iLayer=0; iLayer<4; iLayer++) m_geoCandidatesDiffStWires->push_back(vector<double> ());
}

void Hough3DFinder::initVersion3(vector<float > & initVariables){
  if(1==2) cout<<initVariables.size()<<endl; // Removes warning when compiling

  // Get input variables.
  string inName;
  string inMin;
  string inMax;
  string inNBit;
  ifstream inputFile;

  inputFile.open(m_inputFileName.c_str());
  if( inputFile ) {
    m_LUT = 1;
    cout<<"Open File: "<< m_inputFileName <<endl;
    while( !inputFile.eof() ) {
      inputFile >> inName >> inMin >> inMax >> inNBit;
      if(inName == "rho") {
        //cout<<inName<<" "<<inMin<<" "<<inMax<<" "<<inNBit<<endl;
        m_rhoMin = atof( inMin.c_str() );
        m_rhoMax = atof( inMax.c_str() );
        m_rhoBit = atoi( inNBit.c_str() );
      }
      if(inName == "phi0") {
        m_phi0Min = atof( inMin.c_str() );
        m_phi0Max = atof( inMax.c_str() );
        m_phi0Bit = atoi( inNBit.c_str() );
      }
      if(inName == "stAxWireFactor") {
        m_stAxWireFactor = atoi( inNBit.c_str() );
      }
    }
  } else {
    cout<<"Could not open input file for GeoFinder."<<endl;
    cout<<"Using default values for input."<<endl;
    m_rhoMin = 0;
    m_rhoMax = 7.5;
    m_rhoBit = 15;
    m_phi0Min = 0;
    m_phi0Max = 6.3;
    m_phi0Bit = 15;
    m_stAxWireFactor = 20;
    m_LUT = 0;
  }
  inputFile.close();

  // Set LUT information
  if(m_LUT) {
    // Make arccos LUT
    int rho_bitSize = bitSize(m_rhoBit,0);
    string first,second;
    ifstream inFileLUT;
    int iLUT;
    m_arcCosLUT = new int*[4];
    for(int iLayer=0; iLayer<4; iLayer++) {
      m_arcCosLUT[iLayer] = new int[rho_bitSize];
      iLUT = 0;
      ostringstream strILayer;
      strILayer << iLayer + 1;
      string fileName = "data/trg/GeoFinder.FPGA.ArcCosLayer" + strILayer.str() + ".coe";
      inFileLUT.open(fileName.c_str());
      if(inFileLUT.is_open() != 1) cout<<"Error in opening " + fileName<<endl;
      while(1) {
        if( iLUT == rho_bitSize + 1 ) break;
        inFileLUT >> first >> second;
        istringstream isecond(second);
        if( iLUT != 0 ) {
          isecond >> m_arcCosLUT[iLayer][iLUT -1];
        }
        iLUT++;
      }
      inFileLUT.close();
      //// Print LUT info
      //for(int iLayer=0; iLayer<4; iLayer++){
      //  for(int iLUT=0; iLUT<rho_bitSize; iLUT++){
      //    cout<<"LUT[<<iLayer<<"][<<iLUT<<"]: "<<m_arcCosLUT[iLUT]<<endl;
      //  }
      //}
    }
    // Make wireSpaceConversion LUT
    int PI2_INT = int(m_Trg_PI * 2 * bitSize(m_phi0Bit,0)/(m_phi0Max - m_phi0Min));
    m_wireConvertLUT = new int*[4];
    for(int iLayer=0; iLayer<4; iLayer++) {
      // Include PI2_INT. PI2_INT is smaller than PI2.
      m_wireConvertLUT[iLayer] = new int[PI2_INT+1];
      iLUT = 0;
      ostringstream strILayer;
      strILayer << iLayer + 1;
      string fileName = "data/trg/GeoFinder.FPGA.WireConvertLayer" + strILayer.str() + ".coe";
      inFileLUT.open(fileName.c_str());
      if(inFileLUT.is_open() != 1) cout<<"Error in opening " + fileName<<endl;
      while(1) {
        // Include PI2_INT. PI2_INT is smaller than PI2.
        if( iLUT == PI2_INT + 2 ) break;
        inFileLUT >> first >> second;
        istringstream isecond(second);
        if( iLUT != 0 ) {
          isecond >> m_wireConvertLUT[iLayer][iLUT -1];
        }
        iLUT++;
      }
      inFileLUT.close();
      //// Print LUT info
      //for(int iLayer=0; iLayer<4; iLayer++){
      //  for(int iLUT=0; iLUT<PI2_INT; iLUT++){
      //    cout<<"LUT[<<iLayer<<"][<<iLUT<<"]: "<<m_arcCosLUT[iLUT]<<endl;
      //  }
      //}
    }

  } // End if if m_LUT flag



  // Make hitMap
  m_hitMap = new bool*[4];
  for(int i=0; i<4; i++) {
    m_hitMap[i] = new bool[m_nWires[i]/2];
    for(int j=0; j<m_nWires[i]/2; j++){
      m_hitMap[i][j] = 0;
    }
  }


  // index values of candidates.
  m_geoCandidatesIndex = new vector<vector<int > >;
  for(int iLayer=0; iLayer<4; iLayer++) m_geoCandidatesIndex->push_back(vector<int> ());
  // phi values of candidates.
  m_geoCandidatesPhi = new vector<vector<double > >;
  for(int iLayer=0; iLayer<4; iLayer++) m_geoCandidatesPhi->push_back(vector<double> ());
  // diffStWire values of candidates.
  m_geoCandidatesDiffStWires = new vector<vector<double > >;
  for(int iLayer=0; iLayer<4; iLayer++) m_geoCandidatesDiffStWires->push_back(vector<double> ());

}

void Hough3DFinder::setInputFileName( string inputFileName ) {
  m_inputFileName = inputFileName;
}

void Hough3DFinder::destVersion1(){
  // Deallocate HoughMesh
  for(int i=0; i<m_nCotSteps; i++){
    for(int j=0; j<m_nZ0Steps; j++){
      delete [] m_houghMeshLayerDiff[i][j];
      delete [] m_houghMeshLayer[i][j];
    }
    delete [] m_houghMeshLayerDiff[i];
    delete [] m_houghMeshLayer[i];
    delete [] m_houghMesh[i];
    delete [] m_houghMeshDiff[i];
  }
  delete [] m_houghMeshLayerDiff;
  delete [] m_houghMeshLayer;
  delete [] m_houghMesh;
  delete [] m_houghMeshDiff;

}

void Hough3DFinder::destVersion2(){
  for(int iLayer=0; iLayer<4; iLayer++) {
    delete m_geoCandidatesIndex;
    delete m_geoCandidatesPhi;
    delete m_geoCandidatesDiffStWires;
  }
}

void Hough3DFinder::destVersion3(){

  for(int i=0; i<4; i++){
    delete [] m_hitMap[i];
  }
  delete [] m_hitMap;
  if(m_LUT) {
    for(int i=0; i<4; i++){
      delete [] m_arcCosLUT[i];
    }
    delete [] m_arcCosLUT;
  }

}

// Hough 3D finder
void Hough3DFinder::runFinderVersion1(vector<double> &trackVariables, vector<vector<double> > &stTSs, vector<double> &tsArcS, vector<vector<double> > &tsZ){

  int charge = (int)trackVariables[0];
  double rho = trackVariables[1];
  double fitPhi0 = trackVariables[2];

  // Clear Hough Meshes.
  for(int i=0; i<m_nCotSteps; i++){
    for(int j=0; j<m_nZ0Steps; j++){
      for(int k=0; k<4; k++){
        m_houghMeshLayerDiff[i][j][k] = 999;
        m_houghMeshLayer[i][j][k] = 0;
      }
      m_houghMesh[i][j] = 0;
      m_houghMeshDiff[i][j] = 0.;
    }
  }

  // Fill Hough mesh.
  double tempCotStart, tempCotEnd;
  double tempZ0Start, tempZ0End;
  double tempZ01, tempZ02;
  int tempHoughZ0;
  double actualCot, actualZ0;
  // Find best vote.
  //double minZ0;

  // Vote in Hough Mesh Layers.
  for(int cotStep=0; cotStep<m_nCotSteps; cotStep++){
    // Find cotStep range for mesh.
    tempCotStart=(cotStep-0.5)*m_cotStepSize+m_cotStart;
    tempCotEnd=(cotStep+0.5)*m_cotStepSize+m_cotStart;
    //cout<<"tempCotStart: "<<tempCotStart<<" tempCotEnd: "<<tempCotEnd<<endl;

    // Find z0 range for mesh per layer.
    for(unsigned iLayer=0; iLayer<4; iLayer++){
      for(unsigned iTS=0; iTS<stTSs[iLayer].size(); iTS++){
        // Find z0 for cot.
        tempZ01=-tsArcS[iLayer]*tempCotStart+tsZ[iLayer][iTS];
        tempZ02=-tsArcS[iLayer]*tempCotEnd+tsZ[iLayer][iTS];

        // Find start and end of range z0.
        if(tempZ01<tempZ02){
          tempZ0Start=tempZ01;
          tempZ0End=tempZ02;
        }
        else{
          tempZ0Start=tempZ02;
          tempZ0End=tempZ01;
        }
        //cout<<"z0Start: "<<tempZ0Start<<endl;
        //cout<<"z0End: "<<tempZ0End<<endl;

        // Do proper rounding for plus and minus values.
        if(tempZ0Start>0){
          tempZ0Start=int(tempZ0Start/m_z0StepSize+0.5);
        }
        else{
          tempZ0Start=int(tempZ0Start/m_z0StepSize-0.5);
        }
        if(tempZ0End>0){
          tempZ0End=int(tempZ0End/m_z0StepSize+0.5);
        }
        else{
          tempZ0End=int(tempZ0End/m_z0StepSize-0.5);
        }

        // To save time do z0 cut off here
        if(tempZ0Start<-(m_nZ0Steps-1)/2) {
          tempZ0Start = -(m_nZ0Steps-1)/2;
        }
        if(tempZ0End>(m_nZ0Steps-1)/2) {
          tempZ0End = (m_nZ0Steps-1)/2;
        }

        // Fill Hough Mesh.
        for(int z0Step=int(tempZ0Start);z0Step<=int(tempZ0End);z0Step++){
          // Cut off if z0Step is bigger or smaller than z0 limit.
          // Not needed anymore.
          if(z0Step>(m_nZ0Steps-1)/2 || z0Step<-(m_nZ0Steps-1)/2) {
            cout<<"cutoff because z0step is bigger or smaller than z0 limit ";
            continue;
          }

          // Change temHoughZ0 if minus.
          if(z0Step<0){
            tempHoughZ0=(m_nZ0Steps-1)/2-z0Step;
          }
          else{ tempHoughZ0 = z0Step; }

          //Change to actual value.
          actualCot=cotStep*m_cotStepSize+m_cotStart;
          actualZ0=z0Step*m_z0StepSize;
          //cout<<"actualCot: "<<actualCot<<" actualZ0: "<<actualZ0<<endl;


          m_houghMeshLayer[cotStep][tempHoughZ0][iLayer]=1;
          // Find minimum z difference for the vote.
          m_minDiffHough = abs(actualCot*tsArcS[iLayer]+actualZ0-tsZ[iLayer][iTS]);
          if(m_houghMeshLayerDiff[cotStep][tempHoughZ0][iLayer]>m_minDiffHough){
            m_houghMeshLayerDiff[cotStep][tempHoughZ0][iLayer]=m_minDiffHough;
          }

        } // End of z0 vote loop.
      } // End of TS loop.
    }  // End of layer loop.
  } // End of cot vote loop.

  // Filling HoughMesh. Combines the seperate HoughMeshLayers.
  for(int houghCot=0;houghCot<m_nCotSteps;houghCot++){
    for(int houghZ0=0;houghZ0<m_nZ0Steps;houghZ0++){
      //Change back tempHoughZ0 if minus
      if(houghZ0>(m_nZ0Steps-1)/2){
        tempHoughZ0=(m_nZ0Steps-1)/2-houghZ0;
      } else {
        tempHoughZ0 = houghZ0;
      }
      //Change to actual value
      actualCot=houghCot*m_cotStepSize+m_cotStart;
      actualZ0=tempHoughZ0*m_z0StepSize;

      for(int layer=0; layer<4; layer++){
        m_houghMesh[houghCot][houghZ0] += m_houghMeshLayer[houghCot][houghZ0][layer];
        if(m_houghMeshLayerDiff[houghCot][houghZ0][layer] != 999) m_houghMeshDiff[houghCot][houghZ0] += m_houghMeshLayerDiff[houghCot][houghZ0][layer];
        //if(houghMeshLayer[houghCot][houghZ0][layer]==1) hhough00->Fill(actualCot,actualZ0);
      } // End of combining votes
    } // End loop for houghZ0
  } // End loop for houghCot

  // Find best vote. By finding highest votes and comparing all votes and pick minimum diff z.
  m_houghMax = 0;
  for(int houghCot=0;houghCot<m_nCotSteps;houghCot++){
    for(int houghZ0=0;houghZ0<m_nZ0Steps;houghZ0++){
      // Changes back values for minus
      if(houghZ0>(m_nZ0Steps-1)/2){
        tempHoughZ0=(m_nZ0Steps-1)/2-houghZ0;
      }
      else{ tempHoughZ0=houghZ0;}
      // Find highest vote
      if(m_houghMax<m_houghMesh[houghCot][houghZ0]){
        m_houghMax=m_houghMesh[houghCot][houghZ0];
        // If highest vote changes, need to initialize minZ0, minDiffHough
        //minZ0 = 9999;
        m_minDiffHough = 9999;
      }
      // When highest vote
      if(m_houghMax==m_houghMesh[houghCot][houghZ0]){
        // For second finder version
        // When z0 is minimum
        //if(minZ0>abs(tempHoughZ0)) {
        //  cout<<"minZ0: "<<minZ0<<" tempHoughZ0: "<<tempHoughZ0<<endl;
        //  minZ0 = tempHoughZ0;
        //  bestCot = houghCot;
        //  bestZ0 = tempHoughZ0;
        //}
        // For third finder version
        // When minDiffHough is minimum
        if(m_minDiffHough>m_houghMeshDiff[houghCot][houghZ0]){
          m_minDiffHough = m_houghMeshDiff[houghCot][houghZ0];
          m_bestCot = houghCot;
          m_bestZ0 = tempHoughZ0;
        }
      }
    } // End of houghZ0 loop
  } // End of houghCot loop
  //cout<<"JB bestCot: "<<m_bestCot<<" bestZ0: "<<m_bestZ0<<" "<<"#Votes: "<<m_houghMax<<endl;
  //cout<<"JB foundCot: "<<m_bestCot*m_cotStepSize+m_cotStart<<" foundZ0: "<<m_bestZ0*m_z0StepSize<<endl;
  //cout<<"mcCot: "<<mcCot<<" mcZ0: "<<mcZ0<<endl;

  // Finds the related TS from bestCot and bestZ0

  // Find z and phiSt from bestCot and bestZ0 (arcS is dependent to pT)
  for(int i=0; i<4; i++){
    m_foundZ[i] = (m_bestCot*m_cotStepSize+m_cotStart)*tsArcS[i]+(m_bestZ0*m_z0StepSize); 
    m_foundPhiSt[i] = fitPhi0+charge*acos(m_rr[i]/2/rho)+2*asin((m_ztostraw[i]-m_foundZ[i])*tan(m_anglest[i])/2/m_rr[i]); 
    if(m_foundPhiSt[i]>2*m_Trg_PI) m_foundPhiSt[i]-=2*m_Trg_PI;
    if(m_foundPhiSt[i]<0) m_foundPhiSt[i]+=2*m_Trg_PI;
  }
  //cout<<"JB FoundPhiSt[0]: "<<m_foundPhiSt[0]<<" FoundPhiSt[1]: "<<m_foundPhiSt[1]<<" FoundPhiSt[2]: "<<m_foundPhiSt[2]<<" FoundPhiSt[3]: "<<m_foundPhiSt[3]<<endl;

  // Find closest phi out of canidates
  double minDiff[4]={999,999,999,999};
  for(unsigned iLayer=0; iLayer<4; iLayer++){
    for(unsigned iTS=0; iTS<stTSs[iLayer].size(); iTS++){
      if(minDiff[iLayer]> abs(m_foundPhiSt[iLayer]-stTSs[iLayer][iTS])){
        minDiff[iLayer] = abs(m_foundPhiSt[iLayer]-stTSs[iLayer][iTS]);
        m_bestTS[iLayer] = stTSs[iLayer][iTS];
        m_bestTSIndex[iLayer] = (int)iTS;
      }
    }
  }
  //cout<<"JB BestPhiSt[0]: "<<m_bestTS[0]<<" BestPhiSt[1]: "<<m_bestTS[1]<<" BestPhiSt[2]: "<<m_bestTS[2]<<" BestPhiSt[3]: "<<m_bestTS[3]<<endl;

}

void Hough3DFinder::runFinderVersion2(vector<double> &trackVariables, vector<vector<double> > &stTSs){

  // Clear m_geoCandidatesIndex
  for(int iLayer=0; iLayer<4; iLayer++) {
    (*m_geoCandidatesIndex)[iLayer].clear();
    (*m_geoCandidatesPhi)[iLayer].clear();
    (*m_geoCandidatesDiffStWires)[iLayer].clear();
  }

  int charge = (int)trackVariables[0];
  double rho = trackVariables[1];
  double fitPhi0 = trackVariables[2];
  double tsDiffSt;

  for( int iLayer=0; iLayer<4; iLayer++){
    m_stAxPhi[iLayer] = calStAxPhi(charge, m_anglest[iLayer], m_ztostraw[iLayer],  m_rr[iLayer], rho, fitPhi0);
    if(stTSs[iLayer].size()==0) cout<<"stTSs["<<iLayer<<"] is zero"<<endl;
    for(unsigned iTS=0; iTS<stTSs[iLayer].size(); iTS++){
      // Find number of wire difference
      tsDiffSt = m_stAxPhi[iLayer] - stTSs[iLayer][iTS];
      if(tsDiffSt > m_Trg_PI) tsDiffSt -= 2*m_Trg_PI;
      if(tsDiffSt < -m_Trg_PI) tsDiffSt += 2*m_Trg_PI;
      tsDiffSt = tsDiffSt/2/m_Trg_PI*m_nWires[iLayer]/2;
      
      //cout<<"JB ["<<iLayer<<"]["<<iTS<<"] tsDiffSt: "<<tsDiffSt<<" stTSs:"<<stTSs[iLayer][iTS]<<"rho: "<<rho<<" phi0: "<<fitPhi0<<endl;
      // Save index if condition is in 10 wires
      if(iLayer%2==0){
        if(tsDiffSt>0 && tsDiffSt<=10){
          (*m_geoCandidatesIndex)[iLayer].push_back(iTS);
          (*m_geoCandidatesPhi)[iLayer].push_back(stTSs[iLayer][iTS]);
          (*m_geoCandidatesDiffStWires)[iLayer].push_back(tsDiffSt);
        }
      } else {
        if(tsDiffSt<0 && tsDiffSt>=-10){
          (*m_geoCandidatesIndex)[iLayer].push_back(iTS);
          (*m_geoCandidatesPhi)[iLayer].push_back(stTSs[iLayer][iTS]);
          (*m_geoCandidatesDiffStWires)[iLayer].push_back(tsDiffSt);
        }
      } // End of saving index
      
    } // Candidate loop
  } // Layer loop

  // Print all candidates.
  //for( int iLayer=0; iLayer<4; iLayer++){
  //  for(unsigned iTS=0; iTS<(*m_geoCandidatesIndex)[iLayer].size(); iTS++){
  //    //cout<<"cand ["<<iLayer<<"]["<<iTS<<"]: "<<(*m_geoCandidatesIndex)[iLayer][iTS]<<endl;
  //    cout<<"cand ["<<iLayer<<"]["<<iTS<<"]: "<<(*m_geoCandidatesPhi)[iLayer][iTS]<<endl;
  //  }
  //}

  // Pick middle candidate if multiple candidates
  //double meanWireDiff[4] = { 5, 5, 5, 5 };
  // z=0 wireDiff
  //double meanWireDiff[4] = { 3.08452, 2.61314, 2.84096, 3.06938 };
  // mean wire diff
  double meanWireDiff[4] = { 3.68186, 3.3542, 3.9099, 4.48263 };
  for(int iLayer=0; iLayer<4; iLayer++){
    double bestDiff=999;
    if((*m_geoCandidatesIndex)[iLayer].size()==0) {
      //cout<<"No St Candidate in GeoFinder"<<endl;
    } else {
      for(int iTS=0; iTS<int((*m_geoCandidatesIndex)[iLayer].size()); iTS++){
        tsDiffSt = m_stAxPhi[iLayer] - stTSs[iLayer][(*m_geoCandidatesIndex)[iLayer][iTS]];
        if(tsDiffSt > m_Trg_PI) tsDiffSt -= 2*m_Trg_PI;
        if(tsDiffSt < -m_Trg_PI) tsDiffSt += 2*m_Trg_PI;
        tsDiffSt = tsDiffSt/2/m_Trg_PI*m_nWires[iLayer]/2;
        // Pick the better TS
        //if(abs(abs(tsDiffSt)-5) < bestDiff){
        if(abs(abs(tsDiffSt)-meanWireDiff[iLayer]) < bestDiff){
          //bestDiff = abs(abs(tsDiffSt)-5);
          bestDiff = abs(abs(tsDiffSt)-meanWireDiff[iLayer]);
          m_bestTS[iLayer] = stTSs[iLayer][(*m_geoCandidatesIndex)[iLayer][iTS]]; 
          m_bestTSIndex[iLayer] = (*m_geoCandidatesIndex)[iLayer][iTS];
        }
      } // TS loop
    } // If there is a TS candidate
  } // Layer loop



  //// Print all candidates.
  //for( int iLayer=0; iLayer<4; iLayer++){
  //  cout<<"best ["<<iLayer<<"]: "<<m_bestTSIndex[iLayer]<<" "<<m_bestTS[iLayer]<<endl;
  //}

}

void Hough3DFinder::runFinderVersion3(vector<double> &trackVariables, vector<vector<double> > &stTSs){

  // Clear m_geoCandidatesIndex
  for(int iLayer=0; iLayer<4; iLayer++) {
    (*m_geoCandidatesPhi)[iLayer].clear();
    (*m_geoCandidatesIndex)[iLayer].clear();
    (*m_geoCandidatesDiffStWires)[iLayer].clear();
  }
  // Clear hit map for track
  for( int iLayer=0; iLayer<4; iLayer++ ) {
    for( int iTS=0; iTS<m_nWires[iLayer]/2; iTS++ ){
      m_hitMap[iLayer][iTS] = 0;
    }
  }
  // Clear FPGA input and output values.
  m_FPGAInput.clear();
  m_FPGAOutput.clear();

  // 2D Track Variables
  int charge = (int)trackVariables[0];
  double rho = trackVariables[1];
  double fitPhi0 = trackVariables[2];

  // Fill hitMap
  int iHitTS;
  for(unsigned iLayer=0; iLayer<4; iLayer++) {
    //cout<<"["<<iLayer<<"] size:"<<stTSs[iLayer].size()<<endl;
    for(unsigned iTS=0; iTS<stTSs[iLayer].size(); iTS++) {
      iHitTS = int(stTSs[iLayer][iTS]*m_nWires[iLayer]/2/2/m_Trg_PI+0.5);
      //cout<<"["<<iLayer<<"] iHitTS: "<<iHitTS<<" stTSs: "<<stTSs[iLayer][iTS]<<endl;
      //cout<<iHitTS*2*m_Trg_PI/m_nWires[iLayer]*2<<endl;
      m_hitMap[iLayer][iHitTS] = 1;
    } // End iTS
  } // End layer

  //// Print hitMap
  //for(int iLayer=0; iLayer<4; iLayer++) {
  //  cout<<"["<<iLayer<<"]["<<m_nWires[iLayer]/2<<"]";
  //  for(int iTS=m_nWires[iLayer]/2-1; iTS>=0; iTS--) {
  //    cout<<m_hitMap[iLayer][iTS];
  //  }
  //  cout<<endl;
  //} // End layer

  // For integer space
  int rho_int, rho_bitSize;
  int fitPhi0_int, fitPhi0_bitSize;
  int arcCos_int, myphiz_int;
  rho_bitSize = bitSize(m_rhoBit,0);
  fitPhi0_bitSize = bitSize(m_phi0Bit,0);
  // Find 2*PI in integer space using phi0. phi0 min must be 0.
  // PI2_INT is the slightly smaller than 2*PI
  int PI2_INT = int(m_Trg_PI * 2 / m_phi0Max * fitPhi0_bitSize);

  // For integer space
  findExtreme(m_findRhoMax, m_findRhoMin, rho);
  findExtreme(m_findPhi0Max, m_findPhi0Min, fitPhi0);
  changeInteger(rho_int, rho, m_rhoMin, m_rhoMax, rho_bitSize);
  changeInteger(fitPhi0_int, fitPhi0, m_phi0Min, m_phi0Max, fitPhi0_bitSize);
  findExtreme(m_findRhoIntMax, m_findRhoIntMin, rho_int);
  findExtreme(m_findPhi0IntMax, m_findPhi0IntMin, fitPhi0_int);
  m_FPGAInput.push_back(rho_int);
  m_FPGAInput.push_back(fitPhi0_int);
  changeReal(fitPhi0, fitPhi0_int, m_phi0Min, m_phi0Max, fitPhi0_bitSize);
  changeReal(rho, rho_int, m_rhoMin, m_rhoMax, rho_bitSize);
  m_FPGAInput.push_back(charge);

  // Select TS candidate range variables
  double stCand[4][10];
  //double stCandDiff[4][10];
  double stCandIndex[4][10];
  double stAxPhi[4];
  int stAxWire[4];
  int stAxWire_int[4];
  int indexTS;
  double acos_real, myphiz;
  int stCandHitMap[4][10];

  // Initalize stCand
  for( int i=0; i<4; i++ ) {
    for( int j=0; j<10; j++) {
      stCand[i][j] = 9999;
      //stCandDiff[i][j] = 9999;
      stCandIndex[i][j] = 9999;
      stCandHitMap[i][j] = 0;
    }
    //cout<<"nWires: "<<m_nWires[i]/2<<" ";
  }
  //cout<<endl;


  // Select TS candidate range algorithm
  for( int iLayer=0; iLayer<4; iLayer++) {

    // Prevents crash
    if(m_rr[iLayer]/(2*rho) > 1 ) rho = m_rr[iLayer]/2;
    // This is done in real space
    acos_real = acos(m_rr[iLayer]/(2*rho));

    // For integer space
    findExtreme(m_findArcCosMax, m_findArcCosMin, acos_real);
    changeInteger(arcCos_int, acos_real, m_phi0Min, m_phi0Max, fitPhi0_bitSize);
    findExtreme(m_findArcCosIntMax, m_findArcCosIntMin, arcCos_int);
    m_FPGAOutput.push_back(arcCos_int);
    if(arcCos_int != m_arcCosLUT[iLayer][rho_int]) {
      cout<<"Error with arcCos LUT: "<<rho_int<<" "<<arcCos_int<<" "<<m_arcCosLUT[iLayer][rho_int]<<endl;
    }

    if(charge==1){
      // Actual function
      //myphiz = +acos_real+fitPhi0;
      myphiz_int = +arcCos_int+fitPhi0_int;
    }
    else{
      // Actual function
      //myphiz = -acos_real+fitPhi0;
      myphiz_int = -arcCos_int+fitPhi0_int;
    }

    changeReal(myphiz, myphiz_int, m_phi0Min, m_phi0Max, fitPhi0_bitSize);
    // Find extreme value
    findExtreme(m_findPhiZMax, m_findPhiZMin, myphiz);
    findExtreme(m_findPhiZIntMax, m_findPhiZIntMin, myphiz_int);
    m_FPGAOutput.push_back(myphiz_int);

    // Actual function
    //if(myphiz>2*m_Trg_PI) myphiz-=2*m_Trg_PI;
    //if(myphiz<0) myphiz+=2*m_Trg_PI;
    // This part can be optimized depending on bit size.
    // This part might not be needed.
    // It can be done when changing to wire space below.
    // But can reduce input bitsize for below.
    if(myphiz_int > PI2_INT) myphiz_int -= PI2_INT + 1;
    if(myphiz_int < 0 ) myphiz_int += PI2_INT + 1;

    // For real space
    changeReal(myphiz, myphiz_int, m_phi0Min, m_phi0Max, fitPhi0_bitSize);

    stAxPhi[iLayer] = myphiz;


    // Choose stAxWire depending on layer

    // Change to wire space using fake LUT
    // This is done in real space. Result is in integer space.
    if( iLayer%2 == 0 ) {
      // Round down
      stAxWire[iLayer] = int(stAxPhi[iLayer]/2/m_Trg_PI*m_nWires[iLayer]/2);
    } else {
      // Round up
      stAxWire[iLayer] = int(stAxPhi[iLayer]/2/m_Trg_PI*m_nWires[iLayer]/2 + 1);
    }
    // Change to wire space by multiply division method.
    stAxWire_int[iLayer] = myphiz_int * int(m_nWires[iLayer]/2/2/m_Trg_PI/fitPhi0_bitSize*(m_phi0Max-m_phi0Min)*pow(2.0,m_stAxWireFactor));
    if( iLayer%2 == 0 ) {
      stAxWire_int[iLayer] = stAxWire_int[iLayer] >> m_stAxWireFactor;
    } else {
      stAxWire_int[iLayer] = (stAxWire_int[iLayer] >> m_stAxWireFactor) + 1;
    }
    //cout<<"Conversion Factor["<<iLayer<<"]: "<<int(m_nWires[iLayer]/2/2/m_Trg_PI/fitPhi0_bitSize*(m_phi0Max-m_phi0Min)*pow(2.0,m_stAxWireFactor))<<" "<<m_nWires[iLayer]/2<<endl;
    //// Check if something is different with multiply division method.
    //if(stAxWire[iLayer] != stAxWire_int[iLayer]) {
    //  cout<<"Layer["<<iLayer<<"]: "<<myphiz_int<<" "<<stAxPhi[iLayer]<<" "<<stAxPhi[iLayer]*m_nWires[iLayer]/2/2/m_Trg_PI<<" "<<stAxWire[iLayer]<<endl;
    //  //cout<<"Layer["<<iLayer<<"]: "<<myphiz_int<<" "<<stAxPhi[iLayer]<<" "<<myphiz_int * int(m_nWires[iLayer]/2/2/m_Trg_PI/fitPhi0_bitSize*(m_phi0Max-m_phi0Min)*pow(2.0,m_stAxWireFactor))<<" "<<myphiz_int * int(m_nWires[iLayer]/2/2/m_Trg_PI/fitPhi0_bitSize*(m_phi0Max-m_phi0Min)*pow(2.0,m_stAxWireFactor)) / pow(2.0,m_stAxWireFactor)<<endl;
    //  cout<<"Layer["<<iLayer<<"]: "<<myphiz_int<<" "<<stAxPhi[iLayer]<<" "<<myphiz_int * int(m_nWires[iLayer]/2/2/m_Trg_PI/fitPhi0_bitSize*(m_phi0Max-m_phi0Min)*pow(2.0,m_stAxWireFactor)) / pow(2.0,m_stAxWireFactor)<<" "<<stAxWire_int[iLayer]<<endl;
    //}
    stAxWire[iLayer] = stAxWire_int[iLayer];


    m_FPGAOutput.push_back(stAxWire[iLayer]);

    //cout<<stAxPhi[iLayer]<<" "<<stAxWire[iLayer]<<endl;
    //cout<<m_nWires[iLayer]/2/2/m_Trg_PI/81.2698412698<<endl;

    // If index is at edgeIndex move to 0.
    if(stAxWire[iLayer] == m_nWires[iLayer]/2) stAxWire[iLayer] = 0;
    indexTS = stAxWire[iLayer];

    //// Check if something is different with LUT method.
    //if(stAxWire[iLayer] != m_wireConvertLUT[iLayer][myphiz_int]) {
    //  cout<<"Error with wireConvert LUT: "<<myphiz_int<<" "<<stAxWire[iLayer]<<" "<<m_wireConvertLUT[iLayer][myphiz_int]<<endl;
    //}



    //cout<<"StAxWire["<<iLayer<<"]: "<<stAxWire[iLayer]<<endl;
    // Save stAxPhi
    m_stAxPhi[iLayer] = stAxWire[iLayer]*2*m_Trg_PI/m_nWires[iLayer]*2;

    // Select TS using stAxWire between 10 wires
    // Need to integerize diffPhi, stCand if used later.
    for( int iTS=0; iTS<10; iTS++) {
      if(m_hitMap[iLayer][indexTS] == 1) {
        // Save phi. Convert hitmap location to phi
        // This is done in real space.
        stCand[iLayer][iTS] = indexTS*2*m_Trg_PI/m_nWires[iLayer]*2;
        (*m_geoCandidatesPhi)[iLayer].push_back(stCand[iLayer][iTS]);
        // Fill with dummy. Will be filled later.
        (*m_geoCandidatesIndex)[iLayer].push_back(9999);

        // Save stCandDiff in wire space. Not needed in integer space.
        double diffPhi = stAxPhi[iLayer]-indexTS*2*m_Trg_PI/m_nWires[iLayer]*2;
        if (diffPhi > m_Trg_PI ) diffPhi -= 2*m_Trg_PI;
        if (diffPhi < -m_Trg_PI ) diffPhi += 2*m_Trg_PI;
        //stCandDiff[iLayer][iTS] = diffPhi/2/m_Trg_PI*m_nWires[iLayer]/2;
        (*m_geoCandidatesDiffStWires)[iLayer].push_back(diffPhi/2/m_Trg_PI*m_nWires[iLayer]/2);

        // Save indexTS
        stCandHitMap[iLayer][iTS] = 1;

      }
      
      //// Print info for debugging
      //cout<<"StCand["<<iLayer<<"]["<<iTS<<"]: "<<stCand[iLayer][iTS]<<" index: "<<indexTS;
      //cout<<" stAxPhi: "<<stAxPhi[iLayer];
      //double diffPhi = stAxPhi[iLayer]-indexTS*2*m_Trg_PI/m_nWires[iLayer]*2;
      //if (diffPhi > m_Trg_PI ) diffPhi -= 2*m_Trg_PI;
      //if (diffPhi < -m_Trg_PI ) diffPhi += 2*m_Trg_PI;
      //cout<<" diff: "<<diffPhi/2/m_Trg_PI*m_nWires[iLayer]/2<<endl;

      // Choose scan direction depending on layer
      if(iLayer%2 == 0) {
        indexTS--;
        // If index is below 0 deg goto 360-delta deg
        if(indexTS < 0 ) indexTS =  m_nWires[iLayer]/2-1;
      }
      else {
        indexTS++;
        // If index is 360 deg goto 0 deg
        if(indexTS >= m_nWires[iLayer]/2) indexTS = 0;
      }
    } // Select TS using stAxWire between 10 wires

  } // Layer loop

  //// Print stCandHitMap
  //for(unsigned iLayer=0; iLayer<4; iLayer++){
  //  cout<<"stCandHitMap["<<iLayer<<"]: ";
  //  for(int iTS=9; iTS>=0; iTS--){
  //    cout<<stCandHitMap[iLayer][iTS];
  //  }
  //  cout<<endl;
  //}
  
  // Save stCandHitMap in FPGAOutput
  for(unsigned iLayer=0; iLayer<4; iLayer++){
    int stCandHitMapInt=0;
    for(int iTS=0; iTS<10; iTS++) {
      stCandHitMapInt+=int(stCandHitMap[iLayer][iTS]*pow(10.,iTS));
    }
    m_FPGAOutput.push_back(stCandHitMapInt);
  }


  // Find and save index of TS. For vector.
  for(unsigned iLayer=0; iLayer<4; iLayer++) {
    for(unsigned iCand=0; iCand<(*m_geoCandidatesPhi)[iLayer].size(); iCand++) {
      for(unsigned iTS=0; iTS<stTSs[iLayer].size(); iTS++) {
        //cout<<"["<<iLayer<<"] iHitTS: "<<iHitTS<<" stTSs: "<<stTSs[iLayer][iTS]<<endl;
        if( fabs( (*m_geoCandidatesPhi)[iLayer][iCand] - stTSs[iLayer][iTS] ) < 0.0001) {
          (*m_geoCandidatesIndex)[iLayer][iCand] = iTS;
          break;
        }
      } // End iTS
    } // End iCand
  } // End layer

  // Find and save index of TS. For array
  for(unsigned iLayer=0; iLayer<4; iLayer++) {
    for(unsigned iCand=0; iCand<10; iCand++) {
      for(unsigned iTS=0; iTS<stTSs[iLayer].size(); iTS++) {
        //cout<<"["<<iLayer<<"] iHitTS: "<<iHitTS<<" stTSs: "<<stTSs[iLayer][iTS]<<endl;
        if( fabs( stCand[iLayer][iCand] - stTSs[iLayer][iTS] ) < 0.0001) {
          stCandIndex[iLayer][iCand] = iTS;
          break;
        }
      } // End iTS
    } // End iCand
  } // End layer

  
  //// Find if candidates have a match.
  //for( int iLayer=0; iLayer<4; iLayer++ ) {
  //  int nCandidates = tempIndex[iLayer].size();
  //  int nWCandidates = (*m_geoCandidatesPhi)[iLayer].size();
  //  // Check if same number of candidates
  //  if( nCandidates != nWCandidates ) {
  //    cout<<"Candidates: "<<nCandidates<<" "<<nWCandidates<<endl;
  //    for(int iTS=0; iTS<nCandidates; iTS++){
  //      cout<<"cand1 ["<<iLayer<<"]["<<iTS<<"]: "<<tempPhi[iLayer][iTS]<<endl;
  //    }
  //    for(int iTS=0; iTS<nWCandidates; iTS++){
  //      cout<<"cand2 ["<<iLayer<<"]["<<iTS<<"]: "<<(*m_geoCandidatesPhi)[iLayer][iTS]<<endl;
  //    }
  //  // Check if the candidates are the same
  //  } else {
  //    bool sameFlag = 1;
  //    for(int iTS=0; iTS<nCandidates; iTS++){
  //      bool sameFlagTS = 0;
  //      for(int iTS2=0; iTS2<nCandidates; iTS2++){
  //        if( fabs( tempPhi[iLayer][iTS] - (*m_geoCandidatesPhi)[iLayer][iTS2] ) < 0.001 ) sameFlagTS = 1;
  //        //if( fabs( tempIndex[iLayer][iTS] - (*m_geoCandidatesIndex)[iLayer][iTS2] ) < 0.001 ) sameFlagTS = 1;
  //      }
  //      sameFlag = sameFlag * sameFlagTS;
  //    }
  //    if( sameFlag == 0 ) {
  //      for(int iTS=0; iTS<nCandidates; iTS++){
  //        cout<<"cand1 ["<<iLayer<<"]["<<iTS<<"]: "<<tempPhi[iLayer][iTS]<<endl;
  //      }
  //      for(int iTS=0; iTS<nWCandidates; iTS++){
  //        cout<<"cand2 ["<<iLayer<<"]["<<iTS<<"]: "<<(*m_geoCandidatesPhi)[iLayer][iTS]<<endl;
  //      }
  //    }
  //  }
  //} // End layer loop

  
  // Pick middle candidate if multiple candidates
  //double meanWireDiff[4] = { 5, 5, 5, 5 };
  // z=0 wireDiff
  //double meanWireDiff[4] = { 3.08452, 2.61314, 2.84096, 3.06938 };
  // mean wire diff
  //double meanWireDiff[4] = { 3.68186, 3.3542, 3.9099, 4.48263 };
  // mean wire for iTS
  double meanWireDiff[4] = { 3.1826, 2.84745, 3.40936, 3.99266 };

  double bestTS[4] = {9999, 9999, 9999, 9999};
  for(int iLayer=0; iLayer<4; iLayer++){
    double bestDiff=999;
    for(int iTS=0; iTS<10; iTS++) {
      if( stCand[iLayer][iTS] == 9999 ) continue;
      // Pick the better TS
      if(abs(iTS-meanWireDiff[iLayer]) < bestDiff) {
      //if( abs( fabs(stCandDiff[iLayer][iTS])-meanWireDiff[iLayer] ) < bestDiff ) {
        bestDiff = abs(iTS-meanWireDiff[iLayer]);
        //bestDiff = abs( fabs(stCandDiff[iLayer][iTS])-meanWireDiff[iLayer] );
        //cout<<"BestDiff["<<iLayer<<"]["<<iTS<<"]: "<<bestDiff<<" diffSt: "<<(*m_geoCandidatesDiffStWires)[iLayer][iTS]<<endl;
        bestTS[iLayer] = stCand[iLayer][iTS]; 

        m_bestTS[iLayer] = stCand[iLayer][iTS];
        m_bestTSIndex[iLayer] = int(stCandIndex[iLayer][iTS]+0.5);
      }
    } // TS loop
  } // Layer loop

  // For integer space
  for( int iLayer=0; iLayer<4; iLayer++) {
    m_FPGAOutput.push_back(int(bestTS[iLayer]/2/m_Trg_PI*m_nWires[iLayer]/2+0.5));
  }

  //// Check m_bestTS with version2
  //for(int iLayer=0; iLayer<4; iLayer++) {
  //  if(fabs(m_bestTS[iLayer] - tempBestTS[iLayer])>0.00001) {
  //  //if(fabs(m_bestTSIndex[iLayer] - tempBestTSIndex[iLayer])>0.00001) {
  //    cout<<"Error: "<<m_bestTS[iLayer]<<" "<<tempBestTS[iLayer]<<endl;
  //  }
  //}

  //// Print the best candidate
  //for( int iLayer=0; iLayer<4; iLayer++) {
  //  cout<<"best["<<iLayer<<"]: "<<bestTS[iLayer]<<endl;
  //}

}

void Hough3DFinder::getValues(const string& input, vector<double> &result ){
  //Clear result vector
  result.clear();

  if(input=="bestCot"){
    result.push_back(m_bestCot);
  }
  if(input=="bestZ0"){
    result.push_back(m_bestZ0);
  }
  if(input=="houghMax"){
    result.push_back(m_houghMax);
  }
  if(input=="minDiffHough"){
    result.push_back(m_minDiffHough);
  }
  if(input=="foundZ"){
    result.push_back(m_foundZ[0]);
    result.push_back(m_foundZ[1]);
    result.push_back(m_foundZ[2]);
    result.push_back(m_foundZ[3]);
  }
  if(input=="foundPhiSt"){
    result.push_back(m_foundPhiSt[0]);
    result.push_back(m_foundPhiSt[1]);
    result.push_back(m_foundPhiSt[2]);
    result.push_back(m_foundPhiSt[3]);
  }
  if(input=="bestTS"){
    result.push_back(m_bestTS[0]);
    result.push_back(m_bestTS[1]);
    result.push_back(m_bestTS[2]);
    result.push_back(m_bestTS[3]);
  }
  if(input=="bestTSIndex"){
    result.push_back(m_bestTSIndex[0]);
    result.push_back(m_bestTSIndex[1]);
    result.push_back(m_bestTSIndex[2]);
    result.push_back(m_bestTSIndex[3]);
  }
  if(input=="st0GeoCandidatesPhi"){
    for(int iTS=0; iTS<int((*m_geoCandidatesPhi)[0].size()); iTS++) 
      result.push_back((*m_geoCandidatesPhi)[0][iTS]);
  }
  if(input=="st1GeoCandidatesPhi"){
    for(int iTS=0; iTS<int((*m_geoCandidatesPhi)[1].size()); iTS++) 
      result.push_back((*m_geoCandidatesPhi)[1][iTS]);
  }
  if(input=="st2GeoCandidatesPhi"){
    for(int iTS=0; iTS<int((*m_geoCandidatesPhi)[2].size()); iTS++) 
      result.push_back((*m_geoCandidatesPhi)[2][iTS]);
  }
  if(input=="st3GeoCandidatesPhi"){
    for(int iTS=0; iTS<int((*m_geoCandidatesPhi)[3].size()); iTS++) 
      result.push_back((*m_geoCandidatesPhi)[3][iTS]);
  }

  if(input=="st0GeoCandidatesDiffStWires"){
    for(int iTS=0; iTS<int((*m_geoCandidatesDiffStWires)[0].size()); iTS++) 
      result.push_back((*m_geoCandidatesDiffStWires)[0][iTS]);
  }
  if(input=="st1GeoCandidatesDiffStWires"){
    for(int iTS=0; iTS<int((*m_geoCandidatesDiffStWires)[1].size()); iTS++) 
      result.push_back((*m_geoCandidatesDiffStWires)[1][iTS]);
  }
  if(input=="st2GeoCandidatesDiffStWires"){
    for(int iTS=0; iTS<int((*m_geoCandidatesDiffStWires)[2].size()); iTS++) 
      result.push_back((*m_geoCandidatesDiffStWires)[2][iTS]);
  }
  if(input=="st3GeoCandidatesDiffStWires"){
    for(int iTS=0; iTS<int((*m_geoCandidatesDiffStWires)[3].size()); iTS++) 
      result.push_back((*m_geoCandidatesDiffStWires)[3][iTS]);
  }

  if(input=="st0GeoCandidatesIndex"){
    for(int iTS=0; iTS<int((*m_geoCandidatesIndex)[0].size()); iTS++) 
      result.push_back((*m_geoCandidatesIndex)[0][iTS]);
  }
  if(input=="st1GeoCandidatesIndex"){
    for(int iTS=0; iTS<int((*m_geoCandidatesIndex)[1].size()); iTS++) 
      result.push_back((*m_geoCandidatesIndex)[1][iTS]);
  }
  if(input=="st2GeoCandidatesIndex"){
    for(int iTS=0; iTS<int((*m_geoCandidatesIndex)[2].size()); iTS++) 
      result.push_back((*m_geoCandidatesIndex)[2][iTS]);
  }
  if(input=="st3GeoCandidatesIndex"){
    for(int iTS=0; iTS<int((*m_geoCandidatesIndex)[3].size()); iTS++) 
      result.push_back((*m_geoCandidatesIndex)[3][iTS]);
  }

  if(input=="stAxPhi"){
    result.push_back(m_stAxPhi[0]);
    result.push_back(m_stAxPhi[1]);
    result.push_back(m_stAxPhi[2]);
    result.push_back(m_stAxPhi[3]);
  }

  if(input=="extreme"){
    result.push_back(m_findRhoMax);
    result.push_back(m_findRhoMin);
    result.push_back(m_findPhi0Max);
    result.push_back(m_findPhi0Min);
    result.push_back(m_findArcCosMax);
    result.push_back(m_findArcCosMin);
    result.push_back(m_findPhiZMax);
    result.push_back(m_findPhiZMin);
    result.push_back(m_findRhoIntMax);
    result.push_back(m_findRhoIntMin);
    result.push_back(m_findPhi0IntMax);
    result.push_back(m_findPhi0IntMin);
    result.push_back(m_findArcCosIntMax);
    result.push_back(m_findArcCosIntMin);
    result.push_back(m_findPhiZIntMax);
    result.push_back(m_findPhiZIntMin);
    //// Print All values
    //for(unsigned i=0; i<result.size(); i++) {
    //  cout<<result[i]<<endl;
    //}
  }

  if(input=="FPGAInput") {
    for(unsigned iInput=0; iInput<m_FPGAInput.size(); iInput++){
      result.push_back(m_FPGAInput[iInput]);
    }
  }

  if(input=="FPGAOutput") {
    for(unsigned iOutput=0; iOutput<m_FPGAOutput.size(); iOutput++){
      result.push_back(m_FPGAOutput[iOutput]);
    }
  }

  if(input=="hitmapLayer1") {
    for(unsigned iOutput=0; iOutput<unsigned(m_nWires[0]/2); iOutput++){
      result.push_back(m_hitMap[0][iOutput]);
    }
  }

  if(input=="hitmapLayer2") {
    for(unsigned iOutput=0; iOutput<unsigned(m_nWires[1]/2); iOutput++){
      result.push_back(m_hitMap[1][iOutput]);
    }
  }

  if(input=="hitmapLayer3") {
    for(unsigned iOutput=0; iOutput<unsigned(m_nWires[2]/2); iOutput++){
      result.push_back(m_hitMap[2][iOutput]);
    }
  }

  if(input=="hitmapLayer4") {
    for(unsigned iOutput=0; iOutput<unsigned(m_nWires[3]/2); iOutput++){
      result.push_back(m_hitMap[3][iOutput]);
    }
  }

}

void Hough3DFinder::getHoughMeshLayer(bool ***& houghMeshLayer ){
  houghMeshLayer = m_houghMeshLayer;
}
