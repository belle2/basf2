#ifndef ECLSampleShaper
#define ECLSampleShaper

#include <TH1.h>

class ECLSampledShaper {
public:
  ECLSampledShaper(const TH1F* s)
    : m_sampledShape(s) {}
  ~ECLSampledShaper() { delete m_sampledShape;}
  void fillarray(int N, double* a)
  { for (int i = 0; i < N; i++) a[i] = m_sampledShape->GetBinContent(i + 1); }
private:
  const TH1F* m_sampledShape;
};

#endif
