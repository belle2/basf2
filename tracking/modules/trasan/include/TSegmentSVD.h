#ifndef TSEGMENTSVD_FLAG
#define TSEGMENTSVD_FLAG

#include "tracking/modules/trasan/TSvdHit.h"
#if defined(BELLE_NAMESPACE)
namespace Belle {
#endif

  class TSegmentSVD {

  public:
    TSegmentSVD(void) : m_segLayerId(0), m_segId(0),
      m_movedSegId(0) {};
    TSegmentSVD(TSvdHit& l) : m_segLayerId(0), m_segId(0),
      m_movedSegId(0) { m_list.append(l); }
    TSegmentSVD(HepAList<TSvdHit> &l) : m_segLayerId(0), m_segId(0),
      m_movedSegId(0) { m_list.append(l); }

    ~TSegmentSVD(void) {};

    inline void append(const HepAList<TSvdHit> &l) { m_list.append(l); }
    inline void append(TSvdHit& l) { m_list.append(l); }

    inline void remove(const HepAList<TSvdHit> &l) { m_list.remove(l); }
    inline void remove(TSvdHit& l) { m_list.remove(l); }

    inline const HepAList<TSvdHit> & list(void) const { return m_list; }

    inline void segLayerId(unsigned id) { m_segLayerId = id; }
    inline void segId(unsigned id) { m_segId = id; }
    inline void movedSegId(int id) { m_movedSegId = id; }

    inline unsigned segLayerId(void) const { return m_segLayerId; }
    inline unsigned segId(void) const { return m_segId; }
    inline int      movedSegId(void) const { return m_movedSegId; }

  private:
    unsigned m_segLayerId;
    unsigned m_segId;
    int      m_movedSegId;

    HepAList<TSvdHit> m_list;
  };
#if defined(BELLE_NAMESPACE)
} // namespace Belle
#endif
#endif /* TSEGMENTSVD_FLAG */
