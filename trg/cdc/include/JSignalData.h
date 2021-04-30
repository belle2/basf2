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

/// A class to hold common data for JSignals.
  class TRGCDCJSignalData {
    friend class TRGCDCJSignal;
    friend class TRGCDCJLUT;

  public:

    /// Constructor for class.
    TRGCDCJSignalData();
    /// Sets the filename for VHDL output.
    void setVhdlOutputFile(const std::string&);
    /// Sets if to print VHDL output.
    void setPrintVhdl(bool);
    /// Set to remember that file was printed.
    void setPrintedToFile(bool);
    /// Set the VHDL code that are in a process statement.
    void setVhdlInProcess(const std::string&);
    /// Set the VHDL code that is outside a process statement.
    void setVhdlOutProcess(const std::string&);
    /// Get the VHDL output code.
    std::string getVhdlOutputFile() const;
    /// Gets the status of m_printVhdl.
    bool getPrintVhdl() const;
    /// Gets the status of m_printedToFile.
    bool getPrintedToFile() const;
    /// Gets the signals that were saved for one line of VHDL.
    std::map<std::string, std::vector<int> > const& getSignals() const;
    /// Gets the VHDL code that are in a process statement.
    std::string getVhdlInProcess() const;
    /// Gets the VHDL code that are outside a process statement.
    std::string getVhdlOutProcess() const;
    /// Gets the VHDL code for define statement.
    std::string getVhdlDefine() const;
    /// Utilities
    /// Function to print VHDL code.
    void printToFile();
    /// Function to print buffer VHDL code.
    void buffersVhdlCode();
    /// Function to print definition of signal VHDL code.
    void signalsVhdlCode();
    /// Function to print entry VHDL code.
    void entryVhdlCode();

  private:

    //// Memebers.
    /// Holds the VHDL output filename.
    std::string m_vhdlOutputFile;
    /// Holds VHDL entry code.
    std::string m_vhdlEntry;
    /// Holds VHDL define code.
    std::string m_vhdlDefine;
    /// Holds VHDL process code.
    std::string m_vhdlInProcess;
    /// Holds VHDL out of process code.
    std::string m_vhdlOutProcess;
    /// Status if code should be printed.
    bool m_printVhdl;
    /// Statis if VHDL is printed to file.
    bool m_printedToFile;
    /// vector<int> is {type, bitwidth, buffer}
    /// Holds all the requried VHDL buffers.
    std::map<std::string, std::vector<int> > m_buffers;
    /// Holds all the requried VHDL signals.
    std::map<std::string, std::vector<int> > m_signals;
    /// Holds all the required VHDL types.
    std::map<std::string, bool > m_arrayType;

  };

} /// namespace Belle2

#endif /* TRGCDCJSignalData_FLAG_ */
