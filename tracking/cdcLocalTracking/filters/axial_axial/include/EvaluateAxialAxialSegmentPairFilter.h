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

#include <TFile.h>

#include "MCAxialAxialSegmentPairFilter.h"
#include "AxialAxialSegmentPairFilterTree.h"

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
      /// Getter for the test filter.
      RealAxialAxialSegmentPairFilter& getRealAxialAxialSegmentPairFilter()
      { return m_realAxialAxialSegmentPairFilter; }

      /// Getter for the Monte Carlo reference filter.
      MCAxialAxialSegmentPairFilter& getMCAxialAxialSegmentPairFilter()
      { return m_mcAxialAxialSegmentPairFilter; }

    private:
      RealAxialAxialSegmentPairFilter m_realAxialAxialSegmentPairFilter; ///< Instance of the filter to be compared with Monte Carlo data.
      MCAxialAxialSegmentPairFilter m_mcAxialAxialSegmentPairFilter; ///< Instance of the Monte Carlo filter



    private:
      /// File name of the ROOT output file for further analysis.
      std::string m_outputFileName = "EvaluateAxialAxialSegmentFilter.root";

      /// ROOT output file
      TFile* m_ptrTFileForOutput;

      /// ROOT tree wrapper helping to file the output tree.
      AxialAxialSegmentPairFilterTree m_axialAxialSegmentFilterTree;

    }; // end class EvaluateAxialAxialSegmentPairFilter

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

      m_ptrTFileForOutput = new TFile(m_outputFileName.c_str(), "RECREATE");
      if (m_ptrTFileForOutput) {
        m_axialAxialSegmentFilterTree.create(*m_ptrTFileForOutput);
      }

    }

    template<class RealAxialAxialSegmentPairFilter>
    void EvaluateAxialAxialSegmentPairFilter<RealAxialAxialSegmentPairFilter>::terminate()
    {
      m_axialAxialSegmentFilterTree.save();

      if (m_ptrTFileForOutput != nullptr) {
        m_ptrTFileForOutput->Close();
        delete m_ptrTFileForOutput;
        m_ptrTFileForOutput = nullptr;
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

      // Forget about the Monte Carlo fit
      axialAxialSegmentPair.clearTrajectory2D();

      CellWeight prWeight = getRealAxialAxialSegmentPairFilter().isGoodAxialAxialSegmentPair(axialAxialSegmentPair);

      //do fits
      getRealAxialAxialSegmentPairFilter().getFittedTrajectory2D(*ptrStartSegment);
      getRealAxialAxialSegmentPairFilter().getFittedTrajectory2D(*ptrEndSegment);
      getRealAxialAxialSegmentPairFilter().getFittedTrajectory2D(axialAxialSegmentPair);

      m_axialAxialSegmentFilterTree.setValues(mcWeight, prWeight, axialAxialSegmentPair);
      m_axialAxialSegmentFilterTree.fill();

      return prWeight;

    }

  } //end namespace CDCLocalTracking
} //end namespace Belle2



#endif // EVALUATEAXIALAXIALSEGMENTPAIRFILTER_H_
