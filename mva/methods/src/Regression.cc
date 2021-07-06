/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mva/methods/Regression.h>

#include <vector>

using namespace Belle2;
using namespace MVA;

RegressionDataSet::RegressionDataSet(const GeneralOptions& general_options, Dataset* dataset, double cutValue) :
  Dataset(general_options), m_cutValue(cutValue), m_childDataSet(dataset)
{
}

void RegressionDataSet::loadEvent(unsigned int iEvent)
{
  m_childDataSet->loadEvent(iEvent);
  m_input = m_childDataSet->m_input;
  m_spectators = m_childDataSet->m_spectators;
  m_isSignal = m_childDataSet->m_target >= m_cutValue;
  m_target = m_childDataSet->m_target;
  m_weight = m_childDataSet->m_weight;
}

unsigned int RegressionDataSet::getNumberOfFeatures() const
{
  return m_childDataSet->getNumberOfFeatures();
}

unsigned int RegressionDataSet::getNumberOfEvents() const
{
  return m_childDataSet->getNumberOfEvents();
}

unsigned int RegressionDataSet::getNumberOfSpectators() const
{
  return m_childDataSet->getNumberOfSpectators();
}

std::vector<float> RegressionDataSet::getFeature(unsigned int iFeature)
{
  return m_childDataSet->getFeature(iFeature);
}

std::vector<float> RegressionDataSet::getSpectator(unsigned int iSpectator)
{
  return m_childDataSet->getSpectator(iSpectator);
}

std::vector<float> RegressionDataSet::getWeights()
{
  return m_childDataSet->getWeights();
}

std::vector<float> RegressionDataSet::getTargets()
{
  return m_childDataSet->getTargets();
}
