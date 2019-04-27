/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alberto Martini                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackExtrapolateG4e/MuidPar.h>
#include <tracking/dataobjects/Muid.h>
#include <tracking/dataobjects/MuidHit.h>
#include <tracking/dbobjects/MuidParameters.h>

#include <fstream>
#include <exception>
#include <cmath>

#include <framework/utilities/FileSystem.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

using namespace std;
namespace Belle2 {

  MuidPar::MuidPar() : m_ReducedChiSquaredDx(0.0)
  {
  }

  MuidPar::MuidPar(int expNo, const char hypothesisName[]) : m_ReducedChiSquaredDx(0.0)
  {
    //fill PDFs by reading database
    fillPDFs(hypothesisName);
    if (m_ReducedChiSquaredDx == 0.0) { B2FATAL("Failed to read " << hypothesisName << " PDFs for experiment " << expNo); }
  }

  MuidPar::~MuidPar()
  {
  }


  void MuidPar::fillPDFs(const char hypothesisName[])
  {
    vector<string> const hypotheses = {"Positron", "Electron" , "Deuteron", "Antideuteron", "Proton", "Antiproton", "PionPlus", "PionMinus", "KaonPlus", "KaonMinus", "MuonPlus", "MuonMinus" };
    int hypothesis = -1;
    for (unsigned int ii = 0; ii < hypotheses.size(); ii++) { if (hypothesisName == hypotheses[ii]) {hypothesis = ii; break;}}
    if (hypothesis == -1) B2FATAL("MuidPar::fillPDFs(): hypothesisName " << hypothesisName << "is not expected. ");

    for (int outcome = 1; outcome <= MUID_MaxOutcome; ++outcome) {
      for (int lastLayer = 0; lastLayer <= MUID_MaxBarrelLayer; ++lastLayer) {
        if ((outcome == MuidPar::EMuidOutcome::c_StopInBarrel)
            && (lastLayer > MUID_MaxBarrelLayer - 1)) break; // barrel stop: never in layer 14
        if ((outcome == MuidPar::EMuidOutcome::c_StopInForwardEndcap)
            && (lastLayer > MUID_MaxForwardEndcapLayer - 1)) break; // forward endcap stop: never in layer 13
        if ((outcome == MuidPar::EMuidOutcome::c_ExitBarrel) && (lastLayer > MUID_MaxBarrelLayer)) break; // barrel exit: no layers 15+
        if ((outcome == MuidPar::EMuidOutcome::c_ExitForwardEndcap)
            && (lastLayer > MUID_MaxForwardEndcapLayer)) break; // forward endcap exit: no layers 14+
        if ((outcome == MuidPar::EMuidOutcome::c_StopInBackwardEndcap)
            && (lastLayer > MUID_MaxBackwardEndcapLayer - 1)) break; // backward endcap stop: never in layer 11
        if ((outcome == MuidPar::EMuidOutcome::c_ExitBackWardEndcap)
            && (lastLayer > MUID_MaxBackwardEndcapLayer)) break; // backward endcap exit: no layers 12+
        if ((outcome >= MuidPar::EMuidOutcome::c_CrossBarrelStopInForwardMin)
            && (outcome <= MuidPar::EMuidOutcome::c_CrossBarrelStopInForwardMax)
            && (lastLayer > MUID_MaxForwardEndcapLayer - 1)) break; // like outcome == 2
        if ((outcome >= MuidPar::EMuidOutcome::c_CrossBarrelStopInBackwardMin)
            && (outcome <= MuidPar::EMuidOutcome::c_CrossBarrelStopInBackwardMax)
            && (lastLayer > MUID_MaxBackwardEndcapLayer - 1)) break; // like outcome == 5
        if ((outcome >= MuidPar::EMuidOutcome::c_CrossBarrelExitForwardMin)
            && (outcome <= MuidPar::EMuidOutcome::c_CrossBarrelExitForwardMax)
            && (lastLayer > MUID_MaxForwardEndcapLayer)) break; // like outcome == 4
        if ((outcome >= MuidPar::EMuidOutcome::c_CrossBarrelExitBackwardMin)
            && (outcome <= MuidPar::EMuidOutcome::c_CrossBarrelExitBackwardMax)
            && (lastLayer > MUID_MaxBackwardEndcapLayer)) break; // like outcome == 6
        std::vector<double> layerPDF = m_muidParameters->getProfile(hypothesis, outcome, lastLayer);
        for (unsigned int layer = 0; layer < layerPDF.size(); ++layer) {
          m_LayerPDF[outcome][lastLayer][layer] = layerPDF[layer];
        }
      }
    }

    m_ReducedChiSquaredDx = MUID_ReducedChiSquaredLimit / MUID_ReducedChiSquaredNbins;   // bin size
    for (int detector = 0; detector <= MUID_MaxDetector; ++detector) {

      for (int halfNdof = 1; halfNdof <= MUID_MaxHalfNdof; ++halfNdof) {
        m_ReducedChiSquaredThreshold[detector][halfNdof] = m_muidParameters->getThreshold(hypothesis, detector, halfNdof * 2);
        m_ReducedChiSquaredScaleY[detector][halfNdof] = m_muidParameters->getScaleY(hypothesis, detector, halfNdof * 2);
        m_ReducedChiSquaredScaleX[detector][halfNdof] = m_muidParameters->getScaleX(hypothesis, detector, halfNdof * 2);
        std::vector<double> reducedChiSquaredPDF = m_muidParameters->getPDF(hypothesis, detector, halfNdof * 2);
        if (reducedChiSquaredPDF.size() != MUID_ReducedChiSquaredNbins) {
          B2ERROR("TransversePDF vector for hypothesis " << hypothesisName << "  detector " << detector
                  << " has " << reducedChiSquaredPDF.size() << " entries; should be " << MUID_ReducedChiSquaredNbins);
          m_ReducedChiSquaredDx = 0.0; // invalidate the PDFs for this hypothesis
        } else {
          for (int i = 0; i < MUID_ReducedChiSquaredNbins; ++i) {
            m_ReducedChiSquaredPDF[detector][halfNdof][i] = reducedChiSquaredPDF[i];
          }
          spline(MUID_ReducedChiSquaredNbins - 1, m_ReducedChiSquaredDx,
                 &m_ReducedChiSquaredPDF[detector][halfNdof][0],
                 &m_ReducedChiSquaredD1[detector][halfNdof][0],
                 &m_ReducedChiSquaredD2[detector][halfNdof][0],
                 &m_ReducedChiSquaredD3[detector][halfNdof][0]);
          m_ReducedChiSquaredD1[detector][halfNdof][MUID_ReducedChiSquaredNbins - 1] = 0.0;
          m_ReducedChiSquaredD2[detector][halfNdof][MUID_ReducedChiSquaredNbins - 1] = 0.0;
          m_ReducedChiSquaredD3[detector][halfNdof][MUID_ReducedChiSquaredNbins - 1] = 0.0;
        }
      }
    }

  }

