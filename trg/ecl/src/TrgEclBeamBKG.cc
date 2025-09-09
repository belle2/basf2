/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#define TRG_SHORT_NAMES
#define TRGECLCLUSTER_SHORT_NAMES
#include <framework/gearbox/Unit.h>

#include <trg/ecl/TrgEclBeamBKG.h>

using namespace std;
using namespace Belle2;
//
//
//
TrgEclBeamBKG::TrgEclBeamBKG()
{
  for (int iii = 0; iii <= 2; iii++) {
    for (int jjj = 0; jjj <= 3; jjj++) {
      m_Quadrant[iii][jjj] = 0;
    }
  }
  m_TCMap = new TrgEclMapping();

}
//
//
//
TrgEclBeamBKG::~TrgEclBeamBKG()
{

  delete m_TCMap;
}
//
//
//
int TrgEclBeamBKG::getBeamBkg(std::vector<std::vector<double>> ThetaRingSum)
{

  // forward
  for (int iFwd = 0 ; iFwd < 32 ; iFwd++) {
    int phiid = iFwd + 1;
    if (ThetaRingSum[0][iFwd] > 0) {
      if ((phiid >= 1 && phiid <= 9) || phiid == 32) {
        m_Quadrant[0][0] = 1;
      }
      if (phiid >= 8 && phiid <= 17) {
        m_Quadrant[0][1] = 1;
      }
      if (phiid >= 16 && phiid <= 25) {
        m_Quadrant[0][2] = 1;
      }
      // cppcheck-suppress knownConditionTrueFalse
      if ((phiid >= 24 && phiid <= 32) || phiid == 1) {
        m_Quadrant[0][3] = 1;
      }
    }
  }
  // barrel
  for (int iBr = 0 ; iBr < 36 ; iBr++) {
    int phiid = iBr + 1;
    if (ThetaRingSum[1][iBr] > 0) {
      if ((phiid >= 1 && phiid <= 10) || phiid == 36) {
        m_Quadrant[1][0] = 1;
      }
      if (phiid >= 9 && phiid <= 19) {
        m_Quadrant[1][1] = 1;
      }
      if (phiid >= 18 && phiid <= 28) {
        m_Quadrant[1][2] = 1;
      }
      // cppcheck-suppress knownConditionTrueFalse
      if ((phiid >= 27 && phiid <= 36) || phiid == 1) {
        m_Quadrant[1][3] = 1;
      }
    }
  }
  // selection for forward endcap
  bool boolForward =
    ((m_Quadrant[0][0] && m_Quadrant[0][2]) ||
     (m_Quadrant[0][1] && m_Quadrant[0][3]));
  // selection for barrel
  bool boolBarrel =
    ((m_Quadrant[1][0] && m_Quadrant[1][2]) ||
     (m_Quadrant[1][1] && m_Quadrant[1][3]));
  // bkg bit selection
  int iBeamBkgVeto0 = 0;
  int iBeamBkgVeto1 = 0;
  int iBeamBkgVeto2 = 0;
  if (boolForward || boolBarrel) {
    iBeamBkgVeto0 = 1;
  }
  if ((!boolForward) && boolBarrel) {
    iBeamBkgVeto1 = 1;
  }
  if (boolForward && (!boolBarrel)) {
    iBeamBkgVeto2 = 1;
  }
  // set bkg bit parameter
  int BeamBkgVeto = 0;
  BeamBkgVeto |= (iBeamBkgVeto2 & 0x01);
  BeamBkgVeto <<= 1;
  BeamBkgVeto |= (iBeamBkgVeto1 & 0x01);
  BeamBkgVeto <<= 1;
  BeamBkgVeto |= (iBeamBkgVeto0 & 0x01);

  return BeamBkgVeto;
}
//
//
//
