/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
// Intentionally void
//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : JSignalData.cc
// Section  : TRG CDC
// Owner    : Jaebak KIM (K.U.)
// Email    : jbkim@hep.korea.ac.kr
//-----------------------------------------------------------------------------
// Description : A class for SignalData in CDC Trigger for 3D tracker.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#ifndef __EXTERNAL__
#include "trg/cdc/JSignalData.h"
#include "trg/cdc/JSignal.h"
#else
#include "JSignal.h"
#include "JSignalData.h"
#endif
#include <iostream>
#include <fstream>

using namespace std;

namespace Belle2 {

  TRGCDCJSignalData::TRGCDCJSignalData()
    : m_vhdlOutputFile("vhdlOutput"),
      m_vhdlEntry(""),
      m_vhdlDefine(""),
      m_vhdlInProcess(""),
      m_vhdlOutProcess("")
  {
    m_printVhdl = 0;
    m_printedToFile = 0;
  }

  void TRGCDCJSignalData::setVhdlOutputFile(const string& vhdlOutputFile)
  {
    m_vhdlOutputFile = vhdlOutputFile;
  }

  void TRGCDCJSignalData::setPrintVhdl(bool printVhdl)
  {
    m_printVhdl = printVhdl;
  }

  void TRGCDCJSignalData::setPrintedToFile(bool printedToFile)
  {
    m_printedToFile = printedToFile;
  }

  void TRGCDCJSignalData::setVhdlInProcess(const std::string& vhdlInProcess)
  {
    m_vhdlInProcess = vhdlInProcess;
  }

  void TRGCDCJSignalData::setVhdlOutProcess(const std::string& vhdlOutProcess)
  {
    m_vhdlOutProcess = vhdlOutProcess;
  }

  std::string TRGCDCJSignalData::getVhdlOutputFile() const
  {
    return m_vhdlOutputFile;
  }

  bool TRGCDCJSignalData::getPrintVhdl() const
  {
    return m_printVhdl;
  }

  bool TRGCDCJSignalData::getPrintedToFile() const
  {
    return m_printedToFile;
  }

  std::string TRGCDCJSignalData::getVhdlInProcess() const
  {
    return m_vhdlInProcess;
  }

  std::string TRGCDCJSignalData::getVhdlOutProcess() const
  {
    return m_vhdlOutProcess;
  }

  std::string TRGCDCJSignalData::getVhdlDefine() const
  {
    return m_vhdlDefine;
  }

  std::map<std::string, std::vector<int> > const& TRGCDCJSignalData::getSignals() const
  {
    return m_signals;
  }


  void TRGCDCJSignalData::printToFile()
  {
    // Write to file.
    ofstream outFile;
    outFile.open(m_vhdlOutputFile);
    if (outFile.is_open()) {
      outFile << "library IEEE;" << endl;
      outFile << "use ieee.std_logic_1164.all;" << endl;
      outFile << "use ieee.numeric_std.all;" << endl;
      outFile << endl;
      outFile << "entity Firmware is" << endl;
      outFile << "  PORT ( CLKIN   : in STD_LOGIC;" << endl;
      outFile << "         INPUT   : in std_logic_vector(0 downto 0);" << endl;
      outFile << "         OUTPUT   : in std_logic_vector(0 downto 0)" << endl;
      outFile << ");" << endl;
      outFile << endl;
      outFile << m_vhdlEntry << endl;
      outFile << endl;
      outFile << "end Firmware;" << endl;
      outFile << endl;
      outFile << "architecture Behavioral of Firmware is" << endl;
      outFile << endl;
      outFile << m_vhdlDefine << endl;
      outFile << endl;
      outFile << "begin" << endl;
      outFile << endl;
      outFile << "-- Main algorithm" << endl;
      outFile << "logic: process (CLKIN) is" << endl;
      outFile << "begin" << endl;
      outFile << "  if CLKIN'event and CLKIN='1' then" << endl;
      outFile << endl;
      outFile << m_vhdlInProcess << endl;
      outFile << endl;
      outFile << "  end if;" << endl;
      outFile << "end process;" << endl;
      outFile << endl;
      outFile << m_vhdlOutProcess << endl;
      outFile << endl;
      outFile << "end Behavioral;" << endl;
      outFile.close();
      m_printedToFile = 1;
    }
  }

  void TRGCDCJSignalData::buffersVhdlCode()
  {
    // Define.
    for (map<string, vector<int> >::const_iterator it = m_buffers.begin(); it != m_buffers.end(); ++it) {
      string const& name = it->first;
      int const& type = it->second[0];
      int const& bitwidth = it->second[1];
      int const& buffer = it->second[2];
      string arrayName = (type == 1 ? "U" : "S") + to_string(bitwidth) + "D" + to_string(buffer + 1) + "Array";
      if (m_arrayType.find(arrayName) == m_arrayType.end()) {
        m_vhdlDefine += "type " + arrayName + " is array(" + to_string(buffer) + " downto 0) of " + (type == 1 ? "unsigned" : "signed") +
                        "(" + to_string(bitwidth - 1) + " downto 0);\n";
        m_arrayType[arrayName] = 1;
      }
      m_vhdlDefine += "signal " + name + "_b : " + arrayName + " := (others=>(others=>'0'));\n";
    }
    // Process.
    for (map<string, vector<int> >::const_iterator it = m_buffers.begin(); it != m_buffers.end(); ++it) {
      string const& name = it->first;
      int const& buffer = it->second[2];
      m_vhdlInProcess += name + "_b(" + to_string(0) + ") <= " + name + ";\n";
      for (int iBuffer = 0; iBuffer < buffer; iBuffer++) {
        m_vhdlInProcess += name + "_b(" + to_string(iBuffer + 1) + ") <= " + name + "_b(" + to_string(iBuffer) + ");\n";
      }
    }

  }

