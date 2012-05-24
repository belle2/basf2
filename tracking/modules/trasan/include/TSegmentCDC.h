#ifndef TSEGMENTRGCDC_FLAG
#define TSEGMENTRGCDC_FLAG

#include "tracking/modules/trasan/TLink.h"
#if defined(BELLE_NAMESPACE)
namespace Belle {
#endif

  class TSegmentCDC {

  public:
    TSegmentCDC(void) : m_segLayerId(0), m_segId(0),
      m_movedSegId(0),
      m_superLayerId(0) {};
    TSegmentCDC(TLink& l) : m_segLayerId(0), m_segId(0),
      m_movedSegId(0),
      m_superLayerId(0) { m_list.append(l); }
    TSegmentCDC(HepAList<TLink> &l) : m_segLayerId(0), m_segId(0),
      m_movedSegId(0),
      m_superLayerId(0) { m_list.append(l); }

    ~TSegmentCDC(void) {};

    inline void append(const HepAList<TLink> &l) { m_list.append(l); }
    inline void append(TLink& l) { m_list.append(l); }

    inline void remove(const HepAList<TLink> &l) { m_list.remove(l); }
    inline void remove(TLink& l) { m_list.remove(l); }

    inline const HepAList<TLink> & list(void) const { return m_list; }

    inline void segLayerId(unsigned id) { m_segLayerId = id; }
    inline void segId(unsigned id) { m_segId = id; }
    inline void movedSegId(int id) { m_movedSegId = id; }
    inline void superLayerId(unsigned id) { m_superLayerId = id; }

    inline unsigned segLayerId(void) const { return m_segLayerId; }
    inline unsigned segId(void) const { return m_segId; }
    inline int      movedSegId(void) const { return m_movedSegId; }
    inline unsigned superLayerId(void) const { return m_superLayerId; }

  private:
    unsigned m_segLayerId;
    unsigned m_segId;
    int      m_movedSegId;
    unsigned m_superLayerId;

    HepAList<TLink> m_list;
  };
#if defined(BELLE_NAMESPACE)
} // namespace Belle
#endif
#endif /* TSEGMENTRGCDC_FLAG */
