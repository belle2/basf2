/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/dataobjects/DisplayData.h>

#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

DisplayData::~DisplayData()
{
  m_pointSets.clear();
  m_labels.clear();
  for (auto hist : m_histograms)
    delete hist;
  m_histograms.clear();
}

void DisplayData::addArrow(const std::string& name, const TVector3& start, const TVector3& end, int color)
{
  m_arrows.push_back(Arrow {name, start, end, color});
}

void DisplayData::addPoint(const std::string& name, const TVector3& pos)
{
  m_pointSets[name].push_back(pos);
}

void DisplayData::addLabel(const std::string& text, const TVector3& pos)
{
  m_labels.push_back(std::make_pair(text, pos));
}

void DisplayData::addHistogram(const std::string& name, const TH1* hist)
{
  TH1* newhist = static_cast<TH1*>(hist->Clone(name.c_str()));
  newhist->SetDirectory(0);
  m_histograms.push_back(newhist);
}

void DisplayData::select(const TObject* object)
{
  //where is this thing stored?
  DataStore::StoreEntry* storeEntry = 0;
  int index = -1;
  DataStore::Instance().findStoreEntry(object, storeEntry, index);

  if (storeEntry) {
    m_selectedObjects.push_back(std::make_pair(storeEntry->name, index));
  } else {
    B2WARNING("DisplayData::select(): object must be part of a StoreArray!");
  }
}
