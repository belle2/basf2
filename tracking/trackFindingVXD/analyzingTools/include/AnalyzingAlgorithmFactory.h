/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

// fw:
#include <framework/logging/Logger.h>

// tracking:
#include<tracking/trackFindingVXD/analyzingTools/AlgoritmType.h>
#include <tracking/trackFindingVXD/analyzingTools/algorithms/AnalyzingAlgorithmBase.h>
#include <tracking/trackFindingVXD/analyzingTools/algorithms/AnalyzingAlgorithmResiduals.h>
#include <tracking/trackFindingVXD/analyzingTools/algorithms/AnalyzingAlgorithmValues.h>
#include <tracking/trackFindingVXD/analyzingTools/algorithms/AnalyzingAlgorithmClusterBased.h>



namespace Belle2 {


  /** the analyzingAlgorithm factory for algorithms returning one double for each TC passed:
   *
   * returns the algorithm matching the given ID.
   */
  template <class DataType, class TCInfoType, class VectorType>
  AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>* AnalyzingAlgorithmFactoryDouble(AlgoritmType::Type algorithmID)
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

    if (algorithmID == AnalyzingAlgorithmValueDistSeed2IPZ<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmValueDistSeed2IPZ<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmValueQI<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmValueQI<DataType, TCInfoType, VectorType>(); }

    if (!AlgoritmType::isValueDoubleType(algorithmID)
        and !AlgoritmType::isResidualDoubleType(algorithmID)
        and (AlgoritmType::isHitValueVecDoubleType(algorithmID)
             or AlgoritmType::isHitValueIntType(algorithmID))) {
      B2WARNING(" AnalyzingAlgorithmFactoryDouble: given algorithmID " << AlgoritmType::getTypeName(algorithmID) <<
                " is no algorithm of double type but of another (valid) category. Please use the correct factory for your purpose. Returning non-functioning base-class instead!");
    } else {
      B2ERROR(" AnalyzingAlgorithmFactoryDouble: given algorithmID " << AlgoritmType::getTypeName(algorithmID) <<
              " is not known, returning non-functioning base-class instead!");
    }

    return new AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>();
  }



  /** the analyzingAlgorithm factory for algorithms returning one int for each TC passed:
   *
   * returns the algorithm matching the given ID.
   */
  template <class DataType, class TCInfoType, class VectorType>
  AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>* AnalyzingAlgorithmFactoryInt(AlgoritmType::Type algorithmID)
  {
    /// residuals
    if (algorithmID == AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmLostUClusters<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmLostUClusters<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmLostVClusters<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmLostVClusters<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmTotalUClusters<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmTotalUClusters<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmTotalVClusters<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmTotalVClusters<DataType, TCInfoType, VectorType>(); }

    if (!AlgoritmType::isHitValueIntType(algorithmID)
        and (AlgoritmType::isValueDoubleType(algorithmID)
             or AlgoritmType::isResidualDoubleType(algorithmID)
             or AlgoritmType::isHitValueVecDoubleType(algorithmID))) {
      B2WARNING(" AnalyzingAlgorithmInt: given algorithmID " << AlgoritmType::getTypeName(algorithmID) <<
                " is no algorithm of int type but of another (valid) category. Please use the correct factory for your purpose. Returning non-functioning base-class instead!");
    } else {
      B2ERROR(" AnalyzingAlgorithmInt: given algorithmID " << AlgoritmType::getTypeName(algorithmID) <<
              " is not known, returning non-functioning base-class instead!");
    }

    return new AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>();
  }



  /** the analyzingAlgorithm factory for algorithms returning one vector< double> for each TC passed:
   *
   * returns the algorithm matching the given ID.
   */
  template <class DataType, class TCInfoType, class VectorType>
  AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>* AnalyzingAlgorithmFactoryVecDouble(AlgoritmType::Type algorithmID)
  {
    /// residuals
    if (algorithmID == AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmLostUEDep<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmLostUEDep<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmLostVEDep<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmLostVEDep<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmTotalUEDep<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmTotalUEDep<DataType, TCInfoType, VectorType>(); }

    if (algorithmID == AnalyzingAlgorithmTotalVEDep<DataType, TCInfoType, VectorType>())
    { return new AnalyzingAlgorithmTotalVEDep<DataType, TCInfoType, VectorType>(); }


    if (!AlgoritmType::isHitValueVecDoubleType(algorithmID)
        and (AlgoritmType::isValueDoubleType(algorithmID)
             or AlgoritmType::isResidualDoubleType(algorithmID)
             or AlgoritmType::isHitValueIntType(algorithmID))) {
      B2WARNING(" AnalyzingAlgorithmVecDouble: given algorithmID " << AlgoritmType::getTypeName(algorithmID) <<
                " is no algorithm of vector<double> type but of another (valid) category. Please use the correct factory for your purpose. Returning non-functioning base-class instead!");
    } else {
      B2ERROR(" AnalyzingAlgorithmVecDouble: given algorithmID " << AlgoritmType::getTypeName(algorithmID) <<
              " is not known, returning non-functioning base-class instead!");
    }

    return new AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>();
  }

}
