/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef __CINT__
#include "trg/cdc/Fitter3DUtility.h"
#include "trg/cdc/Hough3DUtility.h"
#include "trg/cdc/JSignal.h"
#include "trg/cdc/JLUT.h"
#include "trg/cdc/JSignalData.h"
#include "trg/cdc/FpgaUtility.h"
#include <cmath>
#include <iostream>
#include <tuple>
#endif



using namespace std;

Hough3DFinder::Hough3DFinder(void) :
  m_mode(0), m_nWires(), m_rr(), m_ztostraw(), m_anglest(),
  m_cotStart(0), m_cotEnd(0), m_z0Start(0), m_z0End(0),
  m_nCotSteps(0), m_nZ0Steps(0), m_cotStepSize(0), m_z0StepSize(0),
  m_houghMeshLayerDiff(0), m_houghMeshLayer(0), m_houghMesh(0), m_houghMeshDiff(0),
  m_hitMap(0), m_driftMap(0), m_geoCandidatesIndex(0), m_geoCandidatesPhi(0),
  m_geoCandidatesDiffStWires(0), m_stAxPhi(), m_bestCot(0), m_bestZ0(0),
  m_houghMax(0), m_minDiffHough(0), m_foundZ(), m_foundPhiSt(), m_bestTSIndex(),
  m_bestTS(), m_inputFileName("GeoFinder.input"), m_findRhoMax(0), m_findRhoMin(0),
  m_findRhoIntMax(0), m_findRhoIntMin(0),
  m_findPhi0Max(0), m_findPhi0Min(0), m_findPhi0IntMax(0), m_findPhi0IntMin(0),
  m_findArcCosMax(0), m_findArcCosMin(0), m_findArcCosIntMax(0), m_findArcCosIntMin(0),
  m_findPhiZMax(0), m_findPhiZMin(0), m_findPhiZIntMax(0), m_findPhiZIntMin(0),
  m_rhoMax(0), m_rhoMin(0), m_rhoBit(0), m_phi0Max(0), m_phi0Min(0), m_phi0Bit(0),
  m_stAxWireFactor(0), m_LUT(0),
  m_arcCosLUT(0), m_wireConvertLUT(0),
  m_commonData(0), m_outputVhdlDirname("./VHDL/finder3D")
{
  m_mode = 2;
  m_Trg_PI = 3.141592653589793;
  m_outputLutDirname = m_outputVhdlDirname + "/" + "LutData";
  // Make driftMap
  m_driftMap = new int* [4];
  for (int iSt = 0; iSt < 4; iSt++) {
    m_driftMap[iSt] = new int[m_nWires[iSt] / 2];
    for (int iTS = 0; iTS < m_nWires[iSt] / 2; iTS++) {
      m_driftMap[iSt][iTS] = 0;
    }
  }
}

Hough3DFinder::~Hough3DFinder(void)
{
  //destruct();
}

void Hough3DFinder::setMode(int mode)
{
  m_mode = mode;
}

int Hough3DFinder::getMode(void)
{
  return m_mode;
}

void Hough3DFinder::initialize(const TVectorD& geometryVariables, vector<float >& initVariables)
{

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

  for (int iLayer = 0; iLayer < 4; iLayer++) {
    m_bestTS[iLayer] = 999;
    m_bestTSIndex[iLayer] = 999;
  }
  for (int i = 0; i < 4; i++) {
    m_rr[i] = geometryVariables[i];
    m_anglest[i] = geometryVariables[i + 4];
    m_ztostraw[i] = geometryVariables[i + 8];
    m_nWires[i] = (int)geometryVariables[i + 12];
  }
  switch (m_mode) {
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
      cout << "[Error] 3DFinder mode is not correct. Current mode is " << m_mode << "." << endl;
      break;
  }
}

