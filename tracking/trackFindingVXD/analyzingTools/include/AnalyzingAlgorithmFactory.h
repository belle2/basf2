/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

// fw:
#include <framework/logging/Logger.h>

// tracking:
#include <tracking/trackFindingVXD/analyzingTools/algorithms/AnalyzingAlgorithmBase.h>
#include <tracking/trackFindingVXD/analyzingTools/algorithms/AnalyzingAlgorithmResiduals.h>
#include <tracking/trackFindingVXD/analyzingTools/algorithms/AnalyzingAlgorithmValues.h>



namespace Belle2 {


  /** the analyzingAlgorithm factory.
   *
   * returns the algorithm matching the given ID.
   */
  template <class DataType, class TCInfoType, class VectorType>
  AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>* AnalyzingAlgorithmFactory(std::string algorithmID)
  {
    /// residuals
    if (algorithmID == AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmResidualPX<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmResidualPX<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmResidualPY<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmResidualPY<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmResidualPZ<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmResidualPZ<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmResidualPT<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmResidualPT<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmResidualP<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmResidualP<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmResidualPTheta<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmResidualPTheta<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmResidualPPhi<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmResidualPPhi<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmResidualPAngle<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmResidualPAngle<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmResidualPTAngle<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmResidualPTAngle<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmResidualPosition<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmResidualPosition<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmResidualPositionXY<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmResidualPositionXY<DataType, TCInfoType, VectorType>(); }


    /// values of single entries
    if (algorithmID == AnalyzingAlgorithmValuePX<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmValuePX<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmValuePY<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmValuePY<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmValuePZ<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmValuePZ<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmValuePT<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmValuePT<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmValueP<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmValueP<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmValuePTheta<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmValuePTheta<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmValuePPhi<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmValuePPhi<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmValueDistSeed2IP<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmValueDistSeed2IP<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmValueDistSeed2IPXY<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmValueDistSeed2IPXY<DataType, TCInfoType, VectorType>(); }


    B2ERROR(" AnalyzingAlgorithmFactory: given algorithmID " << algorithmID <<
            " is not known, returning non-functioning base-class instead!")

    return new AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>();
  }
}
