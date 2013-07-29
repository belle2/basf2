/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <display/dataobjects/DisplayData.h>

using namespace Belle2;

ClassImp(DisplayData)

DisplayData::~DisplayData()
{
  m_pointSets.clear();
  m_labels.clear();
  for (unsigned int i = 0; i < m_histograms.size(); i++)
    delete m_histograms[i];
  m_histograms.clear();
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
