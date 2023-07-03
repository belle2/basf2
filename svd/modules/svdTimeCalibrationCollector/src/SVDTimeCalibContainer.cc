/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdTimeCalibrationCollector/SVDTimeCalibContainer.h>

#include "TDirectory.h"
#include "TH1F.h"
#include "TH2F.h"

#include <iostream>

using namespace Belle2;



bool SVDTimeCalibContainer::fgAddDirectory = true;

SVDTimeCalibContainer::SVDTimeCalibContainer(TString name = "name", TString title = "title")
  : TNamed(name, title)
{
  fDirectory = nullptr;

  if (AddDirectoryStatus()) {
    fDirectory = gDirectory;
    if (fDirectory) {
      fDirectory->Append(this, true);
    }
  }
}

SVDTimeCalibContainer::~SVDTimeCalibContainer()
{
  // for (auto hist : m_TH1F)
  //   delete hist.second;
  // for (auto hist : m_TH2F)
  //   delete hist.second;
  // if (fDirectory) {
  //   fDirectory->Remove(this);
  //   fDirectory = nullptr;
  // }
}


void SVDTimeCalibContainer::SetDirectory(TDirectory* dir)
{
  if (fDirectory == dir) return;
  if (fDirectory) fDirectory->Remove(this);
  fDirectory = dir;
  if (fDirectory) {
    fDirectory->Append(this);
  }
  for (auto hist : m_TH1F)
    hist.second->SetDirectory(fDirectory);
  for (auto hist : m_TH2F)
    hist.second->SetDirectory(fDirectory);
}



void SVDTimeCalibContainer::Reset()
{
  for (auto hist : m_TH1F)
    hist.second->Reset();
  for (auto hist : m_TH2F)
    hist.second->Reset();
}


TObject* SVDTimeCalibContainer::Clone(const char* newname) const
{
  SVDTimeCalibContainer* obj = new SVDTimeCalibContainer(newname);
  Copy(*obj);

  if (newname && strlen(newname)) {
    obj->SetName(newname);
  }
  return obj;
}

void SVDTimeCalibContainer::Copy(TObject& obj) const
{
  if (((SVDTimeCalibContainer&)obj).fDirectory) {
    ((SVDTimeCalibContainer&)obj).fDirectory->Remove(&obj);
    ((SVDTimeCalibContainer&)obj).fDirectory = nullptr;
  }
  TNamed::Copy(obj);
  ((SVDTimeCalibContainer&)obj).m_TH1F = m_TH1F;
  ((SVDTimeCalibContainer&)obj).m_TH2F = m_TH2F;
  if (fgAddDirectory && gDirectory) {
    gDirectory->Append(&obj);
    ((SVDTimeCalibContainer&)obj).fDirectory = gDirectory;
  } else
    ((SVDTimeCalibContainer&)obj).fDirectory = nullptr;
}

void SVDTimeCalibContainer::SetTH1FHistogram(TH1F* hist)
{
  // hist->SetDirectory(fDirectory);
  m_TH1F[TString(hist->GetName())] = hist;
}


void SVDTimeCalibContainer::SetTH2FHistogram(TH2F* hist)
{
  // hist->SetDirectory(fDirectory);
  m_TH2F[hist->GetName()] = hist;
}
