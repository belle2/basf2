/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//basf2
#include <arich/dbobjects/ARICHGeoMerger.h>
#include <framework/logging/Logger.h>
#include <iostream>
#include <iomanip>

//root
#include <TVector3.h>

using namespace std;
using namespace Belle2;

void ARICHGeoMerger::print(const std::string& title) const
{

  ARICHGeoBase::print(title);

  cout << "Outer radius of merger PCB assembly envelope            : " << getEnvelopeOuterRadius() << endl
       << "Inner radius of merger PCB assembly envelope            : " << getEnvelopeInnerRadius() << endl
       << "Thickness of merger PCB assembly envelope               : " << getEnvelopeThickness() << endl;
  cout << "X0 merger PCB assembly envelope                         : " << getEnvelopeCenterPosition().X() << endl
       << "Y0 merger PCB assembly envelope                         : " << getEnvelopeCenterPosition().Y() << endl
       << "Z0 merger PCB assembly envelope                         : " << getEnvelopeCenterPosition().Z() << endl;
  cout << "merger PCB material name                                : " << getMergerPCBMaterialName() << endl
       << "merger PCB lenght                                       : " << getSizeL() << endl
       << "merger PCB width                                        : " << getSizeW() << endl
       << "merger PCB thickness                                    : " << getThickness() << endl;
  cout << "merger PCB screw whole radius                           : " << getMergerPCBscrewholeR() << endl
       << "merger PCB screw whole position from the left and right sides : " << getMergerPCBscrewholePosdY() << endl
       << "merger PCB screw whole position from the bottom edge    : " << getMergerPCBscrewholePosdX1() << endl
       << "merger PCB screw whole position from the top edge       : " << getMergerPCBscrewholePosdX2() << endl;
  cout << "single merger PCB and merger cooling envelope length    : " << getSingleMergerEnvelopeSizeL() << endl
       << "single merger PCB and merger cooling envelope width     : " << getSingleMergerEnvelopeSizeW() << endl
       << "single merger PCB and merger cooling envelope thickness : " << getSingleMergerEnvelopeThickness() << endl;
  cout << "X0 of merger PCB inside the single merger envelope      : " << getSingleMergeEnvelopePosition().X() << endl
       << "Y0 of merger PCB inside the single merger envelope      : " << getSingleMergeEnvelopePosition().Y() << endl
       << "Z0 of merger PCB inside the single merger envelope      : " << getSingleMergeEnvelopePosition().Z() << endl;

  //checkMergerPositionsDataConsistency();

  cout << setw(20) << "#"
       << setw(20) << "mergerSlotID"
       << setw(20) << "mergerPosR"
       << setw(20) << "mergerAngle"
       << setw(20) << "mergerOrientation"
       << setw(20) << "mergerDeltaZ" << endl;
  for (unsigned i = 0; i < getMergerSlotID().size(); i++) {
    cout << setw(20) << i
         << setw(20) << getMergerSlotID().at(i)
         << setw(20) << getMergerPosR().at(i)
         << setw(20) << getMergerAngle().at(i)
         << setw(20) << getMergerOrientation().at(i)
         << setw(20) << getSingleMergerenvelopeDeltaZ().at(i)
         << endl;
  }
}

void ARICHGeoMerger::checkMergerPositionsDataConsistency() const
{

  //if(m_mergerSlotID.size() != m_mergerPosR.size()){
  //string outMessage = " Data of the merger board positions is inconsisten : m_mergerSlotID.size() != m_mergerPosR.size() ";
  //B2ERROR( outMessage << endl );
  //B2FATAL(outMessage);
  //}
  //if(m_mergerSlotID.size() != m_mergerAngle.size()){
  //string outMessage = " Data of the merger board positions is inconsisten : m_mergerSlotID.size() != m_mergerAngle.size() ";
  //B2ERROR( outMessage << endl );
  //B2FATAL(outMessage);
  //}

  B2ASSERT("Data of the merger board positions is inconsisten : getEnvelopeThickness() <= getThickness() ",
           getEnvelopeThickness() > getThickness());
  B2ASSERT("Data of the merger board positions is inconsisten : m_mergerSlotID.size() != m_mergerPosR.size()",
           m_mergerSlotID.size() == m_mergerPosR.size());
  B2ASSERT("Data of the merger board positions is inconsisten : m_mergerSlotID.size() != m_mergerAngle.size()",
           m_mergerSlotID.size() == m_mergerAngle.size());
  B2ASSERT("Data of the merger board positions is inconsisten : m_mergerSlotID.size() != m_singlemergerenvelopeDeltaZ.size()",
           m_mergerSlotID.size() == m_singlemergerenvelopeDeltaZ.size());

}
