/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
// #include <inc/FBDT.h> // externals version (needs an update!)
#include <tracking/trackFindingVXD/tmpFastBDT/FastBDT.h> // temporary copy with necessary changes

#include <tracking/trackFindingVXD/FilterTools/DecorrelationMatrix.h>
#include <array>
#include <vector>
#include <iostream>
#include <iomanip>

namespace Belle2 {

  /** small helper struct */
  template<typename PointType>
  struct FBDTTrainSample {
    std::array<const PointType*, 3> hits;
    bool signal;
  };

  /**
   * FastBDT as RelationsObject to make it storeable and accesible on/via the DataStore
   *
   * PointType: HitType that provides at least accessors to its coordinats via X(), Y() and Z()
   *
   * TODO-List:
   * + Implement Decorrelation efficiently (i.e. matrix-vector multiplication) and store them here
   * + Implement Tests
   * + Longterm: template this properly
   */
  template<typename PointType>
  class FBDTClassifier : public RelationsObject {

  public:

    FBDTClassifier() { ; }// = default; /**< default constructor */

    /** constructor from three main parts. (for testing purposes mainly) */
    FBDTClassifier(FastBDT::Forest forest, std::vector<FastBDT::FeatureBinning<double> > fB, Belle2::DecorrelationMatrix<9> dM) :
      m_forest(forest), m_featBins(fB), m_decorrMat(dM) { ; }

    ~FBDTClassifier() { ; } /**< TODO destructor */

    /** calculate the output of the FastBDT. At the moment fixed to 3 hits */
    double analyze(const std::array<const PointType*, 3>& hits) const;

    /** train the BDT
     * NOTE overwrites a currently existing classifier internally
     * TODO does not work at the moment, look in FastBDT/src/main.cxx how to solve this in all generality
     */
    void train(const std::vector<Belle2::FBDTTrainSample<PointType> >& samples,
               int nTree, int depth, double shrinkage = 0.15, double ratio = 0.5);

    /** read all the necessary data from stream and fill the Forest and the FeatureBinnings
     * NOTE: uses FastBDTs IO stuff. Not compatible with the .xml files that TMVA produces
     */
    void readFromStream(std::istream& is);

    /** write out the data from the Forest and the FeatureBinnings to a stream
     * NOTE: uses FastBDTs IO stuff. Not compatible with the .xml files that TMVA produces
     */
    void writeToStream(std::ostream& os) const;

    /** get the forest */
    FastBDT::Forest getForest() const { return m_forest; }

    /** get the feature binnings */
    std::vector<FastBDT::FeatureBinning<double> > getFeatureBinnings()  const { return m_featBins; }

    /** get the decorrelation matrix */
    Belle2::DecorrelationMatrix<9> getDecorrelationMatrix() const { return m_decorrMat; }

  private:

    FastBDT::Forest m_forest{}; /**< the forest used for classification */

    std::vector<FastBDT::FeatureBinning<double> > m_featBins{}; /**< the feature binnings corresponding to the BDT */

    Belle2::DecorrelationMatrix<9> m_decorrMat{}; /**< the decorrelation matrix used in this classifier */

    // TODO: make this work with the externals -> tell Thomas Keck what is needed for this stuff to work in the externals
    // TODO: there is still some linking problem here. I do not know at the moment how to solve this
    // ClassDef(FBDTClassifier, 1); // first version: only Forest and FeatureBinnings present
  };



  // =================================== IMPLEMENTATION ==============================

  template<typename PointType>
  double FBDTClassifier<PointType>::analyze(const std::array<const PointType*, 3>& hits) const
  {
    std::vector<double> positions(9); // NOTE: hardcoded at the moment
    for (size_t iSP = 0; iSP < hits.size(); ++iSP) {
      positions[iSP * 3] = hits[iSP]->X();
      positions[iSP * 3 + 1] = hits[iSP]->Y();
      positions[iSP * 3 + 2] = hits[iSP]->Z();
    }
    positions = m_decorrMat.decorrelate(positions); // decorrelate the input

    // std::cout << "[";
    // for(double d : positions) std::cout << std::setprecision(20) << d << " ";
    // std::cout << "]" << std::endl;

    size_t nInputs = m_featBins.size();
    std::vector<unsigned> bins(nInputs);
    for (size_t i = 0; i < nInputs; ++i) {
      bins[i] = m_featBins[i].ValueToBin(positions[i]);
    }

    // std::cout << "[";
    // for(auto b : bins) std::cout << b << " ";
    // std::cout << "]" << std::endl;

    return m_forest.Analyse(bins);
  }

} // end namespace Belle2
