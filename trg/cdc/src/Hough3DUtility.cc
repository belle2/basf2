#ifndef __CINT__
#include "trg/cdc/Fitter3DUtility.h"
#include "trg/cdc/Hough3DUtility.h"
#include <cmath>
#include <iostream>
#endif



using namespace std;

Hough3DFinder::Hough3DFinder(void){
  m_mode = 1;
  m_Trg_PI = 3.141592653589793;
}

Hough3DFinder::Hough3DFinder(int mode){
  m_mode = mode;
}

Hough3DFinder::~Hough3DFinder(void){
  destruct();
}

void Hough3DFinder::initialize(TVectorD &geometryVariables, vector<float > &initVariables){
  switch (m_mode){
    case 1: 
      initVersion1(initVariables);
      break;
    default:
      break;
  }
  for(int i=0; i<4; i++){
    m_rr[i] = geometryVariables[i];
    m_anglest[i] = geometryVariables[i+4];
    m_ztostraw[i] = geometryVariables[i+8];
  }
}

void Hough3DFinder::destruct(void){
  switch (m_mode){
    case 1: 
      destVersion1();
      break;
    default:
      break;
  }
}

void Hough3DFinder::runFinder(std::vector<double> &trackVariables, vector<vector<double> > &stTSs){

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
    case 1: 
      runFinderVersion1(trackVariables, stTSs, tsArcS, tsZ);
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
  double minZ0;

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
        //cout<<"JB tsArcS: "<<tsArcS[iLayer]<<" tempCotStart: "<<tempCotStart<<" tsZ:"<<tsZ[iLayer][iTS]<<endl;
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
        minZ0 = 9999;
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

void Hough3DFinder::getValuesVersion1(const string& input, vector<double> &result ){
  //Clear result vector
  //cout<<"Before array size: "<<result.size()<<endl;
  //for(unsigned iResult=0; iResult<result.size(); iResult++) result.pop_back();
  //cout<<"After array size: "<<result.size()<<endl;
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
}


void Hough3DFinder::getHoughMeshLayer(bool ***& houghMeshLayer ){
  houghMeshLayer = m_houghMeshLayer;
}
