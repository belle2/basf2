/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVALUATEAXIALSTEREOSEGMENTPAIRFILTER_H_
#define EVALUATEAXIALSTEREOSEGMENTPAIRFILTER_H_

#include <TFile.h>

#include "MCAxialStereoSegmentPairFilter.h"
#include "AxialStereoSegmentPairFilterTree.h"

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    template<class RealAxialStereoSegmentPairFilter>
    class EvaluateAxialStereoSegmentPairFilter {

    public:

      /// Constructor setting up an out file and a tree to be filled
      EvaluateAxialStereoSegmentPairFilter();

      /// Deconstructor writing the tree to disk and closing the file
      ~EvaluateAxialStereoSegmentPairFilter();

    public:

      /// Clears all remember information from the last event
      void clear();

      /// Forwards the modules initialize to the filter
      void initialize();

      /// Forwards the modules initialize to the filter
      void terminate();

      /// Checks if a pair of axial segments is a good combination
      CellWeight isGoodAxialStereoSegmentPair(const CDCAxialStereoSegmentPair& axialStereoSegmentPair);

    private:
      /// Getter for the test filter.
      RealAxialStereoSegmentPairFilter& getRealAxialStereoSegmentPairFilter()
      { return m_realAxialStereoSegmentPairFilter; }

      /// Getter for the Monte Carlo reference filter.
      MCAxialStereoSegmentPairFilter& getMCAxialStereoSegmentPairFilter()
      { return m_mcAxialStereoSegmentPairFilter; }

    private:
      RealAxialStereoSegmentPairFilter m_realAxialStereoSegmentPairFilter; ///< Instance of the filter to be compared with Monte Carlo data.
      MCAxialStereoSegmentPairFilter m_mcAxialStereoSegmentPairFilter; ///< Instance of the Monte Carlo filter



    private:
      /// File name of the ROOT output file for further analysis.
      std::string m_outputFileName = "EvaluateAxialStereoSegmentFilter.root";

      /// ROOT output file
      TFile* m_ptrTFileForOutput;

      /// ROOT tree wrapper helping to file the output tree.
      AxialStereoSegmentPairFilterTree m_axialStereoSegmentFilterTree;

    }; // end class EvaluateAxialStereoSegmentPairFilter

    template<class RealAxialStereoSegmentPairFilter>
    EvaluateAxialStereoSegmentPairFilter<RealAxialStereoSegmentPairFilter>::EvaluateAxialStereoSegmentPairFilter()
    {
    }


    template<class RealAxialStereoSegmentPairFilter>
    EvaluateAxialStereoSegmentPairFilter<RealAxialStereoSegmentPairFilter>::~EvaluateAxialStereoSegmentPairFilter()
    {
    }

    template<class RealAxialStereoSegmentPairFilter>
    void EvaluateAxialStereoSegmentPairFilter<RealAxialStereoSegmentPairFilter>::clear()
    {
      m_realAxialStereoSegmentPairFilter.clear();
      m_mcAxialStereoSegmentPairFilter.clear();
      //Nothing to do...
    }

    template<class RealAxialStereoSegmentPairFilter>
    void EvaluateAxialStereoSegmentPairFilter<RealAxialStereoSegmentPairFilter>::initialize()
    {
      m_mcAxialStereoSegmentPairFilter.initialize();
      m_realAxialStereoSegmentPairFilter.initialize();

      m_ptrTFileForOutput = new TFile(m_outputFileName.c_str(), "RECREATE");
      if (m_ptrTFileForOutput) {
        m_axialStereoSegmentFilterTree.create(*m_ptrTFileForOutput);
      }

    }

    template<class RealAxialStereoSegmentPairFilter>
    void EvaluateAxialStereoSegmentPairFilter<RealAxialStereoSegmentPairFilter>::terminate()
    {
      m_axialStereoSegmentFilterTree.save();

      if (m_ptrTFileForOutput != nullptr) {
        m_ptrTFileForOutput->Close();
        delete m_ptrTFileForOutput;
        m_ptrTFileForOutput = nullptr;
      }

      m_realAxialStereoSegmentPairFilter.terminate();
      m_mcAxialStereoSegmentPairFilter.terminate();

    }



    template<class RealAxialStereoSegmentPairFilter>
    CellWeight EvaluateAxialStereoSegmentPairFilter<RealAxialStereoSegmentPairFilter>::isGoodAxialStereoSegmentPair(const CDCAxialStereoSegmentPair& axialStereoSegmentPair)
    {

      const CDCAxialRecoSegment2D* ptrStartSegment = axialStereoSegmentPair.getStartSegment();
      const CDCAxialRecoSegment2D* ptrEndSegment = axialStereoSegmentPair.getEndSegment();

      if (ptrStartSegment == nullptr) {
        B2ERROR("EvaluateAxialStereoSegmentPairFilter::isGoodAxialStereoSegmentPair invoked with nullptr as start segment");
        return NOT_A_CELL;
      }

      if (ptrEndSegment == nullptr) {
        B2ERROR("EvaluateAxialStereoSegmentPairFilter::isGoodAxialStereoSegmentPair invoked with nullptr as end segment");
        return NOT_A_CELL;
      }


      bool allowBackward = true;
      CellWeight mcWeight = getMCAxialStereoSegmentPairFilter().isGoodAxialStereoSegmentPair(axialStereoSegmentPair, allowBackward);

      // Forget about the Monte Carlo fit
      axialStereoSegmentPair.clearTrajectory3D();

      CellWeight prWeight = getRealAxialStereoSegmentPairFilter().isGoodAxialStereoSegmentPair(axialStereoSegmentPair);

      //do fits and store them.
      getRealAxialStereoSegmentPairFilter().getFittedTrajectory2D(*ptrStartSegment);
      getRealAxialStereoSegmentPairFilter().getFittedTrajectory2D(*ptrEndSegment);
      getRealAxialStereoSegmentPairFilter().getFittedTrajectory3D(axialStereoSegmentPair);

      m_axialStereoSegmentFilterTree.setValues(mcWeight, prWeight, axialStereoSegmentPair);
      m_axialStereoSegmentFilterTree.fill();

      return prWeight;

    }

  } //end namespace CDCLocalTracking
} //end namespace Belle2



#endif // EVALUATEAXIALSTEREOSEGMENTPAIRFILTER_H
