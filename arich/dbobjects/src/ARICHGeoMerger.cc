/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leonid Burmistrov                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//basf2
#include <arich/dbobjects/ARICHGeoMerger.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <math.h>
#include <iostream>
#include <iomanip>

//root
#include <TVector3.h>

using namespace std;
using namespace Belle2;

void ARICHGeoMerger::print(const std::string& title) const
{

  ARICHGeoBase::print(title);

  cout << "Outer radius of merger PCB assembly envelope : " << getEnvelopeOuterRadius() << endl
       << "Inner radius of merger PCB assembly envelope : " << getEnvelopeInnerRadius() << endl
       << "Thickness of merger PCB assembly envelope    : " << getEnvelopeThickness() << endl;
  cout << "X0 merger PCB assembly envelope              : " << getEnvelopeCenterPosition().X() << endl
       << "Y0 merger PCB assembly envelope              : " << getEnvelopeCenterPosition().Y() << endl
       << "Z0 merger PCB assembly envelope              : " << getEnvelopeCenterPosition().Z() << endl;
  cout << "merger PCB material name                     : " << getMergerPCBMaterialName() << endl
       << "merger PCB lenght                            : " << getSizeL() << endl
       << "merger PCB width                             : " << getSizeW() << endl
       << "merger PCB thickness                         : " << getThickness() << endl;

  //checkMergerPositionsDataConsistency();

  cout << setw(20) << " #" << setw(20) << "mergerSlotID" << setw(20) << "mergerPosR" << setw(20) << "mergerAngle" << endl;
  for (unsigned i = 0; i < getMergerSlotID().size(); i++) {
    //cout<<setw(20)<<i<<setw(20)<<m_mergerSlotID.at(i)<<setw(20)<<m_mergerPosR.at(i)<<setw(20)<<m_mergerAngle.at(i)<<endl;
    cout << setw(20) << i << setw(20) << getMergerSlotID().at(i) << setw(20) << getMergerPosR().at(i) << setw(
           20) << getMergerAngle().at(i) << endl;
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

}
