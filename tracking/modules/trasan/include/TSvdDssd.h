#ifndef _DEFINE_TSVD_DSSD_H_
#define _DEFINE_TSVD_DSSD_H_


#include <cstddef>

#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/AList.h"
#if defined(BELLE_NAMESPACE)
namespace Belle {
#endif

  class Recsvd_cluster;

  class TSvdDssd {
  public:
    // Constructor
    TSvdDssd(void) : m_dssd(-1) {};
    TSvdDssd(int dssd) : m_dssd(dssd) {};

    // Destructor
    virtual ~TSvdDssd(void) {};

    // Modifiers
    void dssd(const int& d) { m_dssd = d; }
    void rphiAppend(Recsvd_cluster& c) { m_rphi.append(c); }
    void zAppend(Recsvd_cluster& c) { m_z.append(c); }
    void rphiRemove(Recsvd_cluster& c) { m_rphi.remove(c); }
    void zRemove(Recsvd_cluster& c) { m_z.remove(c); }

    // Selectors
    inline int dssd(void) const { return m_dssd; }

    Recsvd_cluster* rphi(unsigned index) const {
      if (index < (unsigned) m_rphi.length())return m_rphi[index];
      else return NULL;
    }

    Recsvd_cluster* z(unsigned index) const {
      if (index < (unsigned) m_z.length())return m_z[index];
      else return NULL;
    }

    unsigned nRphi(void) const { return m_rphi.length(); }
    unsigned nZ(void) const { return m_z.length(); }

    const AList<Recsvd_cluster> & rphiList(void) { return m_rphi; }
    const AList<Recsvd_cluster> & zList(void) { return m_z; }

    void clear(void) {
      m_rphi.removeAll();
      m_z.removeAll();
    }
  private:
    int m_dssd;
    AList<Recsvd_cluster> m_rphi;
    AList<Recsvd_cluster> m_z;
  };

#if defined(BELLE_NAMESPACE)
} // namespace Belle
#endif
#endif /* _DEFINE_TSVD_DSSD_H_ */
