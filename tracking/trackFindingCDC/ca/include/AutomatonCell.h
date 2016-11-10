/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once


#include <tracking/trackFindingCDC/ca/CellWeight.h>
#include <tracking/trackFindingCDC/ca/CellState.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Cell used by the cellular automata.
    /** This class represents a cell in the cellular automata algorithm the local tracking is build on.
     *  Different to the ordinary cellular automaton algorihms this class stores the state not as a interger number but \n
     *  as a float value. This enables to have fractional measures of quality attached to the cell if say one new item \n
     *  is not worth a full point, but a probability measure which is additiv. The points that can be gained by picking \n
     *  this cell are stored in the cell weight property. Moreover the class defines status flags to be set and/or read by\n
     *  the cellular automaton. */
    class AutomatonCell  {

    public:
      /// Type for the status flags of cells in the cellular automata
      enum ECellFlag : unsigned int {
        c_Assigned = 1,
        c_Start = 2,
        c_Cycle = 4,
        c_Masked = 8,

        c_Taken = 16,
        c_Background = 32,
      };

      /// Type for an ored combination of the status flags of cells in the cellular automata
      using ECellFlags = ECellFlag;

      /// Constant summing all possible cell flags
      static const ECellFlags c_AllFlags = ECellFlags(ECellFlag::c_Assigned +
                                                      ECellFlag::c_Start +
                                                      ECellFlag::c_Cycle +
                                                      ECellFlag::c_Masked +
                                                      ECellFlag::c_Taken +
                                                      ECellFlag::c_Background);


      /// Flage that are reset at the start of each run of the cellular automaton
      static const ECellFlags c_TemporaryFlags = ECellFlags(ECellFlag::c_Assigned +
                                                            ECellFlag::c_Start +
                                                            ECellFlag::c_Cycle);

    public:

      /// Default constructor for ROOT compatibility. Cell weight defaults to 1
      AutomatonCell() :
        m_weight(0),
        m_flags(ECellFlag(0)),
        m_state(0) {}

      /// Constructor with a certain cell weight
      explicit AutomatonCell(const CellState& weight) :
        m_weight(weight),
        m_flags(ECellFlag(0)),
        m_state(0) {}

      /// Constructor with a certain cell weight and initial flags to be set.
      AutomatonCell(const CellState& weight, const ECellFlags& initialFlags) :
        m_weight(weight),
        m_flags(initialFlags),
        m_state(0) {}

      /// Getter for the cell state
      const CellState& getCellState() const { return m_state; }

      /// Setter for the cell state
      void setCellState(CellState state) const { m_state = state; }

      /// Getter for the cell weight. See details
      /** The cell might carry more than one unit of information to be added to the path. \n
       *  The weight discribes an additiv constant to be gained when picking up this cell. \n
       *  For instance segments may provide their number of hits as weight. \n
       *  Or they could have a reduced number of hits deminshed by the quality of a fit to the segment. \n
       *  For on the other side wire hits this should be one. */
      const CellState& getCellWeight() const
      { return m_weight; }

      /// Setter for the cell weight
      void setCellWeight(CellState weight) const
      { m_weight = weight; }



      /// Sets the already assigned marker flag to the given value. Default value true.
      void setAssignedFlag(bool setTo = true) const
      { setFlags<ECellFlag::c_Assigned>(setTo); }

      /// Resets the already assigned marker flag to false.
      void unsetAssignedFlag() const
      { setFlags<ECellFlag::c_Assigned>(false); }

      /// Gets the current state of the already assigned marker flag.
      bool hasAssignedFlag() const
      { return hasAnyFlags(ECellFlag::c_Assigned); }



      /// Sets the start marker flag to the given value. Default value true.
      void setStartFlag(bool setTo = true) const
      { setFlags<ECellFlag::c_Start>(setTo); }

      /// Resets the start marker flag to false.
      void unsetStartFlag() const
      { setFlags<ECellFlag::c_Start>(false); }

      /// Gets the current state of the start marker flag.
      bool hasStartFlag() const
      { return hasAnyFlags(ECellFlag::c_Start); }



      /// Sets the cycle marker flag to the given value. Default value true.
      void setCycleFlag(bool setTo = true) const
      { setFlags<ECellFlag::c_Cycle>(setTo); }

      /// Resets the cycle marker flag to false.
      void unsetCycleFlag() const
      { setFlags<ECellFlag::c_Cycle>(false); }

      /// Gets the current state of the cycle marker flag.
      bool hasCycleFlag() const
      { return hasAnyFlags(ECellFlag::c_Cycle); }


      /// Resets the assigned, start and cycle marker flag.
      void unsetTemporaryFlags() const
      { setFlags <c_TemporaryFlags> (false); }



      /// Sets the masked flag to the given value. Default value true.
      void setMaskedFlag(bool setTo = true) const
      { setFlags<ECellFlag::c_Masked>(setTo); }

      /// Resets the masked flag to false.
      void unsetMaskedFlag() const
      { setFlags<ECellFlag::c_Masked>(false); }

      /// Gets the current state of the masked marker flag.
      bool hasMaskedFlag() const
      { return hasAnyFlags(ECellFlag::c_Masked); }



      /// Sets the taken flag to the given value. Default value true.
      void setTakenFlag(bool setTo = true) const
      { setFlags<ECellFlag::c_Taken>(setTo); }

      /// Resets the taken flag to false.
      void unsetTakenFlag() const
      { setFlags<ECellFlag::c_Taken>(false); }

      /// Gets the current state of the taken marker flag.
      bool hasTakenFlag() const
      { return hasAnyFlags(ECellFlag::c_Taken); }



      /// Sets the background flag to the given value. Default value true.
      void setBackgroundFlag(bool setTo = true) const
      { setFlags<ECellFlag::c_Background>(setTo); }

      /// Resets the background flag to false.
      void unsetBackgroundFlag() const
      { setFlags<ECellFlag::c_Background>(false); }

      /// Gets the current state of the do not use flag marker flag.
      bool hasBackgroundFlag() const
      { return hasAnyFlags(ECellFlag::c_Background); }



      /// Setting accessing the flag by tag.
      template<ECellFlags cellFlag>
      void setFlags(bool setTo) const
      {
        if (setTo) setFlags(cellFlag);
        else clearFlags(cellFlag);
      }

      /** Getter for the ored combination of the cell flags to mark some status of the cell.
       *  Give the ored combination of all cell flags, therefore providing a bit pattern. \n
       *  Cell flags can be \n
       *  const ECellFlag::c_Assigned = 1; \n
       *  const ECellFlag::c_Start = 2; \n
       *  const ECellFlag::c_Cycle = 4; \n
       *  const ECellFlag::c_Masked = 8; \n
       *  const ECellFlag::c_Taken = 16; \n
       *  const ECellFlag::c_Background = 32; \n
       *  Use rather hasAnyFlags() to retrieve stats even for single state values.
       */
      const ECellFlags& getFlags() const
      { return m_flags; }

      /// Setter for the cell flags
      void setFlags(ECellFlags flags) const
      { m_flags = ECellFlags(m_flags bitor flags); }

      /// Clear all flags
      void clearFlags(ECellFlags flags = c_AllFlags) const
      { m_flags = ECellFlags(m_flags bitand ~flags); }

      /// Checks if a cell has any of a sum of given flags.
      bool hasAnyFlags(ECellFlags flags) const
      { return  m_flags bitand flags; }

    private:
      /// Storage for the cell weight
      mutable CellWeight m_weight = 1;

      /// Storage for the cell status flags
      mutable ECellFlags m_flags = ECellFlag(0);

      /// Storage for the cell state set by the cellular automata
      mutable CellState m_state = 0;

    }; // class

  } // namespace TrackFindingCDC
} // namespace Belle2
