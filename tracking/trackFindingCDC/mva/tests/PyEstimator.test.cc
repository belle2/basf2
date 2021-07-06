/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <gtest/gtest.h>

#include <tracking/trackFindingCDC/mva/PyEstimator.h>
#include <tracking/trackFindingCDC/utilities/TimeIt.h>

#include <framework/logging/Logger.h>

#include <boost/python/import.hpp>
#include <boost/python/exec.hpp>
#include <boost/python/extract.hpp>
#include "Python.h"
#include <vector>
#include <cstdio>

namespace {
  using namespace Belle2::TrackFindingCDC;
  TEST(TrackFindingCDCTest, PyEstimator_predict_regression)
  {
    Py_Initialize();
    // Create dummy regession object
    std::string first_var_regressor_code = R"code(
class FirstVarRegressor(object):
    def predict(self, ys):
        return ys[0, 0:1]

first_var_regressor = FirstVarRegressor()
import pickle
with open("first_var_regressor.pickle", "wb") as first_var_regressor_file:
    pickle.dump(first_var_regressor, first_var_regressor_file)
)code";

    // Retrieve the globals of the main module.
    boost::python::object main = boost::python::import("__main__");
    boost::python::object global = main.attr("__dict__");

    // Pickle the dummy regressor
    boost::python::exec(first_var_regressor_code.c_str(), global);
    PyEstimator pyRegressor("first_var_regressor.pickle");
    std::vector<double> variables;
    variables.resize(5);
    const double expected = 42.;
    variables[0] = expected;
    try {
      double actual = pyRegressor.predict(variables);
      EXPECT_EQ(expected, actual);
    } catch (...) {
      PyErr_Print();
      throw;
    }
    remove("first_var_regressor.pickle");
  }

  TEST(TrackFindingCDCTest, PyEstimator_predict_classifier)
  {
    Py_Initialize();
    // Create dummy regession object
    std::string first_var_classifier_code = R"code(
import numpy
class FirstVarClassifier(object):
    def predict_proba(self, ys):
        signal_proba = ys[0:1, 0:1]
        bkg_proba = 1.0 - signal_proba
        result = numpy.hstack([bkg_proba, signal_proba])
        return result

first_var_classifier = FirstVarClassifier()
import pickle
with open("first_var_classifier.pickle", "wb") as first_var_classifier_file:
    pickle.dump(first_var_classifier, first_var_classifier_file)
)code";

    // Retrieve the globals of the main module.
    boost::python::object main = boost::python::import("__main__");
    boost::python::object global = main.attr("__dict__");

    // Pickle the dummy classifier
    boost::python::exec(first_var_classifier_code.c_str(), global);
    PyEstimator pyClassifier("first_var_classifier.pickle");
    std::vector<double> variables;
    variables.resize(5);
    const double expected = 0.7;
    variables[0] = expected;
    try {
      double actual = pyClassifier.predict(variables);
      EXPECT_EQ(expected, actual);
    } catch (...) {
      PyErr_Print();
      throw;
    }
    remove("first_var_classifier.pickle");
  }


  TEST(DISABLED_TrackFindingCDCTest, PyEstimator_predict_sklearn_regressor)
  {
    Py_Initialize();
    try {
      boost::python::object main = boost::python::import("sklearn");
    } catch (...) {
      // This test only works for people who installed sklearn
      // Do a pip3 install sklearn to enjoy it.
      PyErr_Clear();
      return;
    }

    // Create dummy regession object
    std::string bdt_regressor_code = R"code(

from sklearn import ensemble
from sklearn import datasets
from sklearn.utils import shuffle
import numpy as np

boston = datasets.load_boston()
x, y = shuffle(boston.data, boston.target, random_state=13)
x = x.astype(np.float64)

offset = int(x.shape[0] * 0.9)
trainX, trainY = x[:offset], y[:offset]
testX, testY = x[offset:], y[offset:]

params = {'n_estimators': 500, 'max_depth': 4, 'min_samples_split': 1,
          'learning_rate': 0.01, 'loss': 'ls'}

clf = ensemble.GradientBoostingRegressor(**params)
clf.fit(trainX, trainY)

import pickle
with open("bdt_regressor.pickle", "wb") as bdt_regressor_file:
    pickle.dump(clf, bdt_regressor_file)
)code";

    // Retrieve the globals of the main module.
    boost::python::object main = boost::python::import("__main__");
    boost::python::object global = main.attr("__dict__");

    // Pickle the dummy classifier
    try {
      boost::python::exec(bdt_regressor_code.c_str(), global);
    } catch (...) {
      PyErr_Print();
      throw;
    }

    try {
      PyEstimator pyClassifier("bdt_regressor.pickle");

      boost::python::object testX = global["testX"];
      boost::python::object shape = testX.attr("shape");
      size_t nRowsTestX = boost::python::extract<size_t>(shape[0]);
      size_t nColsTestX = boost::python::extract<size_t>(shape[1]);

      std::vector<std::vector<double> > knowledgeX(nRowsTestX);
      for (size_t iRowTestX = 0; iRowTestX < nRowsTestX; ++iRowTestX) {
	boost::python::object testXRow = testX[iRowTestX];
	std::vector<double>& variables = knowledgeX[iRowTestX];
	for (size_t iColTestX = 0; iColTestX < nColsTestX; ++iColTestX) {
	  variables.push_back(boost::python::extract<double>(testXRow[iColTestX]));
	}
      }

      std::vector<double> predictions;
      predictions.reserve(nRowsTestX);

      // Make predictions
      auto timeItResult =  timeIt(1, false, [&](){
	  predictions.clear();
	  for (size_t iRowTestX = 0; iRowTestX < nRowsTestX; ++iRowTestX) {
	    const std::vector<double>& variables = knowledgeX[iRowTestX];
	    double prediction = pyClassifier.predict(variables);
	    predictions.push_back(prediction);
	  }
	});
      remove("bdt_regressor.pickle");

      timeItResult.printSummary();
      B2INFO("This might be to slow for serious stuff");

      // Compare prediciton
      boost::python::object testY = global["testY"];
      double squareSum = 0;
      for (size_t iRowTestX = 0; iRowTestX < nRowsTestX; ++iRowTestX) {
	double prediction = predictions[iRowTestX];
	double target =  boost::python::extract<double>(testY[iRowTestX]);
	squareSum += (prediction - target) * (prediction - target);
      }

      double mean_square_error = squareSum / nRowsTestX;
      EXPECT_GT(7, mean_square_error);

    } catch (...) {
      PyErr_Print();
      throw;
    }

  }
}
