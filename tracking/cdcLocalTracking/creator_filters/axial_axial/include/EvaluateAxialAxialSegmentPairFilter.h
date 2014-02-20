/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVALUATEAXIALAXIALSEGMENTPAIRFILTER_H_
#define EVALUATEAXIALAXIALSEGMENTPAIRFILTER_H_

#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>

#include <root/TFile.h>
#include <root/TTree.h>
#include <root/TVector2.h>

#include "MCAxialAxialSegmentPairFilter.h"
#include "AxialAxialSegmentPairFilterVars.h"

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    template<class RealAxialAxialSegmentPairFilter>
    class EvaluateAxialAxialSegmentPairFilter {

    public:

      /// Constructor setting up an out file and a tree to be filled
      EvaluateAxialAxialSegmentPairFilter();

      /// Deconstructor writing the tree to disk and closing the file
      ~EvaluateAxialAxialSegmentPairFilter();

    public:

      /// Clears all remember information from the last event
      void clear();

      /// Forwards the modules initialize to the filter
      void initialize();

      /// Forwards the modules initialize to the filter
      void terminate();




      /// Checks if a pair of axial segments is a good combination
      CellWeight isGoodAxialAxialSegmentPair(const CDCAxialAxialSegmentPair& axialAxialSegmentPair);

    private:
      RealAxialAxialSegmentPairFilter& getRealAxialAxialSegmentPairFilter()
      { return m_realAxialAxialSegmentPairFilter; }

      MCAxialAxialSegmentPairFilter& getMCAxialAxialSegmentPairFilter()
      { return m_mcAxialAxialSegmentPairFilter; }

    private:
      RealAxialAxialSegmentPairFilter m_realAxialAxialSegmentPairFilter; //< filter to be compared with Monte Carlo data.
      MCAxialAxialSegmentPairFilter m_mcAxialAxialSegmentPairFilter; //< monte carlo filter



    private:
      void setupTree();

      std::string m_outputFileName = "EvaluateAxialAxialSegmentFilter.root";

      TFile* m_outputFile;
      TTree* m_tree;

      Bool_t m_truth;
      Float_t m_weight;

      AxialAxialSegmentPairFilterVars m_axialAxialSegmentFilterVars;

    }; // end class EvaluateAxialAxialSegmentPairFilter

    template<class RealAxialAxialSegmentPairFilter>
    void EvaluateAxialAxialSegmentPairFilter<RealAxialAxialSegmentPairFilter>::setupTree()
    {

      m_tree->Branch("truth", &m_truth, "truth/O");
      m_tree->Branch("weight", &m_weight, "weight/F");

      Int_t bufferSize = 32000;
      Int_t split = 1; //Split the object in branches such that we do not need the class to read it.

      m_tree->Branch("vars", &m_axialAxialSegmentFilterVars, bufferSize, split);

    }



    template<class RealAxialAxialSegmentPairFilter>
    EvaluateAxialAxialSegmentPairFilter<RealAxialAxialSegmentPairFilter>::EvaluateAxialAxialSegmentPairFilter()
    {
    }



    template<class RealAxialAxialSegmentPairFilter>
    EvaluateAxialAxialSegmentPairFilter<RealAxialAxialSegmentPairFilter>::~EvaluateAxialAxialSegmentPairFilter()
    {
    }



    template<class RealAxialAxialSegmentPairFilter>
    void EvaluateAxialAxialSegmentPairFilter<RealAxialAxialSegmentPairFilter>::clear()
    {

      m_realAxialAxialSegmentPairFilter.clear();
      m_mcAxialAxialSegmentPairFilter.clear();
      //Nothing to do...
    }



    template<class RealAxialAxialSegmentPairFilter>
    void EvaluateAxialAxialSegmentPairFilter<RealAxialAxialSegmentPairFilter>::initialize()
    {
      m_mcAxialAxialSegmentPairFilter.initialize();
      m_realAxialAxialSegmentPairFilter.initialize();

      m_outputFile = new TFile(m_outputFileName.c_str(), "RECREATE");
      m_tree = new TTree("axial_axial", "Variables to select correct axial to axial segment combinations and the mc truth.");
      setupTree();

    }



    template<class RealAxialAxialSegmentPairFilter>
    void EvaluateAxialAxialSegmentPairFilter<RealAxialAxialSegmentPairFilter>::terminate()
    {
      if (m_tree != nullptr) {
        m_tree->Write();
        delete m_tree;
        m_tree = nullptr;
      }
      if (m_outputFile != nullptr) {
        m_outputFile->Close();
        delete m_outputFile;
        m_outputFile = nullptr;
      }

      m_realAxialAxialSegmentPairFilter.terminate();
      m_mcAxialAxialSegmentPairFilter.terminate();

    }



    template<class RealAxialAxialSegmentPairFilter>
    CellWeight EvaluateAxialAxialSegmentPairFilter<RealAxialAxialSegmentPairFilter>::isGoodAxialAxialSegmentPair(const CDCAxialAxialSegmentPair& axialAxialSegmentPair)
    {

      const CDCAxialRecoSegment2D* ptrStartSegment = axialAxialSegmentPair.getStart();
      const CDCAxialRecoSegment2D* ptrEndSegment = axialAxialSegmentPair.getEnd();

      if (ptrStartSegment == nullptr) {
        B2ERROR("EvaluateAxialAxialSegmentPairFilter::isGoodAxialAxialSegmentPair invoked with nullptr as start segment");
        return NOT_A_CELL;
      }

      if (ptrEndSegment == nullptr) {
        B2ERROR("EvaluateAxialAxialSegmentPairFilter::isGoodAxialAxialSegmentPair invoked with nullptr as end segment");
        return NOT_A_CELL;
      }


      CellWeight mcWeight = getMCAxialAxialSegmentPairFilter().isGoodAxialAxialSegmentPair(axialAxialSegmentPair);
      bool mcDecision = not isNotACell(mcWeight);

      // Forget about the Monte Carlo fit
      axialAxialSegmentPair.clearTrajectory2D();

      CellWeight realWeight = getRealAxialAxialSegmentPairFilter().isGoodAxialAxialSegmentPair(axialAxialSegmentPair);
      //bool realDecision = not isNotACell(realWeight);

      m_axialAxialSegmentFilterVars.fill(axialAxialSegmentPair);

      m_truth = mcDecision;
      m_weight = mcWeight;

      m_tree->Fill();

      return realWeight;

    }

  } //end namespace CDCLocalTracking
} //end namespace Belle2



#endif // EVALUATEAXIALAXIALSEGMENTPAIRFILTER_H_
