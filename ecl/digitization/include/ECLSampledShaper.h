#ifndef ECLSampleShaper
#define ECLSampleShaper

#include <TH1.h>

namespace Belle2 {
  namespace ECL {
    class ECLSampledShaper {
    public:
      ECLSampledShaper(const TH1F* s, double ts = 1)
        : m_sampledShape(s), m_timeScale(ts) {}
      ~ECLSampledShaper() { delete m_sampledShape;}
      void fillarray(int N, double* a)
      {
        if (m_timeScale == 1)
          for (int i = 0; i < N; i++) a[i] = m_sampledShape->GetBinContent(i * m_timeScale + 1);
        else {
          for (int i = 0; i < N; i++) {
            a[i] = 0;
            for (int j = 0; j < m_timeScale; j++)
              a[i] += m_sampledShape->GetBinContent(i * m_timeScale + j);
            a[i] /= m_timeScale;
          }
        }
      }
    private:
      const TH1F* m_sampledShape;
      const int m_timeScale;
    };

  }
}
#endif
