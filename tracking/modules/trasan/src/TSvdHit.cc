
#if 0 // old!!!!!!!
#include "tracking/modules/trasan/TSvdHit.h"

namespace Belle {

  unsigned
  TSvdHit::check(const TSvdHit& h)
  {
    if (h.m_phiPattern & m_phiPattern ||
        h.m_zPattern & m_zPattern) {
      return 0;
    }
    return 1;
  }

} // namespace Belle
#endif

