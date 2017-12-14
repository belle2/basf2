/*******************\
  SA0xParamBase.h

  update: 11/02/28

\*******************/

#ifndef SA0xParamBase_H
#define SA0xParamBase_H

class SA0xParamBase {
protected:
  unsigned int m_param;
  unsigned int m_rbparam;
public:
  SA0xParamBase() {}
  virtual ~SA0xParamBase() {}
  unsigned int param() { return m_param; }
  unsigned int rbparam() { return m_rbparam; }
  void param(unsigned int x) { m_param = x; }
  void rbparam(unsigned int x) { m_rbparam = x; }
  void dump(const char* name = "param");
  int compare(bool verbose = true, const char* name = "param");
  virtual unsigned int getMaskVariable() = 0;
protected:
  void setvalue(int value, unsigned int mask, unsigned int shift);
  int getrbvalue(unsigned int mask, unsigned int shift);
};

#endif
