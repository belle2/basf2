/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <analysis/utility/PostProcessingParticleWeighting.h>

/* Basf2 headers. */
#include <framework/database/Database.h>

using namespace Belle2;

PostProcessingParticleWeighting::PostProcessingParticleWeighting(
  const char* lookupTableName) : m_LookupTableName(lookupTableName)
{
}

PostProcessingParticleWeighting::~PostProcessingParticleWeighting()
{
}

void PostProcessingParticleWeighting::setValue(
  std::string variableName, double value)
{
  m_Values[variableName] = value;
}

WeightInfo PostProcessingParticleWeighting::getInfo(int experiment, int run)
{
  if ((experiment != m_Experiment) || (run != m_Run)) {
    m_LookupTable = (ParticleWeightingLookUpTable*)Database::Instance().getData(
                      m_LookupTableName, experiment, run);
  }
  WeightInfo weightInfo = m_LookupTable->getInfo(m_Values);
  return weightInfo;
}

PyObject* PostProcessingParticleWeighting::getInfoPython(
  int experiment, int run)
{
  WeightInfo weightInfo = getInfo(experiment, run);
  PyObject* dictionary = PyDict_New();
  for (const std::pair<std::string, double>& x : weightInfo) {
    PyObject* key = PyUnicode_FromString(x.first.c_str());
    PyObject* value = PyFloat_FromDouble(x.second);
    PyDict_SetItem(dictionary, key, value);
  }
  return dictionary;
}
