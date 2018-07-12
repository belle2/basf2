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
#include <FastBDT.h>

#include <tracking/trackFindingVXD/filterTools/FBDTClassifierHelper.h>
#include <tracking/trackFindingVXD/filterTools/DecorrelationMatrix.h>
#include <array>
#include <vector>
#include <iostream>
#include <iomanip>

#if FastBDT_VERSION_MAJOR >= 3
typedef FastBDT::Forest<unsigned int> FBDTForest;
#else
typedef FastBDT::Forest FBDTForest;
#endif

namespace Belle2 {

  /**
   * FastBDT as RelationsObject to make it storeable and accesible on/via the DataStore
   *
   * Ndims: number of inputs (dimension of input vector)
   *
   * TODO-List:
   * + Longterm: template this properly
   */
  template<size_t Ndims = 9>
  class FBDTClassifier : public RelationsObject {

  public:

    FBDTClassifier() { ; }// = default; /**< default constructor */

    /** constructor from three main parts. (for testing purposes mainly) */
    FBDTClassifier(FBDTForest forest, std::vector<FastBDT::FeatureBinning<double> > fB, Belle2::DecorrelationMatrix<9> dM) :
      m_forest(forest), m_featBins(fB), m_decorrMat(dM) { ; }

    ~FBDTClassifier() { ; } /**< TODO destructor */

    /** calculate the output of the FastBDT. At the moment fixed to 3 hits */
    double analyze(const std::array<double, Ndims>& hits) const;

    /** train the BDT
     * NOTE overwrites a currently existing classifier internally
     * TODO does not work at the moment, look in FastBDT/src/main.cxx how to solve this in all generality
     */
    void train(const std::vector<Belle2::FBDTTrainSample<Ndims> >& samples,
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
    FBDTForest getForest() const { return m_forest; }

    /** get the feature binnings */
    std::vector<FastBDT::FeatureBinning<double> > getFeatureBinnings()  const { return m_featBins; }

    /** get the decorrelation matrix */
    Belle2::DecorrelationMatrix<9> getDecorrelationMatrix() const { return m_decorrMat; }

  private:

    FBDTForest m_forest{}; /**< the forest used for classification */

    std::vector<FastBDT::FeatureBinning<double> > m_featBins{}; /**< the feature binnings corresponding to the BDT */

    Belle2::DecorrelationMatrix<Ndims> m_decorrMat{}; /**< the decorrelation matrix used in this classifier */

    // TODO: make this work with the externals -> tell Thomas Keck what is needed for this stuff to work in the externals
    /// Making this Class a ROOT class
    ClassDef(FBDTClassifier, 2); // first version: only Forest and FeatureBinnings present
  };



  // =================================== IMPLEMENTATION ==============================

  template<size_t Ndims>
  double FBDTClassifier<Ndims>::analyze(const std::array<double, Ndims>& hits) const
  {
    std::vector<double> positions = m_decorrMat.decorrelate(hits);

    std::vector<unsigned> bins(Ndims);
    for (size_t i = 0; i < Ndims; ++i) {
      bins[i] = m_featBins[i].ValueToBin(positions[i]);
    }

    return m_forest.Analyse(bins);
  }

} // end namespace Belle2
