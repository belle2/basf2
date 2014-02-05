/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef AUTOMATONCELL_H
#define AUTOMATONCELL_H

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Cell used by the cellular automata
    /** This class represents a cell in the cellular automata algorithm the local tracking is build on.
     *  Different to the ordinary cellular automaton algorihms this class stores the state not as a interger number but \n
     *  as a float value. This enables to have fractional measures of quality attached to the cell if say one new item \n
     *  is not worth a full point, but a probability measure which is additiv. The points that can be gained by picking \n
     *  this cell are stored in the cell weight property. Moreover the class defines status flags to be set and/or read by\n
     *  the cellular automaton */
    class AutomatonCell : public CDCLocalTracking::UsedTObject {

    public:

      /// Default constructor for ROOT compatibility. Cell weight defaults to 1
      AutomatonCell() :
        m_weight(0),
        m_flags(0),
        m_state(0) {;}

      /// Constructor with a certain cell weight
      AutomatonCell(const CellState& weight) :
        m_weight(weight),
        m_flags(0),
        m_state(0) {;}

      /// Constructor with a certain cell weight and initial flags to be set.
      AutomatonCell(const CellState& weight, const CellFlags& initialFlags) :
        m_weight(weight),
        m_flags(initialFlags),
        m_state(0) {;}

      /// Empty deconstructor
      ~AutomatonCell() {;}

      /// Getter for the cell state
      const CellState& getCellState() const { return m_state; }

      /// Setter for the cell state
      void setCellState(CellState state) const { m_state = state; }

      /** getter and setter for the cell flags */
      /// Getter for the additional cell flags to mark some status of the cell
      /** Give the sum of all cell flags, therefor providing a bit pattern. \n
       *  Cell flags can be \n
       *  const CellFlags IS_SET = 1; \n
       *  const CellFlags IS_START = 2; \n
       *  const CellFlags IS_CYCLE = 4; \n
       *  const CellFlags DO_NOT_USE = 8; \n
       *  Use rather hasAnyFlags() to retrieve stats even for single state values.
       */
      CellFlags getFlags() const { return m_flags; }

      /// Setter for the cell flags
      void setFlags(CellFlags flags) const { m_flags |= flags; }

      /// Clear all flags
      void clearFlags(CellFlags flags = ALL_FLAGS) const { m_flags &= ~flags; }

      /// Checks if a cell has any of a sum of given flags.
      bool hasAnyFlags(CellFlags flags) const { return  m_flags bitand flags; }


      /// Getter for the cell weight. See details
      /** The cell might carry more than one unit of information to be added to the path. \n
       *  The weight discribes an additiv constant to be gained when picking up this cell. \n
       *  For instance segments may provide their number of hits as weight. \n
       *  Or they could have a reduced number of hits deminshed by the quality of a fit to the segment. \n
       *  For on the other side wire hits this should be one. */
      const CellState& getCellWeight() const { return m_weight; }

      /// Setter for the cell weight
      void setCellWeight(CellState weight) const { m_weight = weight; }

      /// Transitional change to migrate AutomatonCell from a base class to a regular member
      AutomatonCell& getAutomatonCell() { return *this; }

      /// Transitional change to migrate AutomatonCell from a base class to a regular member
      const AutomatonCell& getAutomatonCell() const { return *this; }

    private:

      mutable CellState m_weight; ///< Storage for the cell weight
      mutable CellFlags m_flags; ///< Storage for the cell status flags
      mutable CellState m_state; ///< Storage for the cell state set by the cellular automata


      /** ROOT Macro to make AutomatonCell a ROOT class.*/
      ClassDefInCDCLocalTracking(AutomatonCell, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // AUTOMATONCELL