  void TRGCDCJSignalData::signalsVhdlCode()
  {
    for (map<string, vector<int> >::const_iterator it = m_signals.begin(); it != m_signals.end(); ++it) {
      string const& name = it->first;
      int const& type = it->second[0];
      int const& bitwidth = it->second[1];
      string typeName;
      if (type == 1) typeName = "unsigned";
      else if (type == -1) typeName = "signed";
      else if (type == 2) typeName = "std_logic_vector";
      else {
        cout << "[Error] TRGCDCJSignalData::signalsVhdlCode() => signal type is unknown." << endl;
      }
      m_vhdlDefine += "signal " + name + " : " + typeName + "(" + to_string(bitwidth - 1) + " downto 0) := (others=>'0');\n";
    }
  }

  void TRGCDCJSignalData::entryVhdlCode()
  {
    m_vhdlEntry += "function decimal_string_to_unsigned(decimal_string: string; wanted_bitwidth: integer) return unsigned is\n";
    m_vhdlEntry += "  variable tmp_unsigned: unsigned(wanted_bitwidth-1 downto 0) := (others => '0');\n";
    m_vhdlEntry += "  variable character_value: integer;\n";
    m_vhdlEntry += "begin\n";
    m_vhdlEntry += "  for string_pos in decimal_string'range loop\n";
    m_vhdlEntry += "    case decimal_string(string_pos) is\n";
    m_vhdlEntry += "      when '0' => character_value := 0;\n";
    m_vhdlEntry += "      when '1' => character_value := 1;\n";
    m_vhdlEntry += "      when '2' => character_value := 2;\n";
    m_vhdlEntry += "      when '3' => character_value := 3;\n";
    m_vhdlEntry += "      when '4' => character_value := 4;\n";
    m_vhdlEntry += "      when '5' => character_value := 5;\n";
    m_vhdlEntry += "      when '6' => character_value := 6;\n";
    m_vhdlEntry += "      when '7' => character_value := 7;\n";
    m_vhdlEntry += "      when '8' => character_value := 8;\n";
    m_vhdlEntry += "      when '9' => character_value := 9;\n";
    m_vhdlEntry += "      when others => report(\"Illegal number\") severity failure;\n";
    m_vhdlEntry += "    end case;\n";
    m_vhdlEntry += "    tmp_unsigned := resize(tmp_unsigned * 10, wanted_bitwidth);\n";
    m_vhdlEntry += "    tmp_unsigned := tmp_unsigned + character_value;\n";
    m_vhdlEntry += "  end loop;\n";
    m_vhdlEntry += "  return tmp_unsigned;\n";
    m_vhdlEntry += "end decimal_string_to_unsigned;\n";
    m_vhdlEntry += "function decimal_string_to_signed(decimal_string: string; wanted_bitwidth: positive) return signed is\n";
    m_vhdlEntry += "  variable tmp_signed: signed(wanted_bitwidth-1 downto 0) := (others => '0');\n";
    m_vhdlEntry += "  variable character_value: integer := 0;\n";
    m_vhdlEntry += "  variable sign_value: integer := 1;\n";
    m_vhdlEntry += "begin\n";
    m_vhdlEntry += "  for string_pos in decimal_string'range loop\n";
    m_vhdlEntry += "    case decimal_string(string_pos) is\n";
    m_vhdlEntry += "      when '-' => sign_value := -1;\n";
    m_vhdlEntry += "      when '0' => character_value := 0;\n";
    m_vhdlEntry += "      when '1' => character_value := 1;\n";
    m_vhdlEntry += "      when '2' => character_value := 2;\n";
    m_vhdlEntry += "      when '3' => character_value := 3;\n";
    m_vhdlEntry += "      when '4' => character_value := 4;\n";
    m_vhdlEntry += "      when '5' => character_value := 5;\n";
    m_vhdlEntry += "      when '6' => character_value := 6;\n";
    m_vhdlEntry += "      when '7' => character_value := 7;\n";
    m_vhdlEntry += "      when '8' => character_value := 8;\n";
    m_vhdlEntry += "      when '9' => character_value := 9;\n";
    m_vhdlEntry += "      when others => report(\"Illegal number\") severity failure;\n";
    m_vhdlEntry += "    end case;\n";
    m_vhdlEntry += "    tmp_signed := resize(tmp_signed * 10, wanted_bitwidth);\n";
    m_vhdlEntry += "    tmp_signed := tmp_signed + sign_value * character_value;\n";
    m_vhdlEntry += "  end loop;\n";
    m_vhdlEntry += "  return tmp_signed;\n";
    m_vhdlEntry += "end decimal_string_to_signed;\n";
  }

}
