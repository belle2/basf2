#pragma once

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCWireHit;
    class CDCSegment2D;

    /**
     * This class serves as a wrapper around all things that should go into a QuadTree.
     * For usage in the QuadTree we need to provide a used flag.
     * If your typeData-class does provide a used flag itself, just spezialize this template as done for the TrackHit below.
     */
    template<class typeData>
    class QuadTreeItem {
    public:

      /// Import contained data type
      using TypeData = typeData;

      /// Constructor
      explicit QuadTreeItem(typeData* data) : m_usedFlag(false), m_pointer(data) {};

      /**
       * Do not copy!
       */
      QuadTreeItem(const QuadTreeItem& copy) = delete;

      /**
       * Do not copy!
       */
      QuadTreeItem& operator=(QuadTreeItem const& copy) = delete;

      /**
       * Destructor
       */
      ~QuadTreeItem() = default;

      /**
       * Flag is set if the item was used as a result of the quad tree search and
       * should not be used in the next quad tree search round.
       */
      bool isUsed() const
      {
        return m_usedFlag;
      }

      /**
       * Set the used flag if you do not want that item to go into the next search round again.
       */
      void setUsedFlag(bool usedFlag = true)
      {
        m_usedFlag = usedFlag;
      }

      /**
       * Unset the used flag again if you have noticed that a previous setting was wrong.
       */
      void unsetUsedFlag()
      {
        setUsedFlag(false);
      }

      /**
       * Returns a pointer to the underlying item. This function is not called in the quad tree item itself but only in the filling and postprocessing stage.
       */
      typeData* getPointer()
      {
        return m_pointer;
      }

      /**
       * Const version of getPointer() above.
       */
      const typeData* getPointer() const
      {
        return m_pointer;
      }

    private:
      bool m_usedFlag;        /**< This flag can be set to not use the item in the next quad tree search round */
      typeData* m_pointer;    /**< A pointer to the underlying event */
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
