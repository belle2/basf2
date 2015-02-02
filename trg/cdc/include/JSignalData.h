//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : JSignalData.h
// Section  : TRG CDC
// Owner    : Jaebak KIM (K.U.)
// Email    : jbkim@hep.korea.ac.kr
//-----------------------------------------------------------------------------
// Description : A class for SignalData in CDC Trigger for 3D tracker.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCJSignalData_FLAG_
#define TRGCDCJSignalData_FLAG_

#include <string>
#include <vector>
#include <map>

#ifdef TRGCDC_SHORT_NAMES
#define JSignalData TRGCDCJSignalData
#endif

namespace Belle2 {

class TRGCDCJSignal;

class TRGCDCJSignalData {
  friend class TRGCDCJSignal;
  friend class TRGCDCJLUT;

  public:

  TRGCDCJSignalData();
  void setVhdlOutputFile(std::string);
  void setPrintVhdl(bool);
  void setPrintedToFile(bool);
  void setVhdlInProcess(std::string);
  void setVhdlOutProcess(std::string);
  std::string getVhdlOutputFile() const;
  bool getPrintVhdl() const;
  bool getPrintedToFile() const;
  std::string getVhdlInProcess() const;
  std::string getVhdlOutProcess() const;

  // Utilities
  void printToFile();
  void buffersVhdlCode();
  void signalsVhdlCode();
  void entryVhdlCode();

  private:

  /// Memebers.
  std::string m_vhdlOutputFile;
  std::string m_vhdlEntry;
  std::string m_vhdlDefine;
  std::string m_vhdlInProcess;
  std::string m_vhdlOutProcess;
  bool m_printVhdl;
  bool m_printedToFile;
  // vector<int> is {type, bitwidth, buffer}
  std::map<std::string, std::vector<int> > m_buffers;
  std::map<std::string, std::vector<int> > m_signals;
  std::map<std::string, bool > m_arrayType;

};

} // namespace Belle2

#endif /* TRGCDCJSignalData_FLAG_ */
