/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ECLSampleShaper
#define ECLSampleShaper

#include <TH1.h>

namespace Belle2 {
  namespace ECL {

    /** digitisation shaper */
    class ECLSampledShaper {
    public:
      /** constructor */
      explicit ECLSampledShaper(const TH1F* s, double ts = 1)
        : m_sampledShape(s), m_timeScale(ts) {}
      /** destructor */
      ~ECLSampledShaper() { delete m_sampledShape;}
      /** fill the sampled shape array */
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
      const TH1F* m_sampledShape; /**< the sampled shape */
      const int m_timeScale; /**< the time scale */
    };

  }
}
#endif
