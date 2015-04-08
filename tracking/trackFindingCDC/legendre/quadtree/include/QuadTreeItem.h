#pragma once

#include <framework/logging/LogMethod.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    class TrackHit;

    template<class typeData>
    class QuadTreeItem {
    public:
      QuadTreeItem(typeData* data) : m_usedFlag(false), m_pointer(data) { };
      QuadTreeItem(const QuadTreeItem& copy) = delete;
      QuadTreeItem& operator=(QuadTreeItem const& copy) = delete;

      virtual ~QuadTreeItem() { }

      virtual bool isUsed() const
      {
        return m_usedFlag;
      }

      virtual void setUsedFlag(bool usedFlag = true)
      {
        m_usedFlag = usedFlag;
      }

      virtual void unsetUsedFlag() final {
        setUsedFlag(false);
      }

      virtual typeData* getPointer() final {
        return m_pointer;
      }

      virtual const typeData* const getPointer() const final
      {
        return m_pointer;
      }

    private:
      bool m_usedFlag;
      typeData* m_pointer;
    };

    // As special case:
    template<>
    bool QuadTreeItem<TrackHit>::isUsed() const;

    template<>
    void QuadTreeItem<TrackHit>::setUsedFlag(bool usedFlag);

    typedef QuadTreeItem<TrackHit> LegendreQuadTreeItem;

  }

}
