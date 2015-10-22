/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Enumerator for storing current pass of track finding
     */
    enum class LegendreFindingPass {
      NonCurlers, /**< Pass corresponds to High-pt track finding and more deeper quadtree*/
      NonCurlersWithIncreasingThreshold, /**< Pass corresponds to High-pt track finding and more rough quadtree*/
      FullRange, /**< Pass corresponds to full pt range and even more rough quadtree (non-ip tracks, tracks with energy losses etc) */
      End
    };

    /**
     * Class which controls passes of track finding
     */
    class QuadTreePassCounter {

    public:

      /// Constructor
      QuadTreePassCounter()
      {
        reset();
      };

      /// Reset pass counter, should be called at the beginning of each event
      void reset()
      {
        m_legendreFindingPass = LegendreFindingPass::NonCurlers;
      }

      /// Set pass
      void setPass(LegendreFindingPass passToSet)
      {
        m_legendreFindingPass = passToSet;
      }

      /// Check if in current pass will be performed curlers (pow-pt) finding
      bool doCurlerFinding() const
      {
        return (m_legendreFindingPass == LegendreFindingPass::NonCurlersWithIncreasingThreshold)
               || (m_legendreFindingPass == LegendreFindingPass::FullRange) ;
      };

      /// Check if in current pass will be increased threshold on curvature
      bool doIncreaseThreshold() const { return m_legendreFindingPass == LegendreFindingPass::NonCurlersWithIncreasingThreshold;};

      /// Get current pass
      LegendreFindingPass& getPass() {return m_legendreFindingPass;};

      /// Change to next pass
      bool nextPass()
      {
        if (m_legendreFindingPass == LegendreFindingPass::NonCurlers)
          m_legendreFindingPass = LegendreFindingPass::NonCurlersWithIncreasingThreshold;
        else if (m_legendreFindingPass == LegendreFindingPass::NonCurlersWithIncreasingThreshold)
          m_legendreFindingPass = LegendreFindingPass::FullRange;
        else if (m_legendreFindingPass == LegendreFindingPass::FullRange) {
          m_legendreFindingPass = LegendreFindingPass::End;
          return false;
        }

        return true;
      };


    private:

      LegendreFindingPass m_legendreFindingPass; /**< Member containing LegendreFindingPass object*/

    };

  }

}

