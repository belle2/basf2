/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdTimeCalibrationCollector/SVDTimeCalibContainer.h>

using namespace Belle2;



bool SVDTimeCalibContainer::fgAddDirectory = true;

SVDTimeCalibContainer::SVDTimeCalibContainer()
/* : // m_TH1F({}) */
/* // , m_TH2F({}) */
/* // , */
// /* m_name("name") */ {
// for (int ij=0;ij<500;ij++) {
//  m_TH1F[TString::Format("%i",ij)] = new TH1F();
//  m_TH2F[TString::Format("%i",ij)] = new TH2F();
// }
{
  fDirectory = 0;

  histoNames["Surya"] = 0;
  histoNames["Chandrani"] = 1;

  if (AddDirectoryStatus()) {
    fDirectory = gDirectory;
    if (fDirectory) {
      fDirectory->Append(this, true);
    }
  }
}


void SVDTimeCalibContainer::SetDirectory(TDirectory* dir)
{
  // for (auto hist : m_TH1F)
  //  hist.second->SetDirectory(dir);
  // for (auto hist : m_TH2F)
  //  hist.second->SetDirectory(dir);
  if (fDirectory == dir) return;
  if (fDirectory) fDirectory->Remove(this);
  fDirectory = dir;
  if (fDirectory) {
    fDirectory->Append(this);
  }
}



void SVDTimeCalibContainer::Reset()
{
  // m_TH1F.clear();
  // m_TH2F.clear();
  /* m_name = "name"; */
  for (auto hist : m_TH1F)
    hist.second->Reset();
  for (auto hist : m_TH2F)
    hist.second->Reset();
}