void Hough3DFinder::destruct(void)
{
  switch (m_mode) {
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

void Hough3DFinder::runFinder(const std::vector<double>& trackVariables, vector<vector<double> >& stTSs,
                              const vector<vector<int> >& stTSDrift)
{

  // Clear best TS
  for (int iLayer = 0; iLayer < 4; iLayer++) {
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
  for (unsigned i = 0; i < 4; i++) {
    tsArcS.push_back(Fitter3DUtility::calS(rho, m_rr[i]));
    tsZ.push_back(vector<double>());
    for (unsigned j = 0; j < stTSs[i].size(); j++) {
      //fitPhi0 = mcPhi0;
      tsZ[i].push_back(Fitter3DUtility::calZ(charge, m_anglest[i], m_ztostraw[i], m_rr[i], stTSs[i][j], rho, fitPhi0));
    }
  }

  switch (m_mode) {
    // Hough 3D finder
    case 1:
      runFinderVersion1(trackVariables, stTSs, tsArcS, tsZ);
      break;
    // Geo Finder
    case 2:
      runFinderVersion2(trackVariables, stTSs, stTSDrift);
      break;
    // FPGA Geo Finder (For LUT generator. Doesn't use LUTs)
    case 3:
      runFinderVersion3(trackVariables, stTSs, stTSDrift);
      break;
    default:
      break;
  }

}

void Hough3DFinder::initVersion1(const vector<float >& initVariables)
{
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
  m_cotStepSize = m_cotEnd / ((m_nCotSteps - 1) / 2);
  m_z0StepSize = m_z0End / ((m_nZ0Steps - 1) / 2);
  // HoughMesh
  m_houghMeshLayerDiff = new float** [m_nCotSteps];
  m_houghMeshLayer = new bool** [m_nCotSteps];
  m_houghMesh = new int* [m_nCotSteps];
  m_houghMeshDiff = new float*[m_nCotSteps];
  for (int i = 0; i < m_nCotSteps; i++) {
    m_houghMeshLayerDiff[i] = new float*[m_nZ0Steps];
    m_houghMeshLayer[i] = new bool*[m_nZ0Steps];
    m_houghMesh[i] = new int[m_nZ0Steps];
    m_houghMeshDiff[i] = new float[m_nZ0Steps];
    for (int j = 0; j < m_nZ0Steps; j++) {
      m_houghMeshLayerDiff[i][j] = new float[4];
      m_houghMeshLayer[i][j] = new bool[4];
    }
  }
}

void Hough3DFinder::initVersion2(vector<float >& initVariables)
{
  if (false) cout << initVariables.size() << endl; // Removes warning when compiling

  // index values of candidates.
  m_geoCandidatesIndex = new vector<vector<int > >;
  for (int iLayer = 0; iLayer < 4; iLayer++) m_geoCandidatesIndex->push_back(vector<int> ());
  // phi values of candidates.
  m_geoCandidatesPhi = new vector<vector<double > >;
  for (int iLayer = 0; iLayer < 4; iLayer++) m_geoCandidatesPhi->push_back(vector<double> ());
  // diffStWire values of candidates.
  m_geoCandidatesDiffStWires = new vector<vector<double > >;
  for (int iLayer = 0; iLayer < 4; iLayer++) m_geoCandidatesDiffStWires->push_back(vector<double> ());
}

void Hough3DFinder::initVersion3(vector<float >& initVariables)
{
  if (false) cout << initVariables.size() << endl; // Removes warning when compiling

  // Make hitMap
  m_hitMap = new bool*[4];
  for (int i = 0; i < 4; i++) {
    m_hitMap[i] = new bool[m_nWires[i] / 2];
    for (int j = 0; j < m_nWires[i] / 2; j++) {
      m_hitMap[i][j] = 0;
    }
  }
  // Make driftMap
  m_driftMap = new int* [4];
  for (int iSt = 0; iSt < 4; iSt++) {
    m_driftMap[iSt] = new int[m_nWires[iSt] / 2];
    for (int iTS = 0; iTS < m_nWires[iSt] / 2; iTS++) {
      m_driftMap[iSt][iTS] = 0;
    }
  }

  // index values of candidates.
  m_geoCandidatesIndex = new vector<vector<int > >;
  for (int iLayer = 0; iLayer < 4; iLayer++) m_geoCandidatesIndex->push_back(vector<int> ());
  // phi values of candidates.
  m_geoCandidatesPhi = new vector<vector<double > >;
  for (int iLayer = 0; iLayer < 4; iLayer++) m_geoCandidatesPhi->push_back(vector<double> ());
  // diffStWire values of candidates.
  m_geoCandidatesDiffStWires = new vector<vector<double > >;
  for (int iLayer = 0; iLayer < 4; iLayer++) m_geoCandidatesDiffStWires->push_back(vector<double> ());

  // [TODO] Add to class. Add getters and setters.
  m_mBool["fVHDLFile"] = 0;
  m_mBool["fVerbose"] = 0;
}

void Hough3DFinder::setInputFileName(const string& inputFileName)
{
  m_inputFileName = inputFileName;
}

void Hough3DFinder::destVersion1()
{
  // Deallocate HoughMesh
  for (int i = 0; i < m_nCotSteps; i++) {
    for (int j = 0; j < m_nZ0Steps; j++) {
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

void Hough3DFinder::destVersion2()
{

  delete m_geoCandidatesIndex;
  delete m_geoCandidatesPhi;
  delete m_geoCandidatesDiffStWires;
  //for(int iLayer=0; iLayer<4; iLayer++) {
  //  delete m_geoCandidatesIndex;
  //  delete m_geoCandidatesPhi;
  //  delete m_geoCandidatesDiffStWires;
  //}
}

void Hough3DFinder::destVersion3()
{

  for (int i = 0; i < 4; i++) {
    delete [] m_hitMap[i];
  }
  delete [] m_hitMap;
  for (int iSt = 0; iSt < 4; iSt++) {
    delete [] m_driftMap[iSt];
  }
  delete [] m_driftMap;

  if (m_mBool["fVHDLFile"]) {
    if (m_mSavedIoSignals.size() != 0) {
      FpgaUtility::multipleWriteCoe(10, m_mSavedIoSignals, m_outputLutDirname + "/");
    }
    if (m_mSavedSignals.size() != 0) {
      FpgaUtility::writeSignals(m_outputVhdlDirname + "/signals", m_mSavedSignals);
    }
    //if(m_mSavedIoSignals.size()!=0) FpgaUtility::multipleWriteCoe(10, m_mSavedIoSignals, "./");
    //if(m_mSavedSignals.size()!=0) FpgaUtility::writeSignals("signalValues",m_mSavedSignals);
  }

  if (m_commonData) delete m_commonData;

}

// Hough 3D finder
void Hough3DFinder::runFinderVersion1(const vector<double>& trackVariables, const vector<vector<double> >& stTSs,
                                      const vector<double>& tsArcS,
                                      const vector<vector<double> >& tsZ)
{

  int charge = (int)trackVariables[0];
  double rho = trackVariables[1];
  double fitPhi0 = trackVariables[2];

  // Clear Hough Meshes.
  for (int i = 0; i < m_nCotSteps; i++) {
    for (int j = 0; j < m_nZ0Steps; j++) {
      for (int k = 0; k < 4; k++) {
        m_houghMeshLayerDiff[i][j][k] = 999;
        m_houghMeshLayer[i][j][k] = 0;
      }
      m_houghMesh[i][j] = 0;
      m_houghMeshDiff[i][j] = 0.;
    }
  }

  // Fill Hough mesh.
  double tempZ0Start, tempZ0End;
  double tempZ01, tempZ02;
  int tempHoughZ0;
  double actualCot, actualZ0;
  // Find best vote.
  //double minZ0;

  // Vote in Hough Mesh Layers.
  for (int cotStep = 0; cotStep < m_nCotSteps; cotStep++) {
    // Find cotStep range for mesh.
    double tempCotStart = (cotStep - 0.5) * m_cotStepSize + m_cotStart;
    double tempCotEnd = (cotStep + 0.5) * m_cotStepSize + m_cotStart;
    //cout<<"tempCotStart: "<<tempCotStart<<" tempCotEnd: "<<tempCotEnd<<endl;

    // Find z0 range for mesh per layer.
    for (unsigned iLayer = 0; iLayer < 4; iLayer++) {
      for (unsigned iTS = 0; iTS < stTSs[iLayer].size(); iTS++) {
        // Find z0 for cot.
        tempZ01 = -tsArcS[iLayer] * tempCotStart + tsZ[iLayer][iTS];
        tempZ02 = -tsArcS[iLayer] * tempCotEnd + tsZ[iLayer][iTS];

        // Find start and end of range z0.
        if (tempZ01 < tempZ02) {
          tempZ0Start = tempZ01;
          tempZ0End = tempZ02;
        } else {
          tempZ0Start = tempZ02;
          tempZ0End = tempZ01;
        }
        //cout<<"z0Start: "<<tempZ0Start<<endl;
        //cout<<"z0End: "<<tempZ0End<<endl;

        // Do proper rounding for plus and minus values.
        if (tempZ0Start > 0) {
          tempZ0Start = int(tempZ0Start / m_z0StepSize + 0.5);
        } else {
          tempZ0Start = int(tempZ0Start / m_z0StepSize - 0.5);
        }
        if (tempZ0End > 0) {
          tempZ0End = int(tempZ0End / m_z0StepSize + 0.5);
        } else {
          tempZ0End = int(tempZ0End / m_z0StepSize - 0.5);
        }

        // To save time do z0 cut off here
        if (tempZ0Start < -(m_nZ0Steps - 1) / 2) {
          tempZ0Start = -(m_nZ0Steps - 1) / 2;
        }
        if (tempZ0End > (m_nZ0Steps - 1) / 2) {
          tempZ0End = (m_nZ0Steps - 1) / 2;
        }

        // Fill Hough Mesh.
        for (int z0Step = int(tempZ0Start); z0Step <= int(tempZ0End); z0Step++) {
          // Cut off if z0Step is bigger or smaller than z0 limit.
          // Not needed anymore.
          if (z0Step > (m_nZ0Steps - 1) / 2 || z0Step < -(m_nZ0Steps - 1) / 2) {
            cout << "cutoff because z0step is bigger or smaller than z0 limit ";
            continue;
          }

          // Change temHoughZ0 if minus.
          if (z0Step < 0) {
            tempHoughZ0 = (m_nZ0Steps - 1) / 2 - z0Step;
          } else { tempHoughZ0 = z0Step; }

          //Change to actual value.
          actualCot = cotStep * m_cotStepSize + m_cotStart;
          actualZ0 = z0Step * m_z0StepSize;
          //cout<<"actualCot: "<<actualCot<<" actualZ0: "<<actualZ0<<endl;


          m_houghMeshLayer[cotStep][tempHoughZ0][iLayer] = 1;
          // Find minimum z difference for the vote.
          m_minDiffHough = abs(actualCot * tsArcS[iLayer] + actualZ0 - tsZ[iLayer][iTS]);
          if (m_houghMeshLayerDiff[cotStep][tempHoughZ0][iLayer] > m_minDiffHough) {
            m_houghMeshLayerDiff[cotStep][tempHoughZ0][iLayer] = m_minDiffHough;
          }

        } // End of z0 vote loop.
      } // End of TS loop.
    }  // End of layer loop.
  } // End of cot vote loop.

  // Filling HoughMesh. Combines the seperate HoughMeshLayers.
  for (int houghCot = 0; houghCot < m_nCotSteps; houghCot++) {
    for (int houghZ0 = 0; houghZ0 < m_nZ0Steps; houghZ0++) {
      //Change back tempHoughZ0 if minus
      if (houghZ0 > (m_nZ0Steps - 1) / 2) {
        tempHoughZ0 = (m_nZ0Steps - 1) / 2 - houghZ0;
      } else {
        tempHoughZ0 = houghZ0;
      }
      //Change to actual value
      actualCot = houghCot * m_cotStepSize + m_cotStart;
      actualZ0 = tempHoughZ0 * m_z0StepSize;
      // To remove warning of actualCot and actualZ0.
      if (false) cout << actualCot << actualZ0 << endl;

      for (int layer = 0; layer < 4; layer++) {
        m_houghMesh[houghCot][houghZ0] += m_houghMeshLayer[houghCot][houghZ0][layer];
        if (m_houghMeshLayerDiff[houghCot][houghZ0][layer] != 999) m_houghMeshDiff[houghCot][houghZ0] +=
            m_houghMeshLayerDiff[houghCot][houghZ0][layer];
        //if(houghMeshLayer[houghCot][houghZ0][layer]==1) hhough00->Fill(actualCot,actualZ0);
      } // End of combining votes
    } // End loop for houghZ0
  } // End loop for houghCot

  // Find best vote. By finding highest votes and comparing all votes and pick minimum diff z.
  m_houghMax = 0;
  for (int houghCot = 0; houghCot < m_nCotSteps; houghCot++) {
    for (int houghZ0 = 0; houghZ0 < m_nZ0Steps; houghZ0++) {
      // Changes back values for minus
      if (houghZ0 > (m_nZ0Steps - 1) / 2) {
        tempHoughZ0 = (m_nZ0Steps - 1) / 2 - houghZ0;
      } else { tempHoughZ0 = houghZ0;}
      // Find highest vote
      if (m_houghMax < m_houghMesh[houghCot][houghZ0]) {
        m_houghMax = m_houghMesh[houghCot][houghZ0];
        // If highest vote changes, need to initialize minZ0, minDiffHough
        //minZ0 = 9999;
        m_minDiffHough = 9999;
      }
      // When highest vote
      if (m_houghMax == m_houghMesh[houghCot][houghZ0]) {
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
        if (m_minDiffHough > m_houghMeshDiff[houghCot][houghZ0]) {
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
  for (int i = 0; i < 4; i++) {
    m_foundZ[i] = (m_bestCot * m_cotStepSize + m_cotStart) * tsArcS[i] + (m_bestZ0 * m_z0StepSize);
    m_foundPhiSt[i] = fitPhi0 + charge * acos(m_rr[i] / 2 / rho) + 2 * asin((m_ztostraw[i] - m_foundZ[i]) * tan(
                        m_anglest[i]) / 2 / m_rr[i]);
    if (m_foundPhiSt[i] > 2 * m_Trg_PI) m_foundPhiSt[i] -= 2 * m_Trg_PI;
    if (m_foundPhiSt[i] < 0) m_foundPhiSt[i] += 2 * m_Trg_PI;
  }
  //cout<<"JB FoundPhiSt[0]: "<<m_foundPhiSt[0]<<" FoundPhiSt[1]: "<<m_foundPhiSt[1]<<" FoundPhiSt[2]: "<<m_foundPhiSt[2]<<" FoundPhiSt[3]: "<<m_foundPhiSt[3]<<endl;

  // Find closest phi out of canidates
  double minDiff[4] = {999, 999, 999, 999};
  for (unsigned iLayer = 0; iLayer < 4; iLayer++) {
    for (unsigned iTS = 0; iTS < stTSs[iLayer].size(); iTS++) {
      if (minDiff[iLayer] > abs(m_foundPhiSt[iLayer] - stTSs[iLayer][iTS])) {
        minDiff[iLayer] = abs(m_foundPhiSt[iLayer] - stTSs[iLayer][iTS]);
        m_bestTS[iLayer] = stTSs[iLayer][iTS];
        m_bestTSIndex[iLayer] = (int)iTS;
      }
    }
  }
  //cout<<"JB BestPhiSt[0]: "<<m_bestTS[0]<<" BestPhiSt[1]: "<<m_bestTS[1]<<" BestPhiSt[2]: "<<m_bestTS[2]<<" BestPhiSt[3]: "<<m_bestTS[3]<<endl;

}

void Hough3DFinder::runFinderVersion2(const vector<double>& trackVariables, vector<vector<double> >& stTSs,
                                      const std::vector<std::vector<int> >& stTSDrift)
{

  // Clear m_geoCandidatesIndex
  for (int iLayer = 0; iLayer < 4; iLayer++) {
    (*m_geoCandidatesIndex)[iLayer].clear();
    (*m_geoCandidatesPhi)[iLayer].clear();
    (*m_geoCandidatesDiffStWires)[iLayer].clear();
  }

  int charge = (int)trackVariables[0];
  double rho = trackVariables[1];
  double fitPhi0 = trackVariables[2];
  double tsDiffSt;

  //cout<<"charge:"<<charge<<" rho:"<<rho<<" fitPhi0:"<<fitPhi0<<endl;
  for (int iLayer = 0; iLayer < 4; iLayer++) {
    m_stAxPhi[iLayer] = Fitter3DUtility::calStAxPhi(charge, m_anglest[iLayer], m_ztostraw[iLayer],  m_rr[iLayer], rho, fitPhi0);
    //cout<<"iSt:"<<iLayer<<" ztostraw:"<<m_ztostraw[iLayer]<<" m_rr:"<<m_rr[iLayer]<<" stAxPhi:"<<m_stAxPhi[iLayer]<<endl;
    //if(stTSs[iLayer].size()==0) cout<<"stTSs["<<iLayer<<"] is zero"<<endl;
    for (unsigned iTS = 0; iTS < stTSs[iLayer].size(); iTS++) {
      //int t_tdc = (stTSDrift[iLayer][iTS] >> 4);
      //int t_lr = ((stTSDrift[iLayer][iTS] >> 2) & 3);
      int t_priorityPosition = (stTSDrift[iLayer][iTS] & 3);
      //int t_tsId = int(stTSs[iLayer][iTS]*m_nWires[iLayer]/2/2/m_Trg_PI+0.5);
      //cout<<"iSt:"<<iLayer<<" tsId:"<<t_tsId<<" pp:"<<t_priorityPosition<<endl;
      // Reject second priority TSs.
      if (t_priorityPosition != 3) continue;
      // Find number of wire difference
      tsDiffSt = m_stAxPhi[iLayer] - stTSs[iLayer][iTS];
      if (tsDiffSt > m_Trg_PI) tsDiffSt -= 2 * m_Trg_PI;
      if (tsDiffSt < -m_Trg_PI) tsDiffSt += 2 * m_Trg_PI;
      tsDiffSt = tsDiffSt / 2 / m_Trg_PI * m_nWires[iLayer] / 2;
      //cout<<"JB ["<<iLayer<<"]["<<iTS<<"] tsDiffSt: "<<tsDiffSt<<" stTSs:"<<stTSs[iLayer][iTS]<<" rho: "<<rho<<" phi0: "<<fitPhi0<<" m_stAxPhi:"<<m_stAxPhi[iLayer]<<endl;
      // Save index if condition is in 10 wires
      if (iLayer % 2 == 0) {
        if (tsDiffSt > 0 && tsDiffSt <= 10) {
          (*m_geoCandidatesIndex)[iLayer].push_back(iTS);
          (*m_geoCandidatesPhi)[iLayer].push_back(stTSs[iLayer][iTS]);
          (*m_geoCandidatesDiffStWires)[iLayer].push_back(tsDiffSt);
        }
      } else {
        if (tsDiffSt < 0 && tsDiffSt >= -10) {
          (*m_geoCandidatesIndex)[iLayer].push_back(iTS);
          (*m_geoCandidatesPhi)[iLayer].push_back(stTSs[iLayer][iTS]);
          (*m_geoCandidatesDiffStWires)[iLayer].push_back(tsDiffSt);
        }
      } // End of saving index

    } // Candidate loop
  } // Layer loop

  //// Print all candidates.
  //for( int iLayer=0; iLayer<4; iLayer++){
  //  cout<<"[geoFinder] iSt:"<<iLayer<<" nSegments:"<<(*m_geoCandidatesIndex)[iLayer].size()<<endl;
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
  for (int iLayer = 0; iLayer < 4; iLayer++) {
    if ((*m_geoCandidatesIndex)[iLayer].size() == 0) {
      //cout<<"No St Candidate in GeoFinder"<<endl;
      m_bestTS[iLayer] = 999;
      m_bestTSIndex[iLayer] = 999;
    } else {
      double bestDiff = 999;
      for (int iTS = 0; iTS < int((*m_geoCandidatesIndex)[iLayer].size()); iTS++) {
        tsDiffSt = m_stAxPhi[iLayer] - stTSs[iLayer][(*m_geoCandidatesIndex)[iLayer][iTS]];
        if (tsDiffSt > m_Trg_PI) tsDiffSt -= 2 * m_Trg_PI;
        if (tsDiffSt < -m_Trg_PI) tsDiffSt += 2 * m_Trg_PI;
        tsDiffSt = tsDiffSt / 2 / m_Trg_PI * m_nWires[iLayer] / 2;
        // Pick the better TS
        //if(abs(abs(tsDiffSt)-5) < bestDiff){
        if (abs(abs(tsDiffSt) - meanWireDiff[iLayer]) < bestDiff) {
          //bestDiff = abs(abs(tsDiffSt)-5);
          bestDiff = abs(abs(tsDiffSt) - meanWireDiff[iLayer]);
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

void Hough3DFinder::runFinderVersion3(const vector<double>& trackVariables, vector<vector<double> >& stTSs,
                                      const vector<vector<int> >& stTSDrift)
{

  int m_verboseFlag = m_mBool["fVerbose"];

  if (m_verboseFlag) cout << "####geoFinder start####" << endl;

  // Clear m_geoCandidatesIndex
  for (int iLayer = 0; iLayer < 4; iLayer++) {
    (*m_geoCandidatesPhi)[iLayer].clear();
    (*m_geoCandidatesIndex)[iLayer].clear();
    (*m_geoCandidatesDiffStWires)[iLayer].clear();
  }
  // Clear hit map for track
  for (int iLayer = 0; iLayer < 4; iLayer++) {
    for (int iTS = 0; iTS < m_nWires[iLayer] / 2; iTS++) {
      m_hitMap[iLayer][iTS] = 0;
    }
  }
  // Clear drift map for track
  for (int iSt = 0; iSt < 4; iSt++) {
    for (int iTS = 0; iTS < m_nWires[iSt] / 2; iTS++) {
      m_driftMap[iSt][iTS] = 0;
    }
  }

  // 2D Track Variables
  int charge = (int)trackVariables[0];
  double rho = trackVariables[1];
  double fitPhi0 = trackVariables[2];

  // Fill hitMap and driftMap
  int iHitTS;
  int driftInfo;
  for (unsigned iLayer = 0; iLayer < 4; iLayer++) {
    //cout<<"["<<iLayer<<"] size:"<<stTSs[iLayer].size()<<endl;
    for (unsigned iTS = 0; iTS < stTSs[iLayer].size(); iTS++) {
      iHitTS = int(stTSs[iLayer][iTS] * m_nWires[iLayer] / 2 / 2 / m_Trg_PI + 0.5);
      driftInfo = stTSDrift[iLayer][iTS];
      if (m_verboseFlag) cout << "[" << iLayer << "] TSId: " << iHitTS << " stTSs: " << stTSs[iLayer][iTS] << " driftInfo:" << driftInfo
                                << " priorityPosition:" << (driftInfo & 3) << endl;
      //cout<<iHitTS*2*m_Trg_PI/m_nWires[iLayer]*2<<endl;
      m_hitMap[iLayer][iHitTS] = 1;
      m_driftMap[iLayer][iHitTS] = driftInfo;
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
  //// Print driftMap
  //for(int iLayer=0; iLayer<4; iLayer++) {
  //  cout<<"["<<iLayer<<"]["<<m_nWires[iLayer]/2<<"]"<<endl;;
  //  for(int iTS=m_nWires[iLayer]/2-1; iTS>=0; iTS--) {
  //    cout<<"    ["<<iTS<<"]: "<<m_driftMap[iLayer][iTS]<<endl;;
  //  }
  //  cout<<endl;
  //} // End layer

  //// Temp geoFinder
  //for( int iLayer=0; iLayer<4; iLayer++) {
  //  double t_phiAx = acos(m_rr[iLayer]/(2*rho));
  //  double t_dPhiAx = 0;
  //  double t_dPhiAx_c = 0;
  //  if(charge==1) t_dPhiAx = t_phiAx+fitPhi0;
  //  else t_dPhiAx = -t_phiAx+fitPhi0;
  //  if(t_dPhiAx < 0) t_dPhiAx_c = t_dPhiAx + 2* m_Trg_PI;
  //  else if (t_dPhiAx > 2*m_Trg_PI) t_dPhiAx_c = t_dPhiAx - 2*m_Trg_PI;
  //  else t_dPhiAx_c = t_dPhiAx;
  //  double t_dPhiAxWire = 0;
  //  if( iLayer%2 == 0 ) t_dPhiAxWire = int(t_dPhiAx_c/2/m_Trg_PI*m_nWires[iLayer]/2);
  //  else t_dPhiAxWire = int(t_dPhiAx_c/2/m_Trg_PI*m_nWires[iLayer]/2 + 1);
  //  cout<<"iSt:"<<iLayer<<" phiAx:"<<t_phiAx<<" phi0:"<<fitPhi0<<" dPhiAx:"<<t_dPhiAx<<" charge:"<<charge<<endl;
  //  cout<<"      dPhiAx_c:"<<t_dPhiAx_c<<" dPhiAxWire:"<<t_dPhiAxWire<<endl;
  //}

  // Will use cm unit.
  std::map<std::string, std::vector<double> > mConstV;
  std::map<std::string, double > mConstD;
  std::map<std::string, double > mDouble;

  if (m_commonData == 0) {
    m_commonData = new Belle2::TRGCDCJSignalData();
  }
  // Setup firmware parameters
  double phiMax = m_Trg_PI;
  double phiMin = -m_Trg_PI;
  int phiBitSize = 13;
  // pt = 0.3*1.5*rho*0.01;
  /* cppcheck-suppress variableScope */
  double rhoMin = 20;
  double rhoMax = 2500;
  int rhoBitSize = 11;

  // Constraints used in below code.
  mConstV["rr"] = vector<double> (9);
  mConstV["rr3D"] = vector<double> (4);
  mConstV["nTSs"] = vector<double> (9);
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    // Convert unit to cm.
    mConstV["rr"][2 * iSt + 1] = m_rr[iSt] * 100;
    mConstV["rr3D"][iSt] = m_rr[iSt] * 100;
    mConstV["nTSs"][2 * iSt + 1] = m_nWires[iSt] / 2;
  }
  mConstD["acosLUTInBitSize"] = rhoBitSize;
  mConstD["acosLUTOutBitSize"] = phiBitSize - 1;
  mConstD["Trg_PI"] = m_Trg_PI;

  // Constrain rho to rhoMax. For removing warnings when changing to signals.
  {
    mDouble["rho"] = rho * 100;
    if (mDouble["rho"] > rhoMax) {
      mDouble["rho"] = rhoMax;
      mDouble["pt"] = rhoMax * 0.3 * 1.5 * 0.01;
    }
  }
  // Constrain phi0 to [-pi, pi]
  {
    mDouble["phi0"] = fitPhi0;
    bool rangeFail = 1;
    while (rangeFail) {
      if (mDouble["phi0"] > mConstD["Trg_PI"]) mDouble["phi0"] -= 2 * mConstD["Trg_PI"];
      else if (mDouble["phi0"] < -mConstD["Trg_PI"]) mDouble["phi0"] += 2 * mConstD["Trg_PI"];
      else rangeFail = 0;
    }
  }

  // Change to Signals. Convert rho to cm unit.
  {
    vector<tuple<string, double, int, double, double, int> > t_values = {
      make_tuple("phi0", mDouble["phi0"], phiBitSize, phiMin, phiMax, 0),
      make_tuple("rho", mDouble["rho"], rhoBitSize, rhoMin, rhoMax, 0),
      make_tuple("charge", (int)(charge == 1 ? 1 : 0), 1, 0, 1.5, 0),
    };
    Belle2::TRGCDCJSignal::valuesToMapSignals(t_values, m_commonData, m_mSignalStorage);
  }
  //cout<<"<<<phi0>>>"<<endl; m_mSignalStorage["phi0"].dump();
  //cout<<"<<<rho>>>"<<endl; m_mSignalStorage["rho"].dump();
  //cout<<"<<<charge>>>"<<endl; m_mSignalStorage["charge"].dump();

  // Constrain rho
  Fitter3DUtility::constrainRPerStSl(mConstV, m_mSignalStorage);
  // phiAx = arcos(r/2R).
  // Make min max constants for lut.
  if (m_mSignalStorage.find("invPhiAxMin_0") == m_mSignalStorage.end()) {
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      string t_invMinName = "invPhiAxMin_" + to_string(iSt);
      double t_actual = m_mSignalStorage["rho_c_" + to_string(iSt)].getMinActual();
      double t_toReal = m_mSignalStorage["rho_c_" + to_string(iSt)].getToReal();
      m_mSignalStorage[t_invMinName] = Belle2::TRGCDCJSignal(t_actual, t_toReal, m_commonData);
      string t_invMaxName = "invPhiAxMax_" + to_string(iSt);
      t_actual = m_mSignalStorage["rho_c_" + to_string(iSt)].getMaxActual();
      m_mSignalStorage[t_invMaxName] = Belle2::TRGCDCJSignal(t_actual, t_toReal, m_commonData);
    }
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<invPhiAxMin_"<<iSt<<">>>"<<endl; m_mSignalStorage["invPhiAxMin_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<invPhiAxMax_"<<iSt<<">>>"<<endl; m_mSignalStorage["invPhiAxMax_"+to_string(iSt)].dump();}
  // Generate LUT(phiAx[i]=acos(rr[i]/2/rho)).
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    string t_valueName = "phiAx_" + to_string(iSt);
    string t_minName = "phiAxMin_" + to_string(iSt);
    string t_maxName = "phiAxMax_" + to_string(iSt);
    string t_invMinName = "invPhiAxMin_" + to_string(iSt);
    string t_invMaxName = "invPhiAxMax_" + to_string(iSt);
    if (!m_mLutStorage.count(t_valueName)) {
      m_mLutStorage[t_valueName] = new Belle2::TRGCDCJLUT(t_valueName);
      // Lambda can not copy maps.
      double t_parameter = mConstV.at("rr3D")[iSt];
      m_mLutStorage[t_valueName]->setFloatFunction(
        [ = ](double aValue) -> double{return acos(t_parameter / 2 / aValue);},
        m_mSignalStorage["rho_c_" + to_string(iSt)],
        m_mSignalStorage[t_invMinName], m_mSignalStorage[t_invMaxName], m_mSignalStorage["phi0"].getToReal(),
        (int)mConstD.at("acosLUTInBitSize"), (int)mConstD.at("acosLUTOutBitSize"));
      //m_mLutStorage[t_valueName]->makeCOE("./LutData/geoFinder_"+t_valueName+".coe");
    }
  }
  // phiAx[i] = acos(rr[i]/2/rho).
  // Operate using LUT(phiAx[i]).
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    // Set output name
    string t_valueName =  "phiAx_" + to_string(iSt);
    m_mLutStorage[t_valueName]->operate(m_mSignalStorage["rho_c_" + to_string(iSt)], m_mSignalStorage[t_valueName]);
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<phiAx_"<<iSt<<">>>"<<endl; m_mSignalStorage["phiAx_"+to_string(iSt)].dump();}
  // dPhiAx[i] = +-phiAx[i] + phi0
  {
    // Create data for ifElse
    vector<pair<Belle2::TRGCDCJSignal, vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > > > t_data;
    // Compare
    Belle2::TRGCDCJSignal t_compare = Belle2::TRGCDCJSignal::comp(Belle2::TRGCDCJSignal(1, m_mSignalStorage["charge"].getToReal(),
                                      m_commonData), "=", m_mSignalStorage["charge"]);
    // Assignments
    vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > t_assigns = {
      make_pair(&m_mSignalStorage["dPhiAx_0"], m_mSignalStorage["phiAx_0"] + m_mSignalStorage["phi0"]),
      make_pair(&m_mSignalStorage["dPhiAx_1"], m_mSignalStorage["phiAx_1"] + m_mSignalStorage["phi0"]),
      make_pair(&m_mSignalStorage["dPhiAx_2"], m_mSignalStorage["phiAx_2"] + m_mSignalStorage["phi0"]),
      make_pair(&m_mSignalStorage["dPhiAx_3"], m_mSignalStorage["phiAx_3"] + m_mSignalStorage["phi0"])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare
    t_compare = Belle2::TRGCDCJSignal();
    // Assignments
    t_assigns = {
      make_pair(&m_mSignalStorage["dPhiAx_0"], -m_mSignalStorage["phiAx_0"] + m_mSignalStorage["phi0"]),
      make_pair(&m_mSignalStorage["dPhiAx_1"], -m_mSignalStorage["phiAx_1"] + m_mSignalStorage["phi0"]),
      make_pair(&m_mSignalStorage["dPhiAx_2"], -m_mSignalStorage["phiAx_2"] + m_mSignalStorage["phi0"]),
      make_pair(&m_mSignalStorage["dPhiAx_3"], -m_mSignalStorage["phiAx_3"] + m_mSignalStorage["phi0"])
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Process ifElse data.
    Belle2::TRGCDCJSignal::ifElse(t_data);
  }
  //cout<<"<<<phi0>>>"<<endl; m_mSignalStorage["phi0"].dump();
  //cout<<"<<<charge>>>"<<endl; m_mSignalStorage["charge"].dump();
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<dPhiAx_"<<iSt<<">>>"<<endl; m_mSignalStorage["dPhiAx_"+to_string(iSt)].dump();}

  // This could be needed in the future if efficiency is low.
  // To use this code, change dPhiAx to dPhiAx_m when constraining dPhiAx below.
  //// Add two wire unit(rad) to widen start of window.
  //for(unsigned iSt=0; iSt<4; iSt++) {
  //  double wirePhiUnit = 2 * mConstD["Trg_PI"] / mConstV["nTSs"][2*iSt+1];
  //  if (iSt%2 == 0) {
  //    m_mSignalStorage["dPhiAx_m_"+to_string(iSt)] <= m_mSignalStorage["dPhiAx_"+to_string(iSt)] + Belle2::TRGCDCJSignal(wirePhiUnit,m_mSignalStorage["dPhiAx_"+to_string(iSt)].getToReal(), m_commonData);
  //  } else {
  //    m_mSignalStorage["dPhiAx_m_"+to_string(iSt)] <= m_mSignalStorage["dPhiAx_"+to_string(iSt)] - Belle2::TRGCDCJSignal(wirePhiUnit, m_mSignalStorage["dPhiAx_"+to_string(iSt)].getToReal(), m_commonData);
  //  }
  //}

  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<dPhiAx_m_"<<iSt<<">>>"<<endl; m_mSignalStorage["dPhiAx_m_"+to_string(iSt)].dump();}
  // Constrain dPhiAx[i] to [0, 2pi)
  if (m_mSignalStorage.find("dPhiAxMax_0") == m_mSignalStorage.end()) {
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      string t_valueName = "dPhiAx_" + to_string(iSt);
      //string t_valueName = "dPhiAx_m_" + to_string(iSt);
      string t_maxName = "dPhiAxMax_" + to_string(iSt);
      string t_minName = "dPhiAxMin_" + to_string(iSt);
      string t_2PiName = "dPhiAx2Pi_" + to_string(iSt);
      m_mSignalStorage[t_maxName] = Belle2::TRGCDCJSignal(2 * mConstD.at("Trg_PI"), m_mSignalStorage[t_valueName].getToReal(),
                                                          m_commonData);
      m_mSignalStorage[t_minName] = Belle2::TRGCDCJSignal(0, m_mSignalStorage[t_valueName].getToReal(), m_commonData);
      m_mSignalStorage[t_2PiName] = Belle2::TRGCDCJSignal(2 * mConstD.at("Trg_PI"), m_mSignalStorage[t_valueName].getToReal(),
                                                          m_commonData);
    }
  }
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    string t_in1Name = "dPhiAx_" + to_string(iSt);
    //string t_in1Name = "dPhiAx_m_" + to_string(iSt);
    string t_valueName = "dPhiAx_c_" + to_string(iSt);
    string t_maxName = "dPhiAxMax_" + to_string(iSt);
    string t_minName = "dPhiAxMin_" + to_string(iSt);
    string t_2PiName = "dPhiAx2Pi_" + to_string(iSt);
    // Create data for ifElse
    vector<pair<Belle2::TRGCDCJSignal, vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > > > t_data;
    // Compare (dPhiAx_m >= 2*pi)
    Belle2::TRGCDCJSignal t_compare = Belle2::TRGCDCJSignal::comp(m_mSignalStorage[t_in1Name], ">=", m_mSignalStorage[t_maxName]);
    // Assignments (dPhiAx_c <= dPhiAx_m - 2*pi)
    vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > t_assigns = {
      make_pair(&m_mSignalStorage[t_valueName], (m_mSignalStorage[t_in1Name] - m_mSignalStorage[t_2PiName]).limit(m_mSignalStorage[t_minName], m_mSignalStorage[t_maxName])),
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare (dPhiAx_m >= 0)
    t_compare = Belle2::TRGCDCJSignal::comp(m_mSignalStorage[t_in1Name], ">=", m_mSignalStorage[t_minName]);
    // Assignments (dPhiAx_c <= dPhiAx_m)
    t_assigns = {
      make_pair(&m_mSignalStorage[t_valueName], m_mSignalStorage[t_in1Name].limit(m_mSignalStorage[t_minName], m_mSignalStorage[t_maxName])),
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Compare (dPhiAx_m < 0)
    t_compare = Belle2::TRGCDCJSignal();
    // Assignments (dPhiAx_c <= dPhiAx_m + 2*pi)
    t_assigns = {
      make_pair(&m_mSignalStorage[t_valueName], (m_mSignalStorage[t_in1Name] + m_mSignalStorage[t_2PiName]).limit(m_mSignalStorage[t_minName], m_mSignalStorage[t_maxName])),
    };
    // Push to data.
    t_data.push_back(make_pair(t_compare, t_assigns));
    // Process ifElse data.
    Belle2::TRGCDCJSignal::ifElse(t_data);
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<dPhiAxMax_"<<iSt<<">>>"<<endl; m_mSignalStorage["dPhiAxMax_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<dPhiAxMin_"<<iSt<<">>>"<<endl; m_mSignalStorage["dPhiAxMin_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<dPhiAx2Pi_"<<iSt<<">>>"<<endl; m_mSignalStorage["dPhiAx2Pi_"+to_string(iSt)].dump();}
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<dPhiAx_c_"<<iSt<<">>>"<<endl; m_mSignalStorage["dPhiAx_c_"+to_string(iSt)].dump();}
  // Change to wire space
  // Make wireFactor constants
  if (m_mSignalStorage.find("wireFactor_0") == m_mSignalStorage.end()) {
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      int nShiftBits = int(log(pow(2, 24) * 2 * mConstD.at("Trg_PI") / mConstV.at("nTSs")[2 * iSt + 1] /
                               m_mSignalStorage["phi0"].getToReal()) / log(2));
      string t_name;
      t_name = "wireFactor_" + to_string(iSt);
      m_mSignalStorage[t_name] = Belle2::TRGCDCJSignal(mConstV.at("nTSs")[2 * iSt + 1] / 2 / mConstD.at("Trg_PI"),
                                                       1 / m_mSignalStorage["phi0"].getToReal() / pow(2, nShiftBits), m_commonData);
    }
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<wireFactor_"<<iSt<<">>>"<<endl; m_mSignalStorage["wireFactor_"+to_string(iSt)].dump();}
  // wireDPhiAx[i] <= dPhiAx_c[i] * wireFactor[i]
  // Shift to find actual wire.
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    int nShiftBits = log(1 / m_mSignalStorage["dPhiAx_c_" + to_string(iSt)].getToReal() / m_mSignalStorage["wireFactor_" + to_string(
                           iSt)].getToReal()) / log(2);
    m_mSignalStorage["wireDPhiAx_" + to_string(iSt)] <= (m_mSignalStorage["dPhiAx_c_" + to_string(
                                                           iSt)] * m_mSignalStorage["wireFactor_" + to_string(iSt)]).shift(nShiftBits, 0);
  }
  //for(int iSt=0; iSt<4; iSt++) {cout<<"<<<wireDPhiAx_"<<iSt<<">>>"<<endl; m_mSignalStorage["wireDPhiAx_"+to_string(iSt)].dump();}
  // Select TS window from wireDPhiAx
  vector< int > nCandidates = { 10, 10, 10, 13 };
  // stCandHitmap[iSt][iTS]
  vector<vector<bool> > t_stCandHitmap(4);
  for (unsigned iSt = 0; iSt < 4; iSt++) t_stCandHitmap[iSt] = vector<bool> (nCandidates[iSt]);
  // Last driftInfo is for showing no hit.
  vector<vector<int> > t_stCandDriftmap(4);
  for (unsigned iSt = 0; iSt < 4; iSt++) t_stCandDriftmap[iSt] = vector<int> (nCandidates[iSt] + 1);
  // resultValues = [name, value, bitwidth, min, max, clock]
  vector<tuple<string, double, int, double, double, int> > resultValues;
  {
    vector<pair<string, int> > t_chooseSignals = {
      make_pair("wireDPhiAx_0", 1), make_pair("wireDPhiAx_1", 1), make_pair("wireDPhiAx_2", 1), make_pair("wireDPhiAx_3", 1)
    };
    Belle2::TRGCDCJSignal::mapSignalsToValues(m_mSignalStorage, t_chooseSignals, resultValues);
  }
  vector<double> t_wireDPhiAx = {std::get<1>(resultValues[0]), std::get<1>(resultValues[1]), std::get<1>(resultValues[2]), std::get<1>(resultValues[3])};
  // Print wireDPhiAx
  //for(unsigned iSt=0; iSt<4; iSt++) cout<<"iSt:"<<iSt<<" t_wireDPhiAx:"<<t_wireDPhiAx[iSt]<<endl;
  for (int iSt = 0; iSt < 4; iSt++) {
    int indexTS = t_wireDPhiAx[iSt];
    for (int iTS = 0; iTS < nCandidates[iSt]; iTS++) {
      // Copy 10 TSs from hitmap and driftmap
      t_stCandHitmap[iSt][iTS] = m_hitMap[iSt][indexTS];
      t_stCandDriftmap[iSt][iTS] = m_driftMap[iSt][indexTS];
      // Move indexTS
      if (iSt % 2 == 0) {
        indexTS--;
        // If index is below 0 deg goto 360-delta deg
        if (indexTS < 0) indexTS =  m_nWires[iSt] / 2 - 1;
      } else {
        indexTS++;
        // If index is 360 deg goto 0 deg
        if (indexTS >= m_nWires[iSt] / 2) indexTS = 0;
      }
    }
  }
  // Print stCandHitMap
  if (m_verboseFlag) {
    for (unsigned iLayer = 0; iLayer < 4; iLayer++) {
      cout << "iSt:" << iLayer << " t_wireDPhiAx:" << t_wireDPhiAx[iLayer];
      int t_endTSId = 0;
      if (iLayer % 2 == 0) t_endTSId = t_wireDPhiAx[iLayer] - (nCandidates[iLayer] - 1);
      else t_endTSId = t_wireDPhiAx[iLayer] + (nCandidates[iLayer] - 1);
      if (t_endTSId < 0) t_endTSId += mConstV["nTSs"][2 * iLayer + 1];
      else if (t_endTSId >= mConstV["nTSs"][2 * iLayer + 1]) t_endTSId -= mConstV["nTSs"][2 * iLayer + 1];
      //cout<<" endWindow:"<<t_endTSId<<endl;
      cout << " t_stCandHitmap[" << iLayer << "]: " << t_endTSId << "=> ";
      for (int iTS = nCandidates[iLayer] - 1; iTS >= 0; iTS--) {
        cout << t_stCandHitmap[iLayer][iTS];
      }
      cout << " <= " << t_wireDPhiAx[iLayer] << endl;
    }
    //// Print stCandDriftHitMap
    //for(unsigned iSt=0; iSt<4; iSt++){
    //  for(int iTS=0; iTS<nCandidates[iSt]; iTS++){
    //    cout<<"iSt:"<<iSt<<" iTS:"<<iTS<<" stCandDriftMap:"<<t_stCandDriftmap[iSt][iTS]<<endl;
    //  }
    //}
  }
  vector<double> t_targetWirePosition = { 3, 2, 3, 3};
  // When bestRelTsId is nCandidates, it means no hit.
  vector<int> t_bestRelTSId(4);
  for (unsigned iSt = 0; iSt < 4; iSt++) t_bestRelTSId[iSt] = nCandidates[iSt];
  vector<int> t_bestDiff = {16, 16, 16, 16};
  // Select best TS between 10 wires.
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    for (int iTS = 0; iTS < nCandidates[iSt]; iTS++) {
      int t_priorityPosition = (t_stCandDriftmap[iSt][iTS] & 3);
      //cout<<"iSt:"<<iSt<<" iTS:"<<iTS<<" t_priorityPosition:"<<t_priorityPosition<<endl;
      if (t_stCandHitmap[iSt][iTS] == 0) continue;
      if (t_priorityPosition != 3) continue;
      double tsDiffTarget = fabs(t_targetWirePosition[iSt] - iTS);
      if (t_bestDiff[iSt] > tsDiffTarget) {
        t_bestDiff[iSt] = tsDiffTarget;
        t_bestRelTSId[iSt] = iTS;
      }
    }
  }
  // Print best relative output
  if (m_verboseFlag) {
    for (unsigned iSt = 0; iSt < 4;
         iSt++) cout << "iSt:" << iSt << " bestRelTS:" << t_bestRelTSId[iSt] << " diff:" << t_bestDiff[iSt] << endl;
  }
  // Convert best relative TS id to best TS id
  vector<double> t_bestTSId(4);
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    if (iSt % 2 == 0) t_bestTSId[iSt] = t_wireDPhiAx[iSt] - t_bestRelTSId[iSt];
    else t_bestTSId[iSt] = t_wireDPhiAx[iSt] + t_bestRelTSId[iSt];
  }
  // Get bestDriftInfo
  vector<double> t_bestDriftInfo(4);
  for (unsigned iSt = 0; iSt < 4; iSt++) t_bestDriftInfo[iSt] = t_stCandDriftmap[iSt][t_bestRelTSId[iSt]];
  //// Print best output
  //for(unsigned iSt=0; iSt<4; iSt++) cout<<"iSt:"<<iSt<<" bestTS:"<<t_bestTSId[iSt]<<endl;
  // Print best drift info
  if (m_verboseFlag) {
    for (unsigned iSt = 0; iSt < 4; iSt++)cout << "iSt:" << iSt << " bestDriftInfo:" << t_bestDriftInfo[iSt] << endl;
  }

  // Constrain bestTS to [0, nTS-1]
  vector<double> t_bestTSId_c(4);
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    if (t_bestTSId[iSt] >= mConstV["nTSs"][2 * iSt + 1]) t_bestTSId_c[iSt] = t_bestTSId[iSt] - mConstV["nTSs"][2 * iSt + 1];
    else if (t_bestTSId[iSt] < 0) t_bestTSId_c[iSt] = t_bestTSId[iSt] + mConstV["nTSs"][2 * iSt + 1];
    else t_bestTSId_c[iSt] = t_bestTSId[iSt];
  }
  // Print best constrained output
  if (m_verboseFlag) {
    for (unsigned iSt = 0; iSt < 4; iSt++)cout << "iSt:" << iSt << " bestTS_c:" << t_bestTSId_c[iSt] << endl;
  }

  // Save to m_bestTS
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    if (t_bestDriftInfo[iSt] != 0) m_bestTS[iSt] = t_bestTSId_c[iSt] * 2 * mConstD["Trg_PI"] / mConstV["nTSs"][2 * iSt + 1] ;
    else m_bestTS[iSt] = 999;
  }
  //if(m_verboseFlag) {
  //  for(unsigned iSt=0; iSt<4; iSt++)cout<<"iSt:"<<iSt<<" m_bestTS:"<<m_bestTS[iSt]<<endl;
  //}
  // Search and save to m_bestTSIndex
  for (unsigned iSt = 0; iSt < 4; iSt++) {
    if (t_bestDriftInfo[iSt] == 0) {
      m_bestTSIndex[iSt] = 999;
    } else {
      for (unsigned iTS = 0; iTS < stTSs[iSt].size(); iTS++) {
        if (fabs(m_bestTS[iSt] - stTSs[iSt][iTS]) < 0.0001) {
          m_bestTSIndex[iSt] = iTS;
          break;
        }
      }
    }
  }
  //if(m_verboseFlag) {
  //  for(unsigned iSt=0; iSt<4; iSt++)cout<<"iSt:"<<iSt<<" m_bestTSIndex:"<<m_bestTSIndex[iSt]<<endl;
  //}

  // Post handling of signals.
  // Name all signals.
  if ((*m_mSignalStorage.begin()).second.getName() == "") {
    for (auto it = m_mSignalStorage.begin(); it != m_mSignalStorage.end(); ++it) {
      (*it).second.setName((*it).first);
    }
  }
  ////// Dump all signals.
  ////bool done = 0;
  ////if((*m_mSignalStorage.begin()).second.getName() != "" && done==0){
  ////  for(auto it = m_mSignalStorage.begin(); it != m_mSignalStorage.end(); ++it){
  ////    (*it).second.dump();
  ////  }
  ////  done=1;
  ////}

  if (m_mBool["fVHDLFile"]) {
    // Check if there is a name.
    if ((*m_mSignalStorage.begin()).second.getName() != "") {
      // Saves to file only one time.
      // Print Vhdl and Coe
      if (m_commonData->getPrintedToFile() == 0) {
        if (m_commonData->getPrintVhdl() == 0) {
          m_commonData->setVhdlOutputFile(m_outputVhdlDirname + "/Finder3D.vhd");
          m_commonData->setPrintVhdl(1);
        } else {
          m_commonData->setPrintVhdl(0);
          m_commonData->entryVhdlCode();
          m_commonData->signalsVhdlCode();
          m_commonData->buffersVhdlCode();
          m_commonData->printToFile();
          // Print LUTs.
          for (map<string, Belle2::TRGCDCJLUT*>::iterator it = m_mLutStorage.begin(); it != m_mLutStorage.end(); ++it) {
            it->second->makeCOE(m_outputLutDirname + "/" + it->first + ".coe");
            //it->second->makeCOE(it->first+".coe");
          }
        }
      }

      // Saves values to memory. Wrote to file at terminate().
      // Save all signals to m_mSavedSignals. Limit to 10 times.
      if ((*m_mSavedSignals.begin()).second.size() < 10 || m_mSavedSignals.empty()) {
        for (auto it = m_mSignalStorage.begin(); it != m_mSignalStorage.end(); ++it) {
          m_mSavedSignals[(*it).first].push_back((*it).second.getInt());
        }
      }

      // Saves values to memory. Wrote to file at terminate().
      // Save input output signals. Limit to 1024.
      if ((*m_mSavedIoSignals.begin()).second.size() < 1025 || m_mSavedIoSignals.empty()) {
        m_mSavedIoSignals["in_phi0"].push_back(m_mSignalStorage["phi0"].getInt());
        m_mSavedIoSignals["in_rho"].push_back(m_mSignalStorage["rho"].getInt());
        m_mSavedIoSignals["in_charge"].push_back(m_mSignalStorage["charge"].getInt());
        for (unsigned iSt = 0; iSt < 4;
             iSt++) m_mSavedIoSignals["out_wireDPhiAx_" + to_string(iSt)].push_back(m_mSignalStorage["wireDPhiAx_" + to_string(iSt)].getInt());
      }

    }
  }

  if (m_verboseFlag) cout << "####geoFinder end####" << endl;

}

void Hough3DFinder::getValues(const string& input, vector<double>& result)
{
  //Clear result vector
  result.clear();

  if (input == "bestCot") {
    result.push_back(m_bestCot);
  }
  if (input == "bestZ0") {
    result.push_back(m_bestZ0);
  }
  if (input == "houghMax") {
    result.push_back(m_houghMax);
  }
  if (input == "minDiffHough") {
    result.push_back(m_minDiffHough);
  }
  if (input == "foundZ") {
    result.push_back(m_foundZ[0]);
    result.push_back(m_foundZ[1]);
    result.push_back(m_foundZ[2]);
    result.push_back(m_foundZ[3]);
  }
  if (input == "foundPhiSt") {
    result.push_back(m_foundPhiSt[0]);
    result.push_back(m_foundPhiSt[1]);
    result.push_back(m_foundPhiSt[2]);
    result.push_back(m_foundPhiSt[3]);
  }
  if (input == "bestTS") {
    result.push_back(m_bestTS[0]);
    result.push_back(m_bestTS[1]);
    result.push_back(m_bestTS[2]);
    result.push_back(m_bestTS[3]);
  }
  if (input == "bestTSIndex") {
    result.push_back(m_bestTSIndex[0]);
    result.push_back(m_bestTSIndex[1]);
    result.push_back(m_bestTSIndex[2]);
    result.push_back(m_bestTSIndex[3]);
  }
  if (input == "st0GeoCandidatesPhi") {
    for (int iTS = 0; iTS < int((*m_geoCandidatesPhi)[0].size()); iTS++)
      result.push_back((*m_geoCandidatesPhi)[0][iTS]);
  }
  if (input == "st1GeoCandidatesPhi") {
    for (int iTS = 0; iTS < int((*m_geoCandidatesPhi)[1].size()); iTS++)
      result.push_back((*m_geoCandidatesPhi)[1][iTS]);
  }
  if (input == "st2GeoCandidatesPhi") {
    for (int iTS = 0; iTS < int((*m_geoCandidatesPhi)[2].size()); iTS++)
      result.push_back((*m_geoCandidatesPhi)[2][iTS]);
  }
  if (input == "st3GeoCandidatesPhi") {
    for (int iTS = 0; iTS < int((*m_geoCandidatesPhi)[3].size()); iTS++)
      result.push_back((*m_geoCandidatesPhi)[3][iTS]);
  }

  if (input == "st0GeoCandidatesDiffStWires") {
    for (int iTS = 0; iTS < int((*m_geoCandidatesDiffStWires)[0].size()); iTS++)
      result.push_back((*m_geoCandidatesDiffStWires)[0][iTS]);
  }
  if (input == "st1GeoCandidatesDiffStWires") {
    for (int iTS = 0; iTS < int((*m_geoCandidatesDiffStWires)[1].size()); iTS++)
      result.push_back((*m_geoCandidatesDiffStWires)[1][iTS]);
  }
  if (input == "st2GeoCandidatesDiffStWires") {
    for (int iTS = 0; iTS < int((*m_geoCandidatesDiffStWires)[2].size()); iTS++)
      result.push_back((*m_geoCandidatesDiffStWires)[2][iTS]);
  }
  if (input == "st3GeoCandidatesDiffStWires") {
    for (int iTS = 0; iTS < int((*m_geoCandidatesDiffStWires)[3].size()); iTS++)
      result.push_back((*m_geoCandidatesDiffStWires)[3][iTS]);
  }

  if (input == "st0GeoCandidatesIndex") {
    for (int iTS = 0; iTS < int((*m_geoCandidatesIndex)[0].size()); iTS++)
      result.push_back((*m_geoCandidatesIndex)[0][iTS]);
  }
  if (input == "st1GeoCandidatesIndex") {
    for (int iTS = 0; iTS < int((*m_geoCandidatesIndex)[1].size()); iTS++)
      result.push_back((*m_geoCandidatesIndex)[1][iTS]);
  }
  if (input == "st2GeoCandidatesIndex") {
    for (int iTS = 0; iTS < int((*m_geoCandidatesIndex)[2].size()); iTS++)
      result.push_back((*m_geoCandidatesIndex)[2][iTS]);
  }
  if (input == "st3GeoCandidatesIndex") {
    for (int iTS = 0; iTS < int((*m_geoCandidatesIndex)[3].size()); iTS++)
      result.push_back((*m_geoCandidatesIndex)[3][iTS]);
  }

  if (input == "stAxPhi") {
    result.push_back(m_stAxPhi[0]);
    result.push_back(m_stAxPhi[1]);
    result.push_back(m_stAxPhi[2]);
    result.push_back(m_stAxPhi[3]);
  }

  if (input == "extreme") {
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

  if (input == "hitmapLayer1") {
    for (unsigned iOutput = 0; iOutput < unsigned(m_nWires[0] / 2); iOutput++) {
      result.push_back(m_hitMap[0][iOutput]);
    }
  }

  if (input == "hitmapLayer2") {
    for (unsigned iOutput = 0; iOutput < unsigned(m_nWires[1] / 2); iOutput++) {
      result.push_back(m_hitMap[1][iOutput]);
    }
  }

  if (input == "hitmapLayer3") {
    for (unsigned iOutput = 0; iOutput < unsigned(m_nWires[2] / 2); iOutput++) {
      result.push_back(m_hitMap[2][iOutput]);
    }
  }

  if (input == "hitmapLayer4") {
    for (unsigned iOutput = 0; iOutput < unsigned(m_nWires[3] / 2); iOutput++) {
      result.push_back(m_hitMap[3][iOutput]);
    }
  }

}

void Hough3DFinder::getHoughMeshLayer(bool***& houghMeshLayer)
{
  houghMeshLayer = m_houghMeshLayer;
}
