/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/mva/PyEstimator.h>
#include <framework/utilities/FileSystem.h>

#include <boost/python/import.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/list.hpp>
#include <boost/python/tuple.hpp>

#include <framework/logging/Logger.h>
#include <cstdio>

using namespace Belle2;
using namespace TrackFindingCDC;

PyEstimator::PyEstimator(const std::string& pickleFileName)
  : m_pickleFileName(pickleFileName)
{
  try {
    // Construct an array with one entry
    // Expand it once the number of variables is known.
    boost::python::object numpy = boost::python::import("numpy");
    boost::python::list initValues;
    initValues.append(0.0);
    m_array = numpy.attr("array")(initValues);
    m_nCurrent = boost::python::len(m_array);
    // boost::python::object array = boost::python::import("array");
    // m_array = array.attr("array")("d");
    unpickleEstimator(pickleFileName);
  } catch (const boost::python::error_already_set&) {
    PyErr_Print();
    B2ERROR("Could not construct PyEstimator from " << pickleFileName);
  }
}

double PyEstimator::predict(const std::vector<double>& inputVariables)
{
  size_t nVars = inputVariables.size();
  expand(nVars);

  for (size_t iVar = 0; iVar < nVars; ++iVar) {
    m_array[boost::python::make_tuple(0, iVar)] = inputVariables[iVar];
  }
  return predict(m_array);
}

double PyEstimator::predict(const std::vector<NamedFloatTuple*>& floatTuples)
{
  size_t nVars = 0;
  for (NamedFloatTuple* floatTuple : floatTuples) {
    nVars += floatTuple->size();
  }
  expand(nVars);
  size_t iVar = 0;

  for (NamedFloatTuple* floatTuple : floatTuples) {
    for (size_t iTuple = 0; iTuple < floatTuple->size(); ++iTuple) {
      m_array[boost::python::make_tuple(0, iVar)] = floatTuple->get(iTuple);
      ++iVar;
    }
  }
  return predict(m_array);
}

double PyEstimator::predict(boost::python::object& array)
{
  boost::python::object predictions;
  try {
    predictions = m_predict(array);
    if (m_is_binary_classification) {
      // In case of a binary classification we take the signal probability
      boost::python::object prediction = predictions[0];
      return boost::python::extract<double>(prediction[1]);
    } else {
      // In case of regression we take the regression value
      boost::python::object prediction = predictions[0];
      return boost::python::extract<double>(prediction);
    }
  } catch (const boost::python::error_already_set&) {
    PyErr_Print();
    B2ERROR("Estimation failed in python object");
  }
  return NAN;
}

void PyEstimator::unpickleEstimator(const std::string& pickleFileName)
{
  try {
    std::string absPickleFilePath = FileSystem::findFile(pickleFileName);
    boost::python::object io = boost::python::import("io");
    boost::python::object pickle = boost::python::import("pickle");
    boost::python::object pickleFile = io.attr("open")(absPickleFilePath, "rb");
    boost::python::object estimator = pickle.attr("load")(pickleFile);
    m_estimator = estimator;
  } catch (const boost::python::error_already_set&) {
    PyErr_Print();
    B2ERROR("Could not open pickle file " << pickleFileName);
  }

  try {
    m_predict = m_estimator.attr("predict_proba");
    m_is_binary_classification = true;
  } catch (const boost::python::error_already_set&) {
    // AttributeError occured, but this is allowed to fail
    // Clear the exception and carry on.
    PyErr_Clear();
    B2INFO("Estimator in " << m_pickleFileName << " is not a binary classifier. Trying as regressor");
    try {
      m_predict = m_estimator.attr("predict");
      m_is_binary_classification = false;
    } catch (const boost::python::error_already_set&) {
      PyErr_Print();
      B2ERROR("Could neither find 'predict' not 'predict_proba' in python estimator from file " << pickleFileName);
    }
  }
}

void PyEstimator::expand(size_t nVars)
{
  m_nCurrent = boost::python::len(m_array);
  if (nVars == m_nCurrent) return;
  try {
    boost::python::object numpy = boost::python::import("numpy");
    boost::python::object shape = boost::python::make_tuple(1, nVars); // one sample with nVars columns
    m_array = numpy.attr("resize")(m_array, shape);
  } catch (const boost::python::error_already_set&) {
    PyErr_Print();
    B2ERROR("Resize failed");
  }
}
