/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/NamedFloatTuple.h>
#include <boost/python/object.hpp>

namespace Belle2 {
  namespace TrackFindingCDC {

    class NamedFloatTuple;

    /// Class to invoke a pretrained python estimator that follows the sklearn interface.
    class PyEstimator {
    public:
      /** Construct the Estimator
       *  @param pickleFileName   Name of the file that contains the pickled estimator object eg from sklearn
       */
      explicit PyEstimator(const std::string& pickleFileName);

      /// Call the predict method of the estimator
      double predict(const std::vector<double>& inputVariables);

      /// Call the predict method of the estimator
      double predict(const std::vector<NamedFloatTuple*>& floatTuples);

      /// Call the predict method of the estimator
      double predict(boost::python::object& array);

    private:
      /// Load the estimator object from the pickled file.
      void unpickleEstimator(const std::string& pickleFileName);

      /// Reserves space for at least n variable in the input array
      void expand(size_t nVars);

    private:
      /// File name of the pickle file that contains the trained estimator
      std::string m_pickleFileName;

      /// Retrained python estimator object
      boost::python::object m_estimator;

      /// Python bound prediction method - cached to avoid repeated lookup
      boost::python::object m_predict;

      /// Array to be served to the estimator
      boost::python::object m_array;

      /// Cache for the current length of the input array
      size_t m_nCurrent = 0;

      /// Internal flag to keep track whether a binary classification with predict_proba is evaluated.
      bool m_is_binary_classification = false;

    };

  }
}
