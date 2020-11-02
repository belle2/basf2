/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2018 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo                                       *
 *                Alon Hershenhorn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