  void MuidPar::spline(int n, double dx, double Y[], double B[], double C[], double D[])
  {

    // Generate the spline interpolation coefficients B, C, D to smooth out a
    // binned histogram. Restrictions:  equal-size bins. more than 3 bins.

    D[0] = dx;
    C[1] = (Y[1] - Y[0]) / dx;
    for (int i = 1; i < n - 1; i++) {
      D[i]   = dx;
      B[i]   = dx * 4.0;
      C[i + 1] = (Y[i + 1] - Y[i]) / dx;
      C[i]   = C[i + 1] - C[i];
    }
    B[0]   = -dx;
    B[n - 1] = -dx;
    C[0]   = (C[2]   - C[1]) / 6.0;
    C[n - 1] = -(C[n - 2] - C[n - 3]) / 6.0;
    for (int i = 1; i < n; i++) {
      double temp = dx / B[i - 1];
      B[i] -= temp * dx;
      C[i] -= temp * C[i - 1];
    }
    C[n - 1] /= B[n - 1];
    for (int i = n - 2; i >= 0; i--) {
      C[i] = (C[i] - D[i] * C[i + 1]) / B[i];
    }
    B[n - 1] = (Y[n - 1] - Y[n - 2]) / dx + (C[n - 2] + C[n - 1] * 2.0) * dx;
    for (int i = 0; i < n - 1; i++) {
      B[i] = (Y[i + 1] - Y[i]) / dx - (C[i + 1] + C[i] * 2.0) * dx;
      D[i] = (C[i + 1] - C[i]) / dx;
      C[i] = C[i] * 3.0;
    }
    C[n - 1] = C[n - 1] * 3.0;
    D[n - 1] = D[n - 2];
  }


  double MuidPar::getPDF(const Muid* muid, bool isForward) const
  {
    return getPDFLayer(muid, isForward) * getPDFRchisq(muid);
  }

  double MuidPar::getPDF_muon(const Muid* muid, bool isForward) const
  {
    return getPDFLayer_muon(muid, isForward) * getPDFRchisq(muid);
  }


