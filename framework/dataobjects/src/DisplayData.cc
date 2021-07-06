/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
  m_labels.emplace_back(text, pos);
}

void DisplayData::addHistogram(const std::string& name, const TH1* hist)
{
  auto* newhist = static_cast<TH1*>(hist->Clone(name.c_str()));
  newhist->SetDirectory(nullptr);
  m_histograms.push_back(newhist);
}

void DisplayData::select(const TObject* object)
{
  //where is this thing stored?
  DataStore::StoreEntry* storeEntry = nullptr;
  int index = -1;
  DataStore::Instance().findStoreEntry(object, storeEntry, index);

  if (storeEntry) {
    m_selectedObjects.emplace_back(storeEntry->name, index);
  } else {
    B2WARNING("DisplayData::select(): object must be part of a StoreArray!");
  }
}
