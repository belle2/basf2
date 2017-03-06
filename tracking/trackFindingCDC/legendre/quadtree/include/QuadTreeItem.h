/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCWireHit;
    class CDCSegment2D;

    /**
     * This class serves as a wrapper around all things that should go into a QuadTree.
     * For usage in the QuadTree we need to provide a used flag.
     * If your AData-class does provide a used flag itself,
     * just spezialize this template as done for the CDCWireHit and CDCSegment2D below.
     */
    template<class AData>
    class QuadTreeItem {
    public:
      /// Constructor
      explicit QuadTreeItem(AData* data)
        : m_data(data)
        , m_usedFlag(false) {};

    private:
      /// Do not copy!
      QuadTreeItem(const QuadTreeItem& copy) = delete;

      /// Do not copy!
      QuadTreeItem& operator=(QuadTreeItem const& copy) = delete;

    public:
      /// Returns the underlying data.
      AData* getPointer() const
      {
        return m_data;
      }

      /**
       * Flag is set if the item was used as a result of the quad tree search and
       * should not be used in the next quad tree search round.
       */
      bool isUsed() const
      {
        return m_usedFlag;
      }

      /// Set the used flag if you do not want that item to go into the next search round again.
      void setUsedFlag(bool usedFlag = true)
      {
        m_usedFlag = usedFlag;
      }

      /// Unset the used flag again if you have noticed that a previous setting was wrong.
      void unsetUsedFlag()
      {
        setUsedFlag(false);
      }

    private:
      /// A pointer to the underlying item data
      AData* m_data;

      /// This flag can be set to not use the item in the next quad tree search round
      bool m_usedFlag;
    };

    /**
     *  As special case: the QuadTreeHitWrapper has a used flag itself
     */
    template<>
    bool QuadTreeItem<const CDCWireHit>::isUsed() const;

    /**
     *  As special case: the QuadTreeHitWrapper has a used flag itself
     */
    template<>
    void QuadTreeItem<const CDCWireHit>::setUsedFlag(bool usedFlag);

    /**
     *  As special case: the CDCSegment2D has a used flag itself
     */
    template<>
    bool QuadTreeItem<CDCSegment2D>::isUsed() const;

    /**
     *  As special case: the CDCSegment2D has a used flag itself
     */
    template<>
    void QuadTreeItem<CDCSegment2D>::setUsedFlag(bool usedFlag);
  }
}
