/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleDetectorStatsSimTool.h>
#include <framework/datastore/StoreObjPtr.h>
#include <TBranch.h>

#include <arich/dataobjects/ARICHDigit.h>
#include <arich/dataobjects/ARICHSimHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <eklm/dataobjects/EKLMSimHit.h>
#include <eklm/dataobjects/EKLMDigit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDDigit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPSimHit.h>
#include <top/dataobjects/TOPSimPhoton.h>

using namespace Belle2;
using namespace std;

void NtupleDetectorStatsSimTool::setupTree()
{
  m_iARICHDigits = -1;
  m_iARICHSimHits = -1;
  m_iCDCSimHits = -1;
  m_iECLDigits = -1;
  m_iECLSimHits = -1;
  m_iEKLMDigits = -1;
  m_iEKLMSimHits = -1;
  m_iPXDDigits = -1;
  m_iPXDSimHits = -1;
  m_iPXDTrueHits = -1;
  m_iSVDDigits = -1;
  m_iSVDSimHits = -1;
  m_iSVDTrueHits = -1;
  m_iTOPDigits = -1;
  m_iTOPSimHits = -1;
  m_iTOPSimPhotons = -1;

  m_tree->Branch("nARICHDigits",      &m_iARICHDigits,  "nARICHDigits/I");
  m_tree->Branch("nARICHSimHits",   &m_iARICHSimHits,   "nARICHSimHits/I");
  m_tree->Branch("nCDCSimHits",   &m_iCDCSimHits,   "nCDCSimHits/I");
  m_tree->Branch("nECLDigits",          &m_iECLDigits,    "nECLDigits/I");
  m_tree->Branch("nECLSimHits",   &m_iECLSimHits,         "nECLSimHits/I");
  m_tree->Branch("nEKLMDigits",         &m_iEKLMDigits,   "nEKLMDigits/I");
  m_tree->Branch("nEKLMSimHits",  &m_iEKLMSimHits,  "nEKLMSimHits/I");
  m_tree->Branch("nPXDDigits",          &m_iPXDDigits,    "nPXDDigits/I");
  m_tree->Branch("nPXDSimHits",   &m_iPXDSimHits,   "nPXDSimHits/I");
  m_tree->Branch("nPXDTrueHits",  &m_iPXDTrueHits,  "nPXDTrueHits/I");
  m_tree->Branch("nSVDDigits",          &m_iSVDDigits,    "nSVDDigits/I");
  m_tree->Branch("nSVDSimHits",   &m_iSVDSimHits,         "nSVDSimHits/I");
  m_tree->Branch("nSVDTrueHits",        &m_iSVDTrueHits,  "nSVDTrueHits/I");
  m_tree->Branch("nTOPDigits",          &m_iTOPDigits,    "nTOPDigits/I");
  m_tree->Branch("nTOPSimHits",   &m_iTOPSimHits,         "nTOPSimHits/I");
  m_tree->Branch("nTOPSimPhotons",      &m_iTOPSimPhotons,      "nTOPSimPhotons/I");

}

void NtupleDetectorStatsSimTool::eval(const  Particle*)
{

  StoreArray<ARICHDigit>    mARICHDigits;
  m_iARICHDigits = (int) mARICHDigits.getEntries();

  StoreArray<ARICHSimHit>    mARICHSimHits;
  m_iARICHSimHits = (int) mARICHSimHits.getEntries();

  StoreArray<CDCSimHit>    mCDCSimHits;
  m_iCDCSimHits = (int) mCDCSimHits.getEntries();

  StoreArray<ECLDigit>    mECLDigits;
  m_iECLDigits = (int) mECLDigits.getEntries();

  StoreArray<ECLSimHit>    mECLSimHits;
  m_iECLSimHits = (int) mECLSimHits.getEntries();

  StoreArray<EKLMDigit>    mEKLMDigits;
  m_iEKLMDigits = (int) mEKLMDigits.getEntries();

  StoreArray<EKLMSimHit>    mEKLMSimHits;
  m_iEKLMSimHits = (int) mEKLMSimHits.getEntries();

  StoreArray<PXDDigit>    mPXDDigits;
  m_iPXDDigits = (int) mPXDDigits.getEntries();

  StoreArray<PXDSimHit>    mPXDSimHits;
  m_iPXDSimHits = (int) mPXDSimHits.getEntries();

  StoreArray<PXDTrueHit>    mPXDTrueHits;
  m_iPXDTrueHits = (int) mPXDTrueHits.getEntries();

  StoreArray<SVDDigit>    mSVDDigits;
  m_iSVDDigits = (int) mSVDDigits.getEntries();

  StoreArray<SVDSimHit>    mSVDSimHits;
  m_iSVDSimHits = (int) mSVDSimHits.getEntries();

  StoreArray<SVDTrueHit>    mSVDTrueHits;
  m_iSVDTrueHits = (int) mSVDTrueHits.getEntries();

  StoreArray<TOPDigit>    mTOPDigits;
  m_iTOPDigits = (int) mTOPDigits.getEntries();

  StoreArray<TOPSimHit>    mTOPSimHits;
  m_iTOPSimHits = (int) mTOPSimHits.getEntries();

  StoreArray<TOPSimPhoton>    mTOPSimPhotons;
  m_iTOPSimPhotons = (int) mTOPSimPhotons.getEntries();


}