  double MuidPar::getPDFLayer(const Muid* muid, bool isForward) const
  {

    int outcome = muid->getOutcome();
    if ((outcome <= 0) || (outcome > 4)) return 0.0;

    int barrelExtLayer = muid->getBarrelExtLayer();
    int endcapExtLayer = muid->getEndcapExtLayer();
    if (barrelExtLayer > MUID_MaxBarrelLayer) return 0.0;
    if (endcapExtLayer > MUID_MaxForwardEndcapLayer) return 0.0;
    unsigned int extLayerPattern = muid->getExtLayerPattern();
    unsigned int hitLayerPattern = muid->getHitLayerPattern();

    int lastLayer = barrelExtLayer;
    if (outcome == 2) { // forward endcap stop (no barrel hits)
      lastLayer = endcapExtLayer;
      if (barrelExtLayer < 0) {
        outcome = isForward ? 2 : 5; // forward or backward endcap stop (no barrel hits)
      } else {
        outcome = (isForward ? 7 : 22) + barrelExtLayer; // forward/backward endcap stop (B+E)
      }
    } else if (outcome == 4) { // forward endcap exit (no barrel hits)
      lastLayer = endcapExtLayer;
      if (barrelExtLayer < 0) {
        outcome = isForward ? 4 : 6;  // forward or backward endcap exit (no barrel hits)
      } else {
        outcome = (isForward ? 37 : 52) + barrelExtLayer; // forward/backward endcap exit (B+E)
      }
    }


    int nBarrelExt = 0;
    int nBarrelHit = 0;
    for (int layer_new_BKLM = 0; layer_new_BKLM < 15; ++layer_new_BKLM) { // 15 barrel layers
      unsigned int bit = (1 << layer_new_BKLM);
      if ((extLayerPattern & bit) != 0) {
        nBarrelExt += 1; //counting the number of BKLM extrapolated layers
        if ((hitLayerPattern & bit) != 0) {
          nBarrelHit += ((hitLayerPattern & bit) != 0) ? 1 : 0; //counting the number of BKLM layers with hits in the pattern
        }
      }
    }

    //Pdf treatment used to avoid layer inefficiency problems
    double pdf = 1.0;
    unsigned int testBit = 1;
    int sector = -1;
    float thresholdFraction = 0.33;
    MuidHit* muidHits;

    RelationVector<MuidHit> muidhit = muid->getRelationsTo<MuidHit>();
    for (unsigned int muhit = 0; muhit < muidhit.size(); muhit++) {
      muidHits =  muidhit[muhit];
      sector = muidHits->getSector();
      if (sector >= 0) break;
    }

    //BKLM EFFICIENCIES PHASE 2 for each sectors and each layers
    double Eff_BF0[15] = {0.00266667, 0.313149, 0.303922, 0.426471, 0.356061, 0.835341, 0.741627, 0.82205, 0.891935, 0.570646, 0.789902, 0.849438, 0.824931, 0.716707, 0.334281};
    double Eff_BF1[15] = {0.410815, 0.822917, 0.372642, 0.587234, 0.395349, 0.832392, 0.8528, 0.822755, 0.78553, 0.602975, 0.709347, 0.787199, 0, 0, 0.730331};
    double Eff_BF2[15] = {0};
    double Eff_BF3[15] = {0.293146, 0.803922, 0.666667, 0.745038, 0.73838, 0.849817, 0.877976, 0.869728, 0.873935, 0.553882, 0.83905, 0.621658, 0.722701, 0.73315, 0.683995};
    double Eff_BF4[15] = {0.490405, 0.856354, 0.82, 0.845588, 0.733333, 0, 0.963907, 0.96401, 0.852459, 0.574007, 0.736585, 0.773529, 0, 0.773936, 0.771883};
    double Eff_BF5[15] = {0, 0.283128, 0.909091, 0.875224, 0.0368034, 0.89071, 0.898216, 0.895127, 0.877193, 0.904465, 0.936652, 0.203077, 0.857868, 0.865826, 0.806624};
    double Eff_BF6[15] = {0.574825, 0.605797, 0.231481, 0.415459, 0.218605, 0.753846, 0.811533, 0.852476, 0.857749, 0.539116, 0.615248, 0.896259, 0.781475, 0.806358, 0.811833};
    double Eff_BF7[15] = {0, 0.137615, 0.372093, 0.37551, 0.518625, 0.805324, 0.822093, 0.70596, 0.572052, 0.525292, 0.672504, 0.838158, 0.82247, 0.767974, 0.762987};

    double Eff_BB0[15] = {0.0173661, 0.511504, 0.775316, 0.855711, 0.907308, 0.00490998, 0.937226, 0.933579, 0.0379507, 0.850112, 0.74364, 0.89703, 0.0720109, 0.87108, 0.689127};
    double Eff_BB1[15] = {0, 0.167857, 0.622222, 0.764423, 0.573864, 0.8437, 0.859036, 0.556748, 0.81104, 0.656328, 0.669749, 0.808803, 0.797143, 0.645641, 0.467733};
    double Eff_BB2[15] = {0.26357, 0.129688, 0.758065, 0.547105, 0.662222, 0.758564, 0.860849, 0.80853, 0.670051, 0.636285, 0.922764, 0.202169, 0.779736, 0.589841, 0.172103};
    double Eff_BB3[15] = {0.34612, 0.246792, 0.612676, 0.649916, 0.758359, 0.81388, 0.882759, 0.862028, 0.58209, 0.783617, 0.695918, 0.855087, 0.839012, 0.710262, 0.542714};
    double Eff_BB4[15] = {0.383975, 0.405609, 0.511111, 0.370166, 0.378505, 0.829167, 0.871755, 0.885891, 0.855556, 0.603627, 0.695035, 0.866407, 0.806483, 0.607404, 0.68313};
    double Eff_BB5[15] = {0.459225, 0.458082, 0.565359, 0.317967, 0.375375, 0.73494, 0.915799, 0.776042, 0.848387, 0.70607, 0.621698, 0.837285, 0.765024, 0.831115, 0.718705};
    double Eff_BB6[15] = {0.133038, 0.0634518, 0.818505, 0.776632, 0.83959, 0.920382, 0.905817, 0.915014, 0.877493, 0.873563, 0.875776, 0.755906, 0.65625, 0.790476, 0.0493827};
    double Eff_BB7[15] = {0.323093, 0.12311, 0.663677, 0.576842, 0.741093, 0.774687, 0.855208, 0.807381, 0.754821, 0.648686, 0.736682, 0.756126, 0.835646, 0.701065, 0.77396};

    //----------------


    for (int layer = 0; layer <= barrelExtLayer; ++layer) {
      if ((testBit & extLayerPattern) != 0) {
        if ((testBit & hitLayerPattern) != 0) {//checking the presence of a hit in the layer
          //if there are less that a third of hits among the extrapolted path, the track is considered as not a muon
          if (nBarrelHit <= thresholdFraction * nBarrelExt) {
            pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]);
          } else {
            pdf *= m_LayerPDF[outcome][lastLayer][layer];
          }
        } else {
          if (((layer == 0) && (outcome < 7)) || (layer == MUID_MaxBarrelLayer) || (layer < barrelExtLayer)) {
            //if there are less that a third of hits among the extrapolted path, the track is considered as not a muon
            if (nBarrelHit <= thresholdFraction * nBarrelExt) {
              pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]);
            } else {
              unsigned int testBitPrev = testBit >> 1;
              unsigned int testBitNext = testBit << 1;
              unsigned int testBitNextNext = testBit << 2;
              if (layer > 0 && ((((testBitPrev & hitLayerPattern) != 0) && ((testBitNext & hitLayerPattern) != 0))
                                || (((testBitNext & hitLayerPattern) != 0)
                                    && ((testBitNextNext & hitLayerPattern) != 0)))) {
                pdf *= m_LayerPDF[outcome][lastLayer][layer];
              } else {
                //treatment of inefficient layers using BKLM efficiencies
                if (isForward) { //select forward BKLM section
                  switch (sector) {
                    case (0):
                      if (Eff_BF0[layer] < 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (1 - Eff_BF0[layer]) / 6;
                      } else if (Eff_BF0[layer] >= 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (Eff_BF0[layer]);
                      }
                      break;
                    case (1):
                      if (Eff_BF1[layer] < 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (1 - Eff_BF1[layer]) / 6;
                      } else if (Eff_BF1[layer] >= 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (Eff_BF1[layer]);
                      }
                      break;
                    case (2):
                      if (Eff_BF2[layer] < 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (1 - Eff_BF2[layer]) / 6;
                      } else if (Eff_BF2[layer] >= 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (Eff_BF2[layer]);
                      }
                      break;
                    case (3):
                      if (Eff_BF3[layer] < 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (1 - Eff_BF3[layer]) / 6;
                      } else if (Eff_BF3[layer] >= 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (Eff_BF3[layer]);
                      }
                      break;
                    case (4):
                      if (Eff_BF4[layer] < 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (1 - Eff_BF4[layer]) / 6;
                      } else if (Eff_BF4[layer] >= 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (Eff_BF4[layer]);
                      }
                      break;
                    case (5):
                      if (Eff_BF5[layer] < 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (1 - Eff_BF5[layer]) / 6;
                      } else if (Eff_BF5[layer] >= 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (Eff_BF5[layer]);
                      }
                      break;
                    case (6):
                      if (Eff_BF6[layer] < 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (1 - Eff_BF6[layer]) / 6;
                      } else if (Eff_BF6[layer] >= 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (Eff_BF6[layer]);
                      }
                      break;
                    case (7):
                      if (Eff_BF7[layer] < 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (1 - Eff_BF7[layer]) / 6;
                      } else if (Eff_BF7[layer] >= 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (Eff_BF7[layer]);
                      }
                      break;
                  }
                }
                if (!isForward) { //select backward BKLM section
                  switch (sector) {
                    case (0):
                      if (Eff_BB0[layer] < 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (1 - Eff_BB0[layer]) / 6;
                      } else if (Eff_BB0[layer] >= 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (Eff_BB0[layer]);
                      }
                      break;
                    case (1):
                      if (Eff_BB1[layer] < 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (1 - Eff_BB1[layer]) / 6;
                      } else if (Eff_BB1[layer] >= 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (Eff_BB1[layer]);
                      }
                      break;
                    case (2):
                      if (Eff_BB2[layer] < 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (1 - Eff_BB2[layer]) / 6;
                      } else if (Eff_BB2[layer] >= 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (Eff_BB2[layer]);
                      }
                      break;
                    case (3):
                      if (Eff_BB3[layer] < 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (1 - Eff_BB3[layer]) / 6;
                      } else if (Eff_BB3[layer] >= 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (Eff_BB3[layer]);
                      }
                      break;
                    case (4):
                      if (Eff_BB4[layer] < 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (1 - Eff_BB4[layer]) / 6;
                      } else if (Eff_BB4[layer] >= 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (Eff_BB4[layer]);
                      }
                      break;
                    case (5):
                      if (Eff_BB5[layer] < 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (1 - Eff_BB5[layer]) / 6;
                      } else if (Eff_BB5[layer] >= 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (Eff_BB5[layer]);
                      }
                      break;
                    case (6):
                      if (Eff_BB6[layer] < 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (1 - Eff_BB6[layer]) / 6;
                      } else if (Eff_BB6[layer] >= 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (Eff_BB6[layer]);
                      }
                      break;
                    case (7):
                      if (Eff_BB7[layer] < 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (1 - Eff_BB7[layer]) / 6;
                      } else if (Eff_BB7[layer] >= 0.5) {
                        pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer]) * (Eff_BB7[layer]);
                      }
                      break;
                  }
                }
              }
            }
          }
        }
      }
      testBit <<= 1; // move to next bit
    }

    int nEndcapExt = 0;
    int nEndcapHit = 0;
    int outermostEndcapLayerWithHit = -1;
    unsigned int hitPattern_EKLM = 0;
    for (int layer_new_EKLM = 0; layer_new_EKLM < 14; ++layer_new_EKLM) { // 14 endcap layers
      unsigned int bit = (0x8000 << layer_new_EKLM);
      if ((extLayerPattern & bit) != 0) {
        nEndcapExt += 1; //counting the number of extrapolated EKLM layers
        if ((hitLayerPattern & bit) != 0) {
          outermostEndcapLayerWithHit = layer_new_EKLM;
          nEndcapHit += ((hitLayerPattern & bit) != 0) ? 1 : 0;//counting the number of EKLM layers with hits in the pattern
        }
      }
    }

    // re-write a hit pattern forcing the presence of a hit in each of the layer before the outermost one with a real hit
    if ((nBarrelHit + nEndcapHit) >= 1) {
      for (int layer_new_EKLM_bitPattern = 0; layer_new_EKLM_bitPattern < outermostEndcapLayerWithHit;
           ++layer_new_EKLM_bitPattern) { // 14 endcap layers
        unsigned int bit_pattern_EKLM = (0x8000 << layer_new_EKLM_bitPattern);
        if ((extLayerPattern & bit_pattern_EKLM) != 0) {
          hitPattern_EKLM |= bit_pattern_EKLM;
        }
      }
    }

    int maxLayer = isForward ? MUID_MaxForwardEndcapLayer : MUID_MaxBackwardEndcapLayer;
    testBit = 1 << (MUID_MaxBarrelLayer + 1);
    for (int layer = 0; layer <= endcapExtLayer; ++layer) {
      if ((testBit & extLayerPattern) != 0) {
        if ((testBit & hitPattern_EKLM) != 0) {//checking the presence of a hit in the layer
          //if there are less that a third of hits among the extrapolted path, the track is considered as not a muon
          if (nBarrelHit + nEndcapHit <= thresholdFraction * (nEndcapExt + nBarrelExt)) {
            pdf *= (1 - m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1]);
          } else {
            pdf *= m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1];
          }
        } else {
          if ((layer == 0) || (layer == maxLayer) || (layer < endcapExtLayer)) {
            //if there are less that a third of hits among the extrapolted path, the track is considered as not a muon
            if (nBarrelHit + nEndcapHit <= thresholdFraction * (nEndcapExt + nBarrelExt)) {
              pdf *= 1 - m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1];
            } else {
              //treatment of inefficient very lasts EKLM layers
              if (isForward) {
                if (layer == 12) {
                  if ((11 & hitPattern_EKLM) != 0) {
                    pdf *= m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1];
                  } else {
                    pdf *= (1.0 - m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1]);
                  }
                } else if (layer == 13) {
                  if (((12 & hitPattern_EKLM) != 0) || ((11 & hitPattern_EKLM) != 0)) {
                    pdf *= m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1];
                  } else {
                    pdf *= (1.0 - m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1]);
                  }
                } else {
                  pdf *= (1.0 - m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1]);
                }
              }
              if (!isForward) {
                if (layer == 10) {
                  if ((9 & hitPattern_EKLM) != 0) {
                    pdf *= m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1];
                  } else {
                    pdf *= (1.0 - m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1]);
                  }
                } else if (layer == 11) {
                  if (((10 & hitPattern_EKLM) != 0) || ((9 & hitPattern_EKLM) != 0)) {
                    pdf *= m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1];
                  } else {
                    pdf *= (1.0 - m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1]);
                  }
                } else {
                  pdf *= (1.0 - m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1]);
                }
              }
            }
          }
        }
      }
      testBit <<= 1; // move to next bit
    }

    //    delete muidHits;
    return pdf;

  }


  double MuidPar::getPDFLayer_muon(const Muid* muid, bool isForward) const
  {

    // outcome:  0=Not in KLM, 1=Barrel Stop, 2=Endcap Stop, 3=Barrel Exit, 4=Endcap Exit

    int outcome = muid->getOutcome();
    if ((outcome <= 0) || (outcome > 4)) return 0.0;

    int barrelExtLayer = muid->getBarrelExtLayer();
    int endcapExtLayer = muid->getEndcapExtLayer();
    if (barrelExtLayer > MUID_MaxBarrelLayer) return 0.0;
    if (endcapExtLayer > MUID_MaxForwardEndcapLayer) return 0.0;
    unsigned int extLayerPattern = muid->getExtLayerPattern();
    unsigned int hitLayerPattern = muid->getHitLayerPattern();

    // Use finer granularity for non-zero outcome:
    //  1: stop in barrel
    //  2: stop in forward endcap (without crossing barrel)
    //  3: exit from barrel (without crossing endcap)
    //  4: exit from forward endcap (without crossing barrel)
    //  5: stop in forward endcap (after crossing barrel)
    //  6: exit from forward endcap (after crossing barrel)
    //  7-21: stop in forward endcap (after crossing barrel)
    //  22-36: stop in backward endcap (after crossing barrel)
    //  37-51: exit from forward endcap (after crossing barrel)
    //  52-66: exit from backward endcap (after crossing barrel)

    int lastLayer = barrelExtLayer;
    if (outcome == 2) { // forward endcap stop (no barrel hits)
      lastLayer = endcapExtLayer;
      if (barrelExtLayer < 0) {
        outcome = isForward ? 2 : 5; // forward or backward endcap stop (no barrel hits)
      } else {
        outcome = (isForward ? 7 : 22) + barrelExtLayer; // forward/backward endcap stop (B+E)
      }
    } else if (outcome == 4) { // forward endcap exit (no barrel hits)
      lastLayer = endcapExtLayer;
      if (barrelExtLayer < 0) {
        outcome = isForward ? 4 : 6;  // forward or backward endcap exit (no barrel hits)
      } else {
        outcome = (isForward ? 37 : 52) + barrelExtLayer; // forward/backward endcap exit (B+E)
      }
    }


    float thresholdFraction = 0.33;
    int nBarrelExt = 0;
    int nBarrelHit = 0;
    int outermostBarrelLayerWithHit = -1;
    unsigned int hitPattern_BKLM = 0;
    for (int layer_new_BKLM = 0; layer_new_BKLM < 15; ++layer_new_BKLM) { // 15 barrel layers
      unsigned int bit = (1 << layer_new_BKLM);
      if ((extLayerPattern & bit) != 0) {
        nBarrelExt += 1; //counting the number of BKLM extrapolated layers
        if ((hitLayerPattern & bit) != 0) {
          outermostBarrelLayerWithHit = layer_new_BKLM;
          nBarrelHit += ((hitLayerPattern & bit) != 0) ? 1 : 0; //counting the number of BKLM layers with hits in the pattern
        }
      }
      // re-write a hit pattern forcing the presence of a hit in each of the layer before the outermost one with a real hit
      if (nBarrelHit >= thresholdFraction * nBarrelExt) {
        for (int layer_new_BKLM_bitPattern = 0; layer_new_BKLM_bitPattern < outermostBarrelLayerWithHit; ++layer_new_BKLM_bitPattern) {
          unsigned int bit_pattern = (1 << layer_new_BKLM_bitPattern);
          if ((extLayerPattern & bit_pattern) != 0) {
            hitPattern_BKLM |= bit_pattern;
          }
        }
      }
    }


    double pdf = 1.0;
    unsigned int testBit = 1;
    int sector = -1;
    MuidHit* muidHits;

    RelationVector<MuidHit> muidhit = muid->getRelationsTo<MuidHit>();
    for (unsigned int muhit = 0; muhit < muidhit.size(); muhit++) {
      muidHits =  muidhit[muhit];
      sector = muidHits->getSector();
      if (sector >= 0) break;
    }

    //BKLM EFFICIENCIES PHASE 2 for each sectors and each layers
    double Eff_BF0[15] = {0.00266667, 0.313149, 0.303922, 0.426471, 0.356061, 0.835341, 0.741627, 0.82205, 0.891935, 0.570646, 0.789902, 0.849438, 0.824931, 0.716707, 0.334281};
    double Eff_BF1[15] = {0.410815, 0.822917, 0.372642, 0.587234, 0.395349, 0.832392, 0.8528, 0.822755, 0.78553, 0.602975, 0.709347, 0.787199, 0, 0, 0.730331};
    double Eff_BF2[15] = {0};
    double Eff_BF3[15] = {0.293146, 0.803922, 0.666667, 0.745038, 0.73838, 0.849817, 0.877976, 0.869728, 0.873935, 0.553882, 0.83905, 0.621658, 0.722701, 0.73315, 0.683995};
    double Eff_BF4[15] = {0.490405, 0.856354, 0.82, 0.845588, 0.733333, 0, 0.963907, 0.96401, 0.852459, 0.574007, 0.736585, 0.773529, 0, 0.773936, 0.771883};
    double Eff_BF5[15] = {0, 0.283128, 0.909091, 0.875224, 0.0368034, 0.89071, 0.898216, 0.895127, 0.877193, 0.904465, 0.936652, 0.203077, 0.857868, 0.865826, 0.806624};
    double Eff_BF6[15] = {0.574825, 0.605797, 0.231481, 0.415459, 0.218605, 0.753846, 0.811533, 0.852476, 0.857749, 0.539116, 0.615248, 0.896259, 0.781475, 0.806358, 0.811833};
    double Eff_BF7[15] = {0, 0.137615, 0.372093, 0.37551, 0.518625, 0.805324, 0.822093, 0.70596, 0.572052, 0.525292, 0.672504, 0.838158, 0.82247, 0.767974, 0.762987};

    double Eff_BB0[15] = {0.0173661, 0.511504, 0.775316, 0.855711, 0.907308, 0.00490998, 0.937226, 0.933579, 0.0379507, 0.850112, 0.74364, 0.89703, 0.0720109, 0.87108, 0.689127};
    double Eff_BB1[15] = {0, 0.167857, 0.622222, 0.764423, 0.573864, 0.8437, 0.859036, 0.556748, 0.81104, 0.656328, 0.669749, 0.808803, 0.797143, 0.645641, 0.467733};
    double Eff_BB2[15] = {0.26357, 0.129688, 0.758065, 0.547105, 0.662222, 0.758564, 0.860849, 0.80853, 0.670051, 0.636285, 0.922764, 0.202169, 0.779736, 0.589841, 0.172103};
    double Eff_BB3[15] = {0.34612, 0.246792, 0.612676, 0.649916, 0.758359, 0.81388, 0.882759, 0.862028, 0.58209, 0.783617, 0.695918, 0.855087, 0.839012, 0.710262, 0.542714};
    double Eff_BB4[15] = {0.383975, 0.405609, 0.511111, 0.370166, 0.378505, 0.829167, 0.871755, 0.885891, 0.855556, 0.603627, 0.695035, 0.866407, 0.806483, 0.607404, 0.68313};
    double Eff_BB5[15] = {0.459225, 0.458082, 0.565359, 0.317967, 0.375375, 0.73494, 0.915799, 0.776042, 0.848387, 0.70607, 0.621698, 0.837285, 0.765024, 0.831115, 0.718705};
    double Eff_BB6[15] = {0.133038, 0.0634518, 0.818505, 0.776632, 0.83959, 0.920382, 0.905817, 0.915014, 0.877493, 0.873563, 0.875776, 0.755906, 0.65625, 0.790476, 0.0493827};
    double Eff_BB7[15] = {0.323093, 0.12311, 0.663677, 0.576842, 0.741093, 0.774687, 0.855208, 0.807381, 0.754821, 0.648686, 0.736682, 0.756126, 0.835646, 0.701065, 0.77396};
    //----------------

    for (int layer = 0; layer <= barrelExtLayer; ++layer) {
      if ((testBit & extLayerPattern) != 0) {
        if ((testBit & hitLayerPattern) != 0) {//checking the presence of a hit in the layer
          pdf *= m_LayerPDF[outcome][lastLayer][layer];
        } else {
          if (((layer == 0) && (outcome < 7)) || (layer == MUID_MaxBarrelLayer) || (layer < barrelExtLayer)) {
            if (nBarrelHit >= 0.8 *
                nBarrelExt) { //if there are more than 80% of hits among the extrapolted path, the track is considered as a muon
              pdf *= m_LayerPDF[outcome][lastLayer][layer];
            } else {
              unsigned int testBitPrev = testBit >> 1;
              unsigned int testBitNext = testBit << 1;
              unsigned int testBitNextNext = testBit << 2;
              if (layer > 0 && ((((testBitPrev & hitLayerPattern) != 0) && ((testBitNext & hitLayerPattern) != 0))
                                || (((testBitNext & hitLayerPattern) != 0)
                                    && ((testBitNextNext & hitLayerPattern) != 0)))) {
                pdf *= m_LayerPDF[outcome][lastLayer][layer];
              } else {
                //treatment of inefficient layers using BKLM efficiencies
                if (isForward) { //select forward BKLM section
                  switch (sector) {
                    case (0):
                      if (Eff_BF0[layer] < 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BF0[layer]) / 6;
                      } else if (Eff_BF0[layer] >= 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BF0[layer]);
                      }
                      break;
                    case (1):
                      if (Eff_BF1[layer] < 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BF1[layer]) / 6;
                      } else if (Eff_BF1[layer] >= 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BF1[layer]);
                      }
                      break;
                    case (2):
                      if (Eff_BF2[layer] < 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BF2[layer]) / 6;
                      } else if (Eff_BF2[layer] >= 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BF2[layer]);
                      }
                      break;
                    case (3):
                      if (Eff_BF3[layer] < 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BF3[layer]) / 6;
                      } else if (Eff_BF3[layer] >= 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BF3[layer]);
                      }
                      break;
                    case (4):
                      if (Eff_BF4[layer] < 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BF4[layer]) / 6;
                      } else if (Eff_BF4[layer] >= 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BF4[layer]);
                      }
                      break;
                    case (5):
                      if (Eff_BF5[layer] < 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BF5[layer]) / 6;
                      } else if (Eff_BF5[layer] >= 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BF5[layer]);
                      }
                      break;
                    case (6):
                      if (Eff_BF6[layer] < 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BF6[layer]) / 6;
                      } else if (Eff_BF6[layer] >= 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BF6[layer]);
                      }
                      break;
                    case (7):
                      if (Eff_BF7[layer] < 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BF7[layer]) / 6;
                      } else if (Eff_BF7[layer] >= 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BF7[layer]);
                      }
                      break;
                  }
                }
                if (!isForward) { //select backward BKLM section
                  switch (sector) {
                    case (0):
                      if (Eff_BB0[layer] < 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BB0[layer]) / 6;
                      } else if (Eff_BB0[layer] >= 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BB0[layer]);
                      }
                      break;
                    case (1):
                      if (Eff_BB1[layer] < 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BB1[layer]) / 6;
                      } else if (Eff_BB1[layer] >= 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BB1[layer]);
                      }
                      break;
                    case (2):
                      if (Eff_BB2[layer] < 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BB2[layer]) / 6;
                      } else if (Eff_BB2[layer] >= 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BB2[layer]);
                      }
                      break;
                    case (3):
                      if (Eff_BB3[layer] < 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BB3[layer]) / 6;
                      } else if (Eff_BB3[layer] >= 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BB3[layer]);
                      }
                      break;
                    case (4):
                      if (Eff_BB4[layer] < 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BB4[layer]) / 6;
                      } else if (Eff_BB4[layer] >= 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BB4[layer]);
                      }
                      break;
                    case (5):
                      if (Eff_BB5[layer] < 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BB5[layer]) / 6;
                      } else if (Eff_BB5[layer] >= 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BB5[layer]);
                      }
                      break;
                    case (6):
                      if (Eff_BB6[layer] < 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BB6[layer]) / 6;
                      } else if (Eff_BB6[layer] >= 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BB6[layer]);
                      }
                      break;
                    case (7):
                      if (Eff_BB7[layer] < 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BB7[layer]) / 6;
                      } else if (Eff_BB7[layer] >= 0.5) {
                        pdf *= m_LayerPDF[outcome][lastLayer][layer] * (1 - Eff_BB7[layer]);
                      }
                      break;
                  }
                }
              }
            }
          }
        }
      }
      testBit <<= 1; // move to next bit
    }

    int nEndcapExt = 0;
    int nEndcapHit = 0;
    int outermostEndcapLayerWithHit = -1;
    unsigned int hitPattern_EKLM = 0;
    for (int layer_new_EKLM = 0; layer_new_EKLM < 14; ++layer_new_EKLM) { // 14 endcap layers
      unsigned int bit = (0x8000 << layer_new_EKLM);
      if ((extLayerPattern & bit) != 0) {
        nEndcapExt += 1;//counting the number of EKLM extrapolated layers
        if ((hitLayerPattern & bit) != 0) {
          outermostEndcapLayerWithHit = layer_new_EKLM;
          nEndcapHit += ((hitLayerPattern & bit) != 0) ? 1 : 0;//counting the number of EKLM layers with hits in the pattern
        }
      }
      // re-write a hit pattern forcing the presence of a hit in each of the layer before the outermost one with a real hit
      if ((nBarrelHit + nEndcapHit) >= 1) {
        for (int layer_new_EKLM_bitPattern = 0; layer_new_EKLM_bitPattern < outermostEndcapLayerWithHit;
             ++layer_new_EKLM_bitPattern) { // 14 endcap layers
          unsigned int bit_pattern_EKLM = (0x8000 << layer_new_EKLM_bitPattern);
          if ((extLayerPattern & bit_pattern_EKLM) != 0) {
            hitPattern_EKLM |= bit_pattern_EKLM;
          }
        }
      }
    }

    int maxLayer = isForward ? MUID_MaxForwardEndcapLayer : MUID_MaxBackwardEndcapLayer;
    testBit = 1 << (MUID_MaxBarrelLayer + 1);
    for (int layer = 0; layer <= endcapExtLayer; ++layer) {
      if ((testBit & extLayerPattern) != 0) {
        if ((testBit & hitPattern_EKLM) != 0) {//checking the presence of a hit in the layer
          pdf *= m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1];
        } else {
          if ((layer == 0) || (layer == maxLayer) || (layer < endcapExtLayer)) {
            //if there are more than 80% of hits among the extrapolted path, the track is considered as a muon
            if (nBarrelHit + nEndcapHit >= 0.8 * (nBarrelExt + nEndcapExt)) {
              pdf *= m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1];
            } else {
              //treatment of inefficient very lasts EKLM layers
              if (isForward) {
                if (layer == 12) {
                  if ((11 & hitPattern_EKLM) != 0) {
                    pdf *= m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1];
                  } else {
                    pdf *= (1.0 - std::min(0.9, m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1]));
                  }
                } else if (layer == 13) {
                  if (((12 & hitPattern_EKLM) != 0) || ((11 & hitPattern_EKLM) != 0)) {
                    pdf *= m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1];
                  } else {
                    pdf *= (1.0 - std::min(0.9, m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1]));
                  }
                } else {
                  pdf *= (1.0 - std::min(0.9, m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1]));
                }
              }
              if (!isForward) {
                if (layer == 10) {
                  if ((9 & hitPattern_EKLM) != 0) {
                    pdf *= m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1];
                  } else {
                    pdf *= (1.0 - std::min(0.9, m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1]));
                  }
                } else if (layer == 11) {
                  if (((10 & hitPattern_EKLM) != 0) || ((9 & hitPattern_EKLM) != 0)) {
                    pdf *= m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1];
                  } else {
                    pdf *= (1.0 - std::min(0.9, m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1]));
                  }
                } else {
                  pdf *= (1.0 - std::min(0.9, m_LayerPDF[outcome][lastLayer][layer + MUID_MaxBarrelLayer + 1]));
                }
              }

            }
          }
        }
      }
      testBit <<= 1; // move to next bit
    }

    //    delete muidHits;
    return pdf;

  }

  double MuidPar::getPDFRchisq(const Muid* muid) const
  {

    // Evaluate the transverse-coordinate PDF for this particleID hypothesis.

    int ndof = muid->getDegreesOfFreedom();
    if (ndof <= 0) return 1.0;
    double chiSquared = muid->getChiSquared();
    if (chiSquared < 0.0) return 0.0;
    int halfNdof = (ndof >> 1);
    double x = chiSquared / ndof;

    // Assume that the track crossed both barrel and endcap
    int detector = 0;
    if (muid->getEndcapExtLayer() < 0) {
      detector = 1; // crossed barrel only
    } else if (muid->getBarrelExtLayer() < 0) {
      detector = 2;  // crossed endcap only
    }

    // Use spline interpolation of the logarithms of the PDF to avoid binning artifacts.
    // Use fitted tail function for reduced-chiSquared values beyond the tabulated threshold.

    double pdf = 0.0;
    if (halfNdof > MUID_MaxHalfNdof) { // extremely rare
      x *= m_ReducedChiSquaredScaleX[detector][MUID_MaxHalfNdof] * halfNdof;
      if (halfNdof == 1) {
        pdf = m_ReducedChiSquaredScaleY[detector][MUID_MaxHalfNdof] * std::exp(-x);
      } else if (halfNdof == 2) {
        pdf = m_ReducedChiSquaredScaleY[detector][MUID_MaxHalfNdof] * x * std::exp(-x);
      } else if (halfNdof == 3) {
        pdf = m_ReducedChiSquaredScaleY[detector][MUID_MaxHalfNdof] * x * x * std::exp(-x);
      } else {
        pdf = m_ReducedChiSquaredScaleY[detector][MUID_MaxHalfNdof] * std::pow(x, halfNdof - 1.0) * std::exp(-x);
      }
    } else {
      if (x > m_ReducedChiSquaredThreshold[detector][halfNdof]) { // tail function for large x
        x *= m_ReducedChiSquaredScaleX[detector][halfNdof] * halfNdof;
        if (halfNdof == 1) {
          pdf = m_ReducedChiSquaredScaleY[detector][halfNdof] * std::exp(-x);
        } else if (halfNdof == 2) {
          pdf = m_ReducedChiSquaredScaleY[detector][halfNdof] * x * std::exp(-x);
        } else if (halfNdof == 3) {
          pdf = m_ReducedChiSquaredScaleY[detector][halfNdof] * x * x * std::exp(-x);
        } else {
          pdf = m_ReducedChiSquaredScaleY[detector][halfNdof] * std::pow(x, halfNdof - 1.0) * std::exp(-x);
        }
      } else { // spline-interpolated histogram for small x
        x -= 0.5 * m_ReducedChiSquaredDx;
        int i  = (int)(x / m_ReducedChiSquaredDx);
        double logPdf = m_ReducedChiSquaredPDF[detector][halfNdof][i];
        double dx = x - i * m_ReducedChiSquaredDx;
        logPdf += dx * (m_ReducedChiSquaredD1[detector][halfNdof][i] +
                        dx * (m_ReducedChiSquaredD2[detector][halfNdof][i] +
                              dx * m_ReducedChiSquaredD3[detector][halfNdof][i]));
        pdf = std::exp(logPdf);
      }
    }
    return pdf;
  }

} // end of namespace Belle2
