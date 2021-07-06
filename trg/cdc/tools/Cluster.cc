/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifdef __clang__
// This file takes forever when compiled with clang (~25min just for this one
// file) so let's selectively disable optimization. As this is a simple enough
// tool this should not make any problems
#pragma clang optimize off
#endif

#include <fstream>
#include <iomanip>

using namespace std;


int main()
{

  ofstream outputp("UT3_0_Cluster_M.vhd");
  ofstream outputpp("UT3_0_Cluster_P.vhd");

  ///////library///////////////////////////////////////////////////////////////////////
  outputp << "library IEEE;" << endl;
  outputp << "use IEEE.STD_LOGIC_1164.ALL;" << endl;
  outputpp << "library IEEE;" << endl;
  outputpp << "use IEEE.STD_LOGIC_1164.ALL;" << endl;
  outputpp << " " << endl;
  outputp << " " << endl;

  ///////entity///////////////////////////////////////////////////////////////////////
  outputp << "entity UT3_0_Cluster_M is" << endl;
  outputpp << "entity UT3_0_Cluster_P is" << endl;
  outputp << "Port (";
  outputpp << "Port (";

  for (unsigned a = 1; a <= 16; a++) {
    outputp << "     Minus_row" << a << "   : out  STD_LOGIC_VECTOR (79 downto 40);" << endl;
    outputpp << "     Plus_row" << a << "   : out  STD_LOGIC_VECTOR (39 downto 0);" << endl;
  }

  outputp << "        SL0_TS		 : IN  std_logic_vector(80 downto 0);" << endl;
  outputp << "	SL2_TS		 : IN  std_logic_vector(96 downto 0);" << endl;
  outputp << "	SL4_TS		 : IN  std_logic_vector(128 downto 0);" << endl;
  outputp << "	SL6_TS		 : IN  std_logic_vector(160 downto 0);" << endl;
  outputp << "	SL8_TS		 : IN  std_logic_vector(192 downto 0);" << endl;
  outputp << "	Top_clkData_s: IN  std_logic" << endl;
  outputp << "	);" << endl;
  outputpp << "        SL0_TS		 : IN  std_logic_vector(80 downto 0);" << endl;
  outputpp << "	SL2_TS		 : IN  std_logic_vector(96 downto 0);" << endl;
  outputpp << "	SL4_TS		 : IN  std_logic_vector(128 downto 0);" << endl;
  outputpp << "	SL6_TS		 : IN  std_logic_vector(160 downto 0);" << endl;
  outputpp << "	SL8_TS		 : IN  std_logic_vector(192 downto 0);" << endl;
  outputpp << "	Top_clkData_s: IN  std_logic" << endl;
  outputpp << "	);" << endl;
  outputpp << " " << endl;
  outputp << " " << endl;
  outputp << "end  UT3_0_Cluster_M;" << endl;
  outputpp << "end  UT3_0_Cluster_P;" << endl;
  outputpp << " " << endl;
  outputp << " " << endl;
  ///////architecture///////////////////////////////////////////////////////////////////////
  outputp << "architecture Behavioral of UT3_0_Cluster_M is" << endl;
  outputpp << "architecture Behavioral of UT3_0_Cluster_P is" << endl;
  outputpp << " " << endl;
  outputp << " " << endl;
  ///////signal///////////////////////////////////////////////////////////////////////
  for (unsigned a = 1; a <= 16; a++) {
    outputp << "signal  row" << a << "   :   STD_LOGIC_VECTOR (79 downto 40);" << endl;
    outputpp << "signal  row" << a << "   :   STD_LOGIC_VECTOR (0 to 39);" << endl;
    outputpp << "signal  row" << a << "_s   :   STD_LOGIC_VECTOR (39 downto 0);" << endl;
  }
  outputpp << " " << endl;
  outputp << " " << endl;
  for (unsigned js = 1; js < 16; js += 2) {
    for (unsigned ts = 0; ts < 80; ts += 2) {
      if (ts < 40) {
        outputpp << "signal	patternI_" << ts % 160 + 1 << "_" << js << "_A  :   STD_LOGIC_VECTOR (3 downto 0);" << endl;
        outputpp << "signal	patternI_" << ts % 160 + 1 << "_" << js << "_B  :   STD_LOGIC_VECTOR (3 downto 0);" << endl;
        outputpp << "signal	patternI_" << ts % 160 + 1 << "_" << js << "_D  :   STD_LOGIC_VECTOR (3 downto 0);" << endl;
        outputpp << "signal	patternI_" << ts % 160 + 1 << "_" << js << "_E  :   STD_LOGIC_VECTOR (3 downto 0);" << endl;
        outputpp << "signal	patternI_" << ts % 160 + 1 << "_" << js << "_G  :   STD_LOGIC_VECTOR (3 downto 0);" << endl;
        outputpp << "signal	patternI_" << ts % 160 + 1 << "_" << js << "_H  :   STD_LOGIC_VECTOR (3 downto 0);" << endl;
      }
      if (ts >= 40 && ts < 80) {
        outputp << "signal	patternI_" << ts % 160 << "_" << js << "_A  :   STD_LOGIC_VECTOR (3 downto 0);" << endl;
        outputp << "signal	patternI_" << ts % 160 << "_" << js << "_B  :   STD_LOGIC_VECTOR (3 downto 0);" << endl;
        outputp << "signal	patternI_" << ts % 160 << "_" << js << "_D  :   STD_LOGIC_VECTOR (3 downto 0);" << endl;
        outputp << "signal	patternI_" << ts % 160 << "_" << js << "_E  :   STD_LOGIC_VECTOR (3 downto 0);" << endl;
        outputp << "signal	patternI_" << ts % 160 << "_" << js << "_G  :   STD_LOGIC_VECTOR (3 downto 0);" << endl;
        outputp << "signal	patternI_" << ts % 160 << "_" << js << "_H  :   STD_LOGIC_VECTOR (3 downto 0);" << endl;
      }
    }
  }
  outputpp << " " << endl;
  outputp << " " << endl;
  ///////component///////////////////////////////////////////////////////////////////////
  ///////HoughVoting///////////////////////////////////////////////////////////////////////
  outputp << "  COMPONENT UT3_0_HoughVoting_M" << endl;
  outputp << "  Port ( ";
  outputpp << " COMPONENT UT3_0_HoughVoting_P" << endl;
  outputpp << " Port ( ";
  for (unsigned a = 1; a <= 16; a++) {
    outputp << "      row" << a << "   : out  STD_LOGIC_VECTOR (79 downto 40);" << endl;
    outputpp << "     row" << a << "   : out  STD_LOGIC_VECTOR (39 downto 0);" << endl;
  }
  outputp << "        SL0_TS		 : IN  std_logic_vector(80 downto 0);" << endl;
  outputp << "	SL2_TS		 : IN  std_logic_vector(96 downto 0);" << endl;
  outputp << "	SL4_TS		 : IN  std_logic_vector(128 downto 0);" << endl;
  outputp << "	SL6_TS		 : IN  std_logic_vector(160 downto 0);" << endl;
  outputp << "	SL8_TS		 : IN  std_logic_vector(192 downto 0);" << endl;
  outputp << "	Top_clkData_s: IN  std_logic" << endl;
  outputp << "	);" << endl;
  outputpp << "        SL0_TS		 : IN  std_logic_vector(80 downto 0);" << endl;
  outputpp << "	SL2_TS		 : IN  std_logic_vector(96 downto 0);" << endl;
  outputpp << "	SL4_TS		 : IN  std_logic_vector(128 downto 0);" << endl;
  outputpp << "	SL6_TS		 : IN  std_logic_vector(160 downto 0);" << endl;
  outputpp << "	SL8_TS		 : IN  std_logic_vector(192 downto 0);" << endl;
  outputpp << "	Top_clkData_s: IN  std_logic" << endl;
  outputpp << "	);" << endl;
  outputp << "	end COMPONENT;" << endl;
  outputpp << "	end COMPONENT;" << endl;
  outputpp << " " << endl;
  outputp << " " << endl;

  ///////cluster_center///////////////////////////////////////////////////////////////////////
  outputp << "	COMPONENT cluster_center is" << endl;
  outputp << "	port(		new_cluster_A : out  std_logic_vector(3 downto 0);" << endl;
  outputp << "	new_cluster_B : out  std_logic_vector(3 downto 0);" << endl;
  outputp << "	new_cluster_D : out  std_logic_vector(3 downto 0);" << endl;
  outputp << "	new_cluster_E : out  std_logic_vector(3 downto 0);" << endl;
  outputp << "	new_cluster_G : out  std_logic_vector(3 downto 0);" << endl;
  outputp << "	new_cluster_H : out  std_logic_vector(3 downto 0);	" << endl;
  outputp << "	X : IN  std_logic_vector(3 downto 0);" << endl;
  outputp << "	Y : IN  std_logic_vector(3 downto 0);" << endl;
  outputp << "	Z : IN  std_logic_vector(3 downto 0);" << endl;
  outputp << "	A : IN  std_logic_vector(3 downto 0);" << endl;
  outputp << "	B : IN  std_logic_vector(3 downto 0);" << endl;
  outputp << "	D : IN  std_logic_vector(3 downto 0);" << endl;
  outputp << "	E : IN  std_logic_vector(3 downto 0);" << endl;
  outputp << "	G : IN  std_logic_vector(3 downto 0);" << endl;
  outputp << "	H : IN  std_logic_vector(3 downto 0);" << endl;
  outputp << "	top_clkdata_s:IN std_logic" << endl;
  outputp << "	);" << endl;
  outputp << "	end COMPONENT;" << endl;
  outputpp << "	COMPONENT cluster_center is" << endl;
  outputpp << "	port(		new_cluster_A : out  std_logic_vector(3 downto 0);" << endl;
  outputpp << "	new_cluster_B : out  std_logic_vector(3 downto 0);" << endl;
  outputpp << "	new_cluster_D : out  std_logic_vector(3 downto 0);" << endl;
  outputpp << "	new_cluster_E : out  std_logic_vector(3 downto 0);" << endl;
  outputpp << "	new_cluster_G : out  std_logic_vector(3 downto 0);" << endl;
  outputpp << "	new_cluster_H : out  std_logic_vector(3 downto 0);	" << endl;
  outputpp << "	X : IN  std_logic_vector(3 downto 0);" << endl;
  outputpp << "	Y : IN  std_logic_vector(3 downto 0);" << endl;
  outputpp << "	Z : IN  std_logic_vector(3 downto 0);" << endl;
  outputpp << "	A : IN  std_logic_vector(3 downto 0);" << endl;
  outputpp << "	B : IN  std_logic_vector(3 downto 0);" << endl;
  outputpp << "	D : IN  std_logic_vector(3 downto 0);" << endl;
  outputpp << "	E : IN  std_logic_vector(3 downto 0);" << endl;
  outputpp << "	G : IN  std_logic_vector(3 downto 0);" << endl;
  outputpp << "	H : IN  std_logic_vector(3 downto 0);" << endl;
  outputpp << "	top_clkdata_s:IN std_logic" << endl;
  outputpp << "	);" << endl;
  outputpp << "	end COMPONENT;" << endl;
  outputpp << " " << endl;
  outputp << " " << endl;
  outputp << "begin" << endl;
  outputpp << "begin" << endl;
  outputpp << " " << endl;
  outputp << " " << endl;

  ///////////////////////////////////////////////////////////////////
  outputp << "Center_1: UT3_0_HoughVoting_M " << endl;
  outputp << "	Port map(" << endl;
  outputpp << "Center_1: UT3_0_HoughVoting_P" << endl;
  outputpp << "	Port map(" << endl;
  for (unsigned a = 1; a <= 16; a++) {
    outputp << "	row" << a << "(79 downto 40) =>row" << a << "(79 downto 40)," << endl;
    outputpp << "	row" << a << "(39 downto 0) =>row" << a << "_s(39 downto 0)," << endl;
  }
  outputp << "	SL0_TS(80 downto 0)=>SL0_TS(80 downto 0)," << endl;
  outputp << "	SL2_TS(96 downto 0)=>SL2_TS(96 downto 0)," << endl;
  outputp << "	SL4_TS(128 downto 0)=>SL4_TS(128 downto 0)," << endl;
  outputp << "	SL6_TS(160 downto 0)=>SL6_TS(160 downto 0)," << endl;
  outputp << "	SL8_TS(192 downto 0)=>SL8_TS(192 downto 0)," << endl;
  outputp << "	Top_clkData_s =>Top_clkData_s " << endl;
  outputp << "	);" << endl;
  outputpp << "	SL0_TS(80 downto 0)=>SL0_TS(80 downto 0)," << endl;
  outputpp << "	SL2_TS(96 downto 0)=>SL2_TS(96 downto 0)," << endl;
  outputpp << "	SL4_TS(128 downto 0)=>SL4_TS(128 downto 0)," << endl;


  outputpp << "	SL6_TS(160 downto 0)=>SL6_TS(160 downto 0)," << endl;
  outputpp << "	SL8_TS(192 downto 0)=>SL8_TS(192 downto 0)," << endl;
  outputpp << "	Top_clkData_s =>Top_clkData_s " << endl;
  outputpp << "	);" << endl;
  outputpp << " " << endl;
  outputp << " " << endl;

  outputpp << "ss : for i in 0 to 39 generate" << endl;
  for (unsigned a = 1; a <= 16; a++) {
    outputpp << "row" << a << "(i)<=row" << a << "_s(i);" << endl;
  }
  outputpp << "end generate;" << endl;
  outputpp << " " << endl;

  //////Clust_part1////////////////////////////////////////////////////////////////////

  int i = 1, c = 1, z = 0;
  for (unsigned j = 1; j < 16; j += 2) {
    for (unsigned t = z % 160; t < 80; t += 2) {

      if (t >= 40 && t < 80) {
        ++i;
        outputp << "Center_" << i << " :cluster_center" << endl;
        outputp << "port map (" << endl;
      }

      if (t < 40) {
        ++c;
        outputpp << "Center_" << c << " :cluster_center" << endl;
        outputpp << "port map (" << endl;
      }
      //////Minus//////////////////////////////////////////////////////////////////////////
      if (t >= 40 && t <= 78) {
        if (t == 40) {
          if (j == 1) {
            outputp << setw(21) << "X =>\"0000\"," << endl;
            outputp << setw(21) << "Y =>\"0000\"," << endl;
            outputp << setw(21) << "Z =>\"0000\"," << endl;
            outputp << setw(26) << "A(1 downto 0)=>row" << j << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "B(1 downto 0)=>row" << j << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "B(3 downto 2)=>row" << j + 1 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "D(1 downto 0)=>row" << j + 2 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "D(3 downto 2)=>row" << j + 3 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "E(1 downto 0)=>row" << j + 2 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "E(3 downto 2)=>row" << j + 3 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "G(1 downto 0)=>row" << j + 4 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "G(3 downto 2)=>row" << j + 5 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "H(1 downto 0)=>row" << j + 4 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "H(3 downto 2)=>row" << j + 5 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << t << "_" << j << "_A(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << t << "_" << j << "_B(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << t << "_" << j << "_D(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << t << "_" << j << "_E(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << t << "_" << j << "_G(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << t << "_" << j << "_H(3 downto 0)," << endl;
          }
          if (j > 1 and j <= 11) {
            outputp << setw(21) << "X =>\"0000\"," << endl;
            outputp << setw(21) << "Y =>\"0000\"," << endl;
            outputp << setw(26) << "Z(1 downto 0)=>row" << j - 2 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "Z(3 downto 2)=>row" << j - 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(1 downto 0)=>row" << j << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "B(1 downto 0)=>row" << j << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "B(3 downto 2)=>row" << j + 1 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "D(1 downto 0)=>row" << j + 2 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "D(3 downto 2)=>row" << j + 3 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "E(1 downto 0)=>row" << j + 2 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "E(3 downto 2)=>row" << j + 3 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "G(1 downto 0)=>row" << j + 4 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "G(3 downto 2)=>row" << j + 5 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "H(1 downto 0)=>row" << j + 4 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "H(3 downto 2)=>row" << j + 5 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;

            outputp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << t << "_" << j << "_A(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << t << "_" << j << "_B(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << t << "_" << j << "_D(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << t << "_" << j << "_E(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << t << "_" << j << "_G(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << t << "_" << j << "_H(3 downto 0)," << endl;

          }

          if (j == 13) {

            outputp << setw(21) << "X =>\"0000\"," << endl;
            outputp << setw(21) << "Y =>\"0000\"," << endl;
            outputp << setw(26) << "Z(1 downto 0)=>row" << j - 2 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "Z(3 downto 2)=>row" << j - 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(1 downto 0)=>row" << j << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "B(1 downto 0)=>row" << j << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "B(3 downto 2)=>row" << j + 1 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "D(1 downto 0)=>row" << j + 2 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "D(3 downto 2)=>row" << j + 3 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "E(1 downto 0)=>row" << j + 2 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "E(3 downto 2)=>row" << j + 3 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(21) << "G =>\"0000\"," << endl;
            outputp << setw(21) << "H =>\"0000\"," << endl;

            outputp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << t << "_" << j << "_A(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << t << "_" << j << "_B(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << t << "_" << j << "_D(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << t << "_" << j << "_E(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << t << "_" << j << "_G(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << t << "_" << j << "_H(3 downto 0)," << endl;

          }

          if (j == 15) {
            outputp << setw(21) << "X =>\"0000\"," << endl;
            outputp << setw(21) << "Y =>\"0000\"," << endl;
            outputp << setw(26) << "Z(1 downto 0)=>row" << j - 2 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "Z(3 downto 2)=>row" << j - 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(1 downto 0)=>row" << j << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "B(1 downto 0)=>row" << j << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "B(3 downto 2)=>row" << j + 1 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(21) << "D =>\"0000\"," << endl;
            outputp << setw(21) << "E =>\"0000\"," << endl;
            outputp << setw(21) << "G =>\"0000\"," << endl;
            outputp << setw(21) << "H =>\"0000\"," << endl;

            outputp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << t << "_" << j << "_A(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << t << "_" << j << "_B(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << t << "_" << j << "_D(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << t << "_" << j << "_E(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << t << "_" << j << "_G(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << t << "_" << j << "_H(3 downto 0)," << endl;

          }
        }
        if (t == 78) {
          if (j == 1) {
            outputp << setw(26) << "X(1 downto 0)=>row" << j << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "X(3 downto 2)=>row" << j + 1 << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(21) << "Y =>\"0000\"," << endl;
            outputp << setw(21) << "Z =>\"0000\"," << endl;
            outputp << setw(26) << "A(1 downto 0)=>row" << j << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(21) << "B =>\"0000\"," << endl;
            outputp << setw(26) << "D(1 downto 0)=>row" << j + 2 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "D(3 downto 2)=>row" << j + 3 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(21) << "E =>\"0000\"," << endl;
            outputp << setw(26) << "G(1 downto 0)=>row" << j + 4 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "G(3 downto 2)=>row" << j + 5 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(21) << "H =>\"0000\"," << endl;

            outputp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << t << "_" << j << "_A(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << t << "_" << j << "_B(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << t << "_" << j << "_D(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << t << "_" << j << "_E(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << t << "_" << j << "_G(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << t << "_" << j << "_H(3 downto 0)," << endl;
          }

          if (j > 1 and j <= 11) {
            outputp << setw(26) << "X(1 downto 0)=>row" << j << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "X(3 downto 2)=>row" << j + 1 << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "Y(1 downto 0)=>row" << j - 2 << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "Y(3 downto 2)=>row" << j - 1 << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "Z(1 downto 0)=>row" << j - 2 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "Z(3 downto 2)=>row" << j - 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(1 downto 0)=>row" << j << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(21) << "B =>\"0000\"," << endl;
            outputp << setw(26) << "D(1 downto 0)=>row" << j + 2 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "D(3 downto 2)=>row" << j + 3 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(21) << "E =>\"0000\"," << endl;
            outputp << setw(26) << "G(1 downto 0)=>row" << j + 4 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "G(3 downto 2)=>row" << j + 5 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(21) << "H =>\"0000\"," << endl;

            outputp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << t << "_" << j << "_A(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << t << "_" << j << "_B(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << t << "_" << j << "_D(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << t << "_" << j << "_E(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << t << "_" << j << "_G(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << t << "_" << j << "_H(3 downto 0)," << endl;

          }

          if (j == 13) {
            outputp << setw(26) << "X(1 downto 0)=>row" << j << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "X(3 downto 2)=>row" << j + 1 << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "Y(1 downto 0)=>row" << j - 2 << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "Y(3 downto 2)=>row" << j - 1 << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "Z(1 downto 0)=>row" << j - 2 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "Z(3 downto 2)=>row" << j - 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(1 downto 0)=>row" << j << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(21) << "B =>\"0000\"," << endl;
            outputp << setw(26) << "D(1 downto 0)=>row" << j + 2 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "D(3 downto 2)=>row" << j + 3 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(21) << "E =>\"0000\"," << endl;
            outputp << setw(21) << "G =>\"0000\"," << endl;
            outputp << setw(21) << "H =>\"0000\"," << endl;

            outputp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << t << "_" << j << "_A(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << t << "_" << j << "_B(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << t << "_" << j << "_D(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << t << "_" << j << "_E(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << t << "_" << j << "_G(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << t << "_" << j << "_H(3 downto 0)," << endl;

          }

          if (j == 15) {
            outputp << setw(26) << "X(1 downto 0)=>row" << j << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "X(3 downto 2)=>row" << j + 1 << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "Y(1 downto 0)=>row" << j - 2 << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "Y(3 downto 2)=>row" << j - 1 << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "Z(1 downto 0)=>row" << j - 2 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "Z(3 downto 2)=>row" << j - 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(1 downto 0)=>row" << j << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(21) << "B =>\"0000\"," << endl;
            outputp << setw(21) << "D =>\"0000\"," << endl;
            outputp << setw(21) << "E =>\"0000\"," << endl;
            outputp << setw(21) << "G =>\"0000\"," << endl;
            outputp << setw(21) << "H =>\"0000\"," << endl;

            outputp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << t << "_" << j << "_A(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << t << "_" << j << "_B(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << t << "_" << j << "_D(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << t << "_" << j << "_E(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << t << "_" << j << "_G(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << t << "_" << j << "_H(3 downto 0)," << endl;

          }
        }
        if (t > 40 and t < 78) {
          if (j == 1) {
            outputp << setw(26) << "X(1 downto 0)=>row" << j << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "X(3 downto 2)=>row" << j + 1 << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(21) << "Y =>\"0000\"," << endl;
            outputp << setw(21) << "Z =>\"0000\"," << endl;
            outputp << setw(26) << "A(1 downto 0)=>row" << j << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "B(1 downto 0)=>row" << j << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "B(3 downto 2)=>row" << j + 1 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "D(1 downto 0)=>row" << j + 2 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "D(3 downto 2)=>row" << j + 3 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "E(1 downto 0)=>row" << j + 2 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "E(3 downto 2)=>row" << j + 3 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "G(1 downto 0)=>row" << j + 4 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "G(3 downto 2)=>row" << j + 5 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "H(1 downto 0)=>row" << j + 4 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "H(3 downto 2)=>row" << j + 5 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;

            outputp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << t << "_" << j << "_A(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << t << "_" << j << "_B(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << t << "_" << j << "_D(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << t << "_" << j << "_E(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << t << "_" << j << "_G(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << t << "_" << j << "_H(3 downto 0)," << endl;
          }
          if (j > 1 and j <= 11) {

            outputp << setw(26) << "X(1 downto 0)=>row" << j << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "X(3 downto 2)=>row" << j + 1 << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "Y(1 downto 0)=>row" << j - 2 << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "Y(3 downto 2)=>row" << j - 1 << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "Z(1 downto 0)=>row" << j - 2 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "Z(3 downto 2)=>row" << j - 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(1 downto 0)=>row" << j << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "B(1 downto 0)=>row" << j << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "B(3 downto 2)=>row" << j + 1 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "D(1 downto 0)=>row" << j + 2 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "D(3 downto 2)=>row" << j + 3 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "E(1 downto 0)=>row" << j + 2 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "E(3 downto 2)=>row" << j + 3 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "G(1 downto 0)=>row" << j + 4 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "G(3 downto 2)=>row" << j + 5 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "H(1 downto 0)=>row" << j + 4 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "H(3 downto 2)=>row" << j + 5 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;

            outputp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << t << "_" << j << "_A(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << t << "_" << j << "_B(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << t << "_" << j << "_D(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << t << "_" << j << "_E(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << t << "_" << j << "_G(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << t << "_" << j << "_H(3 downto 0)," << endl;

          }

          if (j == 13) {
            outputp << setw(26) << "X(1 downto 0)=>row" << j << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "X(3 downto 2)=>row" << j + 1 << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "Y(1 downto 0)=>row" << j - 2 << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "Y(3 downto 2)=>row" << j - 1 << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "Z(1 downto 0)=>row" << j - 2 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "Z(3 downto 2)=>row" << j - 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(1 downto 0)=>row" << j << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "B(1 downto 0)=>row" << j << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "B(3 downto 2)=>row" << j + 1 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "D(1 downto 0)=>row" << j + 2 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "D(3 downto 2)=>row" << j + 3 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "E(1 downto 0)=>row" << j + 2 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "E(3 downto 2)=>row" << j + 3 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(21) << "G =>\"0000\"," << endl;
            outputp << setw(21) << "H =>\"0000\"," << endl;

            outputp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << t << "_" << j << "_A(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << t << "_" << j << "_B(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << t << "_" << j << "_D(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << t << "_" << j << "_E(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << t << "_" << j << "_G(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << t << "_" << j << "_H(3 downto 0)," << endl;

          }

          if (j == 15) {
            outputp << setw(26) << "X(1 downto 0)=>row" << j << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "X(3 downto 2)=>row" << j + 1 << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "Y(1 downto 0)=>row" << j - 2 << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "Y(3 downto 2)=>row" << j - 1 << "(" << t - 1 << " downto " << t - 2 << ")," << endl;
            outputp << setw(26) << "Z(1 downto 0)=>row" << j - 2 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "Z(3 downto 2)=>row" << j - 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(1 downto 0)=>row" << j << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << t + 1 << " downto " << t << ")," << endl;
            outputp << setw(26) << "B(1 downto 0)=>row" << j << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(26) << "B(3 downto 2)=>row" << j + 1 << "(" << t + 3 << " downto " << t + 2 << ")," << endl;
            outputp << setw(21) << "D =>\"0000\"," << endl;
            outputp << setw(21) << "E =>\"0000\"," << endl;
            outputp << setw(21) << "G =>\"0000\"," << endl;
            outputp << setw(21) << "H =>\"0000\"," << endl;

            outputp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << t << "_" << j << "_A(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << t << "_" << j << "_B(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << t << "_" << j << "_D(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << t << "_" << j << "_E(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << t << "_" << j << "_G(3 downto 0)," << endl;
            outputp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << t << "_" << j << "_H(3 downto 0)," << endl;

          }
        }
      }
      //////Plus//////////////////////////////////////////////////////////////////////////
      if (t <= 38) {
        if (t == 0) {
          if (j == 1) {
            outputpp << setw(21) << "X =>\"0000\"," << endl;
            outputpp << setw(21) << "Y =>\"0000\"," << endl;
            outputpp << setw(21) << "Z =>\"0000\"," << endl;
            outputpp << setw(26) << "A(1 downto 0)=>row" << j << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "B(1 downto 0)=>row" << j << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "B(3 downto 2)=>row" << j + 1 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "D(1 downto 0)=>row" << j + 2 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "D(3 downto 2)=>row" << j + 3 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "E(1 downto 0)=>row" << j + 2 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "E(3 downto 2)=>row" << j + 3 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "G(1 downto 0)=>row" << j + 4 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "G(3 downto 2)=>row" << j + 5 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "H(1 downto 0)=>row" << j + 4 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "H(3 downto 2)=>row" << j + 5 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_A(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_B(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_D(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_E(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_G(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_H(3 downto 0)," << endl;
          }
          if (j > 1 and j <= 11) {
            outputpp << setw(21) << "X =>\"0000\"," << endl;
            outputpp << setw(21) << "Y =>\"0000\"," << endl;
            outputpp << setw(26) << "Z(1 downto 0)=>row" << j - 2 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "Z(3 downto 2)=>row" << j - 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(1 downto 0)=>row" << j << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "B(1 downto 0)=>row" << j << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "B(3 downto 2)=>row" << j + 1 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "D(1 downto 0)=>row" << j + 2 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "D(3 downto 2)=>row" << j + 3 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "E(1 downto 0)=>row" << j + 2 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "E(3 downto 2)=>row" << j + 3 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "G(1 downto 0)=>row" << j + 4 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "G(3 downto 2)=>row" << j + 5 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "H(1 downto 0)=>row" << j + 4 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "H(3 downto 2)=>row" << j + 5 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_A(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_B(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_D(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_E(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_G(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_H(3 downto 0)," << endl;
          }
          if (j == 13) {
            outputpp << setw(21) << "X =>\"0000\"," << endl;
            outputpp << setw(21) << "Y =>\"0000\"," << endl;
            outputpp << setw(26) << "Z(1 downto 0)=>row" << j - 2 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "Z(3 downto 2)=>row" << j - 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(1 downto 0)=>row" << j << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "B(1 downto 0)=>row" << j << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "B(3 downto 2)=>row" << j + 1 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "D(1 downto 0)=>row" << j + 2 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "D(3 downto 2)=>row" << j + 3 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "E(1 downto 0)=>row" << j + 2 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "E(3 downto 2)=>row" << j + 3 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(21) << "G =>\"0000\"," << endl;
            outputpp << setw(21) << "H =>\"0000\"," << endl;
            outputpp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_A(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_B(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_D(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_E(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_G(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_H(3 downto 0)," << endl;
          }
          if (j == 15) {
            outputpp << setw(21) << "X =>\"0000\"," << endl;
            outputpp << setw(21) << "Y =>\"0000\"," << endl;
            outputpp << setw(26) << "Z(1 downto 0)=>row" << j - 2 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "Z(3 downto 2)=>row" << j - 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(1 downto 0)=>row" << j << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "B(1 downto 0)=>row" << j << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "B(3 downto 2)=>row" << j + 1 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(21) << "D =>\"0000\"," << endl;
            outputpp << setw(21) << "E =>\"0000\"," << endl;
            outputpp << setw(21) << "G =>\"0000\"," << endl;
            outputpp << setw(21) << "H =>\"0000\"," << endl;
            outputpp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_A(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_B(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_D(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_E(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_G(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_H(3 downto 0)," << endl;
          }
        }
        if (t == 38) {
          if (j == 1) {
            outputpp << setw(26) << "X(1 downto 0)=>row" << j << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "X(3 downto 2)=>row" << j + 1 << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(21) << "Y =>\"0000\"," << endl;
            outputpp << setw(21) << "Z =>\"0000\"," << endl;
            outputpp << setw(26) << "A(1 downto 0)=>row" << j << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(21) << "B =>\"0000\"," << endl;
            outputpp << setw(26) << "D(1 downto 0)=>row" << j + 2 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "D(3 downto 2)=>row" << j + 3 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(21) << "E =>\"0000\"," << endl;
            outputpp << setw(26) << "G(1 downto 0)=>row" << j + 4 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "G(3 downto 2)=>row" << j + 5 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(21) << "H =>\"0000\"," << endl;
            outputpp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_A(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_B(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_D(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_E(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_G(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_H(3 downto 0)," << endl;
          }
          if (j > 1 and j <= 11) {
            outputpp << setw(26) << "X(1 downto 0)=>row" << j << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "X(3 downto 2)=>row" << j + 1 << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "Y(1 downto 0)=>row" << j - 2 << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "Y(3 downto 2)=>row" << j - 1 << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "Z(1 downto 0)=>row" << j - 2 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "Z(3 downto 2)=>row" << j - 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(1 downto 0)=>row" << j << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(21) << "B =>\"0000\"," << endl;
            outputpp << setw(26) << "D(1 downto 0)=>row" << j + 2 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "D(3 downto 2)=>row" << j + 3 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(21) << "E =>\"0000\"," << endl;
            outputpp << setw(26) << "G(1 downto 0)=>row" << j + 4 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "G(3 downto 2)=>row" << j + 5 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(21) << "H =>\"0000\"," << endl;
            outputpp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_A(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_B(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_D(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_E(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_G(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_H(3 downto 0)," << endl;
          }
          if (j == 13) {
            outputpp << setw(26) << "X(1 downto 0)=>row" << j << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "X(3 downto 2)=>row" << j + 1 << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "Y(1 downto 0)=>row" << j - 2 << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "Y(3 downto 2)=>row" << j - 1 << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "Z(1 downto 0)=>row" << j - 2 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "Z(3 downto 2)=>row" << j - 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(1 downto 0)=>row" << j << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(21) << "B =>\"0000\"," << endl;
            outputpp << setw(26) << "D(1 downto 0)=>row" << j + 2 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "D(3 downto 2)=>row" << j + 3 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(21) << "E =>\"0000\"," << endl;
            outputpp << setw(21) << "G =>\"0000\"," << endl;
            outputpp << setw(21) << "H =>\"0000\"," << endl;
            outputpp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_A(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_B(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_D(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_E(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_G(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_H(3 downto 0)," << endl;
          }
          if (j == 15) {
            outputpp << setw(26) << "X(1 downto 0)=>row" << j << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "X(3 downto 2)=>row" << j + 1 << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "Y(1 downto 0)=>row" << j - 2 << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "Y(3 downto 2)=>row" << j - 1 << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "Z(1 downto 0)=>row" << j - 2 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "Z(3 downto 2)=>row" << j - 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(1 downto 0)=>row" << j << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(21) << "B =>\"0000\"," << endl;
            outputpp << setw(21) << "D =>\"0000\"," << endl;
            outputpp << setw(21) << "E =>\"0000\"," << endl;
            outputpp << setw(21) << "G =>\"0000\"," << endl;
            outputpp << setw(21) << "H =>\"0000\"," << endl;
            outputpp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_A(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_B(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_D(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_E(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_G(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_H(3 downto 0)," << endl;
          }
        }
        if (t > 0 and t < 38) {
          if (j == 1) {
            outputpp << setw(26) << "X(1 downto 0)=>row" << j << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "X(3 downto 2)=>row" << j + 1 << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(21) << "Y =>\"0000\"," << endl;
            outputpp << setw(21) << "Z =>\"0000\"," << endl;
            outputpp << setw(26) << "A(1 downto 0)=>row" << j << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "B(1 downto 0)=>row" << j << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "B(3 downto 2)=>row" << j + 1 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "D(1 downto 0)=>row" << j + 2 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "D(3 downto 2)=>row" << j + 3 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "E(1 downto 0)=>row" << j + 2 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "E(3 downto 2)=>row" << j + 3 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "G(1 downto 0)=>row" << j + 4 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "G(3 downto 2)=>row" << j + 5 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "H(1 downto 0)=>row" << j + 4 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "H(3 downto 2)=>row" << j + 5 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_A(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_B(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_D(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_E(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_G(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_H(3 downto 0)," << endl;
          }
          if (j > 1 and j <= 11) {
            outputpp << setw(26) << "X(1 downto 0)=>row" << j << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "X(3 downto 2)=>row" << j + 1 << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "Y(1 downto 0)=>row" << j - 2 << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "Y(3 downto 2)=>row" << j - 1 << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "Z(1 downto 0)=>row" << j - 2 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "Z(3 downto 2)=>row" << j - 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(1 downto 0)=>row" << j << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "B(1 downto 0)=>row" << j << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "B(3 downto 2)=>row" << j + 1 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "D(1 downto 0)=>row" << j + 2 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "D(3 downto 2)=>row" << j + 3 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "E(1 downto 0)=>row" << j + 2 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "E(3 downto 2)=>row" << j + 3 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "G(1 downto 0)=>row" << j + 4 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "G(3 downto 2)=>row" << j + 5 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "H(1 downto 0)=>row" << j + 4 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "H(3 downto 2)=>row" << j + 5 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_A(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_B(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_D(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_E(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_G(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_H(3 downto 0)," << endl;
          }
          if (j == 13) {
            outputpp << setw(26) << "X(1 downto 0)=>row" << j << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "X(3 downto 2)=>row" << j + 1 << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "Y(1 downto 0)=>row" << j - 2 << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "Y(3 downto 2)=>row" << j - 1 << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "Z(1 downto 0)=>row" << j - 2 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "Z(3 downto 2)=>row" << j - 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(1 downto 0)=>row" << j << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "B(1 downto 0)=>row" << j << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "B(3 downto 2)=>row" << j + 1 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "D(1 downto 0)=>row" << j + 2 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "D(3 downto 2)=>row" << j + 3 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "E(1 downto 0)=>row" << j + 2 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "E(3 downto 2)=>row" << j + 3 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(21) << "G =>\"0000\"," << endl;
            outputpp << setw(21) << "H =>\"0000\"," << endl;
            outputpp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_A(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_B(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_D(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_E(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_G(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_H(3 downto 0)," << endl;
          }
          if (j == 15) {
            outputpp << setw(26) << "X(1 downto 0)=>row" << j << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "X(3 downto 2)=>row" << j + 1 << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "Y(1 downto 0)=>row" << j - 2 << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "Y(3 downto 2)=>row" << j - 1 << "(" << 40 - t << " to " << 41 - t << ")," << endl;
            outputpp << setw(26) << "Z(1 downto 0)=>row" << j - 2 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "Z(3 downto 2)=>row" << j - 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(1 downto 0)=>row" << j << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "A(3 downto 2)=>row" << j + 1 << "(" << 38 - t << " to " << 39 - t << ")," << endl;
            outputpp << setw(26) << "B(1 downto 0)=>row" << j << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(26) << "B(3 downto 2)=>row" << j + 1 << "(" << 36 - t << " to " << 37 - t << ")," << endl;
            outputpp << setw(21) << "D =>\"0000\"," << endl;
            outputpp << setw(21) << "E =>\"0000\"," << endl;
            outputpp << setw(21) << "G =>\"0000\"," << endl;
            outputpp << setw(21) << "H =>\"0000\"," << endl;
            outputpp << setw(43) << "new_cluster_A(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_A(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_B(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_B(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_D(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_D(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_E(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_E(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_G(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_G(3 downto 0)," << endl;
            outputpp << setw(43) << "new_cluster_H(3 downto 0) =>patternI_" << 39 - t << "_" << j << "_H(3 downto 0)," << endl;
          }
        }
      }


      if (t >= 40 && t < 80) {
        outputp << setw(40) << "top_clkData_s =>top_clkData_s );" << endl;
        outputp << " " << endl;
      }
      if (t < 40) {
        outputpp << setw(40) << "top_clkData_s =>top_clkData_s );" << endl;
        outputpp << " " << endl;
      }
    }
  }

  //////Clust_part2////////////////////////////////////////////////////////////////////
  //////Minus//////////////////////////////////////////////////////////////////////////
  for (int jj = 1; jj <= 15; jj += 2) {
    for (int tt = 0; tt < 80; tt += 2) {
      if (tt >= 40 && tt < 80) {
        if (jj == 1 or jj == 15) {
          if (tt == 40) {
            outputp << "Minus_row" << jj << "(" << tt + 1 << " downto " << tt << ")<=patternI_" << tt << "_" << jj << "_A(1 downto 0);" << endl;
            outputp << "Minus_row" << jj + 1 << "(" << tt + 1 << " downto " << tt << ")<=patternI_" << tt << "_" << jj << "_A(3 downto 2);" <<
                    endl;
            outputp << " " << endl;
          }
          if (tt != 40 && jj == 1) {
            outputp << "Minus_row" << jj << "(" << tt << ")<=patternI_" << tt << "_" << jj << "_A(0) or patternI_" << tt - 2 << "_" << jj <<
                    "_B(0);" << endl;
            outputp << "Minus_row" << jj << "(" << tt + 1 << ")<=patternI_" << tt << "_" << jj << "_A(1) or patternI_" << tt - 2 << "_" << jj <<
                    "_B(1);" << endl;
            outputp << "Minus_row" << jj + 1 << "(" << tt << ")<=patternI_" << tt << "_" << jj << "_A(2) or patternI_" << tt - 2 << "_" << jj <<
                    "_B(2);" << endl;
            outputp << "Minus_row" << jj + 1 << "(" << tt + 1 << ")<=patternI_" << tt << "_" << jj << "_A(3) or patternI_" << tt - 2 << "_" <<
                    jj << "_B(3);" << endl;
            outputp << " " << endl;
          }
          if (tt != 40 && jj == 15) {
            outputp << "Minus_row" << jj << "(" << tt << ")<=patternI_" << tt << "_" << jj << "_A(0) or patternI_" << tt - 2 << "_" << jj <<
                    "_B(0) or patternI_";
            outputp << tt << "_" << jj - 2 << "_D(0) or patternI_" << tt - 2 << "_" << jj - 2 << "_E(0) or patternI_" << tt << "_" << jj - 4 <<
                    "_G(0) or patternI_" << tt - 2 << "_" << jj - 4 << "_H(0);" << endl;
            outputp << "Minus_row" << jj << "(" << tt + 1 << ")<=patternI_" << tt << "_" << jj << "_A(1) or patternI_" << tt - 2 << "_" << jj <<
                    "_B(1) or patternI_";
            outputp << tt << "_" << jj - 2 << "_D(1) or patternI_" << tt - 2 << "_" << jj - 2 << "_E(1) or patternI_" << tt << "_" << jj - 4 <<
                    "_G(1) or patternI_" << tt - 2 << "_" << jj - 4 << "_H(1);" << endl;
            outputp << "Minus_row" << jj + 1 << "(" << tt << ")<=patternI_" << tt << "_" << jj << "_A(2) or patternI_" << tt - 2 << "_" << jj <<
                    "_B(2) or patternI_";
            outputp << tt << "_" << jj - 2 << "_D(2) or patternI_" << tt - 2 << "_" << jj - 2 << "_E(2) or patternI_" << tt << "_" << jj - 4 <<
                    "_G(2) or patternI_" << tt - 2 << "_" << jj - 4 << "_H(2);" << endl;
            outputp << "Minus_row" << jj + 1 << "(" << tt + 1 << ")<=patternI_" << tt << "_" << jj << "_A(3) or patternI_" << tt - 2 << "_" <<
                    jj << "_B(3) or patternI_";
            outputp << tt << "_" << jj - 2 << "_D(3) or patternI_" << tt - 2 << "_" << jj - 2 << "_E(3) or patternI_" << tt << "_" << jj - 4 <<
                    "_G(3) or patternI_" << tt - 2 << "_" << jj - 4 << "_H(3);" << endl;
            outputp << " " << endl;
          }
        }
        if (jj >= 3 && jj < 15) {
          if (jj == 3 && tt == 40) {
            outputp << "Minus_row" << jj << "(" << tt << ")<=patternI_" << tt << "_" << jj << "_A(0) or patternI_" << tt << "_" << jj - 2 <<
                    "_D(0);" << endl;
            outputp << "Minus_row" << jj << "(" << tt + 1 << ")<=patternI_" << tt << "_" << jj << "_A(1) or patternI_" << tt << "_" << jj - 2 <<
                    "_D(1);" << endl;
            outputp << "Minus_row" << jj + 1 << "(" << tt << ")<=patternI_" << tt << "_" << jj << "_A(2) or patternI_" << tt << "_" << jj - 2 <<
                    "_D(2);" << endl;
            outputp << "Minus_row" << jj + 1 << "(" << tt + 1 << ")<=patternI_" << tt << "_" << jj << "_A(3) or patternI_" << tt << "_" << jj -
                    2 << "_D(3);" << endl;
          }
          if (jj == 3 && tt != 40) {
            outputp << "Minus_row" << jj << "(" << tt << ")<=patternI_" << tt << "_" << jj << "_A(0) or patternI_" << tt - 2 << "_" << jj <<
                    "_B(0) or patternI_" << tt << "_" << jj - 2 << "_D(0) or patternI_" << tt - 2 << "_" << jj - 2 << "_E(0);" << endl;
            outputp << "Minus_row" << jj << "(" << tt + 1 << ")<=patternI_" << tt << "_" << jj << "_A(1) or patternI_" << tt - 2 << "_" << jj <<
                    "_B(1) or patternI_" << tt << "_" << jj - 2 << "_D(1) or patternI_" << tt - 2 << "_" << jj - 2 << "_E(1);" << endl;
            outputp << "Minus_row" << jj + 1 << "(" << tt << ")<=patternI_" << tt << "_" << jj << "_A(2) or patternI_" << tt - 2 << "_" << jj <<
                    "_B(2) or patternI_" << tt << "_" << jj - 2 << "_D(2) or patternI_" << tt - 2 << "_" << jj - 2 << "_E(2);" << endl;
            outputp << "Minus_row" << jj + 1 << "(" << tt + 1 << ")<=patternI_" << tt << "_" << jj << "_A(3) or patternI_" << tt - 2 << "_" <<
                    jj << "_B(3) or patternI_" << tt << "_" << jj - 2 << "_D(3) or patternI_" << tt - 2 << "_" << jj - 2 << "_E(3);" << endl;
            outputp << " " << endl;
          }
          if (jj != 3 && tt == 40) {
            outputp << "Minus_row" << jj << "(" << tt << ")<=patternI_" << tt << "_" << jj << "_A(0) or patternI_" << tt << "_" << jj - 2 <<
                    "_D(0) or patternI_" << tt << "_" << jj - 4 << "_G(0);" << endl;
            outputp << "Minus_row" << jj << "(" << tt + 1 << ")<=patternI_" << tt << "_" << jj << "_A(1) or patternI_" << tt << "_" << jj - 2 <<
                    "_D(1) or patternI_" << tt << "_" << jj - 4 << "_G(1);" << endl;
            outputp << "Minus_row" << jj + 1 << "(" << tt << ")<=patternI_" << tt << "_" << jj << "_A(2) or patternI_" << tt << "_" << jj - 2 <<
                    "_D(2) or patternI_" << tt << "_" << jj - 4 << "_G(2);" << endl;
            outputp << "Minus_row" << jj + 1 << "(" << tt + 1 << ")<=patternI_" << tt << "_" << jj << "_A(3) or patternI_" << tt << "_" << jj -
                    2 << "_D(3) or patternI_" << tt << "_" << jj - 4 << "_G(3);" << endl;
            outputp << " " << endl;
          }
          if (jj != 3 && tt != 40) {
            outputp << "Minus_row" << jj << "(" << tt << ")<=patternI_" << tt << "_" << jj << "_A(0) or patternI_" << tt - 2 << "_" << jj <<
                    "_B(0) or patternI_" << tt << "_" << jj - 2 << "_D(0) or patternI_" << tt - 2 << "_" << jj - 2 << "_E(0) or patternI_" << tt << "_"
                    << jj - 4 << "_G(0) or patternI_" << tt - 2 << "_" << jj - 4 << "_H(0);" << endl;
            outputp << "Minus_row" << jj << "(" << tt + 1 << ")<=patternI_" << tt << "_" << jj << "_A(1) or patternI_" << tt - 2 << "_" << jj <<
                    "_B(1) or patternI_" << tt << "_" << jj - 2 << "_D(1) or patternI_" << tt - 2 << "_" << jj - 2 << "_E(1) or patternI_" << tt << "_"
                    << jj - 4 << "_G(1) or patternI_" << tt - 2 << "_" << jj - 4 << "_H(1);" << endl;
            outputp << "Minus_row" << jj + 1 << "(" << tt << ")<=patternI_" << tt << "_" << jj << "_A(2) or patternI_" << tt - 2 << "_" << jj <<
                    "_B(2) or patternI_" << tt << "_" << jj - 2 << "_D(2) or patternI_" << tt - 2 << "_" << jj - 2 << "_E(2) or patternI_" << tt << "_"
                    << jj - 4 << "_G(2) or patternI_" << tt - 2 << "_" << jj - 4 << "_H(2);" << endl;
            outputp << "Minus_row" << jj + 1 << "(" << tt + 1 << ")<=patternI_" << tt << "_" << jj << "_A(3) or patternI_" << tt - 2 << "_" <<
                    jj << "_B(3) or patternI_" << tt << "_" << jj - 2 << "_D(3) or patternI_" << tt - 2 << "_" << jj - 2 << "_E(3) or patternI_" << tt
                    << "_" << jj - 4 << "_G(3) or patternI_" << tt - 2 << "_" << jj - 4 << "_H(3);" << endl;
            outputp << " " << endl;
          }
        }
      }
      //////Plus///////////////////////////////////////////////////////////////////////////
      if (tt >= 0 && tt < 40) {
        if (jj == 1 or jj == 15) {
          if (tt == 38) {
            outputpp << "Plus_row" << jj << "(" << tt + 1 << ")<=patternI_" << tt + 1 << "_" << jj << "_A(0);" << endl;
            outputpp << "Plus_row" << jj << "(" << tt << ")<=patternI_" << tt + 1 << "_" << jj << "_A(1);" << endl;
            outputpp << "Plus_row" << jj + 1 << "(" << tt + 1 << ")<=patternI_" << tt + 1 << "_" << jj << "_A(2);" << endl;
            outputpp << "Plus_row" << jj + 1 << "(" << tt << ")<=patternI_" << tt + 1 << "_" << jj << "_A(3);" << endl;
            outputpp << " " << endl;
          }
          if (tt != 38 && jj == 1) {
            outputpp << "Plus_row" << jj << "(" << tt + 1 << ")<=patternI_" << tt + 1 << "_" << jj << "_A(0) or patternI_" << tt + 3 << "_" <<
                     jj << "_B(0);" << endl;
            outputpp << "Plus_row" << jj << "(" << tt << ")<=patternI_" << tt + 1 << "_" << jj << "_A(1) or patternI_" << tt + 3 << "_" << jj <<
                     "_B(1);" << endl;
            outputpp << "Plus_row" << jj + 1 << "(" << tt + 1 << ")<=patternI_" << tt + 1 << "_" << jj << "_A(2) or patternI_" << tt + 3 << "_"
                     << jj << "_B(2);" << endl;
            outputpp << "Plus_row" << jj + 1 << "(" << tt << ")<=patternI_" << tt + 1 << "_" << jj << "_A(3) or patternI_" << tt + 3 << "_" <<
                     jj << "_B(3);" << endl;
            outputpp << " " << endl;
          }
          if (tt != 38 && jj == 15) {
            outputpp << "Plus_row" << jj << "(" << tt + 1 << ")<=patternI_" << tt + 1 << "_" << jj << "_A(0) or patternI_" << tt + 3 << "_" <<
                     jj << "_B(0) or patternI_";
            outputpp << tt + 1 << "_" << jj - 2 << "_D(0) or patternI_" << tt + 3 << "_" << jj - 2 << "_E(0) or patternI_" << tt + 1 << "_" <<
                     jj - 4 << "_G(0) or patternI_" << tt + 3 << "_" << jj - 4 << "_H(0);" << endl;
            outputpp << "Plus_row" << jj << "(" << tt << ")<=patternI_" << tt + 1 << "_" << jj << "_A(1) or patternI_" << tt + 3 << "_" << jj <<
                     "_B(1) or patternI_";
            outputpp << tt + 1 << "_" << jj - 2 << "_D(1) or patternI_" << tt + 3 << "_" << jj - 2 << "_E(1) or patternI_" << tt + 1 << "_" <<
                     jj - 4 << "_G(1) or patternI_" << tt + 3 << "_" << jj - 4 << "_H(1);" << endl;
            outputpp << "Plus_row" << jj + 1 << "(" << tt + 1 << ")<=patternI_" << tt + 1 << "_" << jj << "_A(2) or patternI_" << tt + 3 << "_"
                     << jj << "_B(2) or patternI_";
            outputpp << tt + 1 << "_" << jj - 2 << "_D(2) or patternI_" << tt + 3 << "_" << jj - 2 << "_E(2) or patternI_" << tt + 1 << "_" <<
                     jj - 4 << "_G(2) or patternI_" << tt + 3 << "_" << jj - 4 << "_H(2);" << endl;
            outputpp << "Plus_row" << jj + 1 << "(" << tt << ")<=patternI_" << tt + 1 << "_" << jj << "_A(3) or patternI_" << tt + 3 << "_" <<
                     jj << "_B(3) or patternI_";
            outputpp << tt + 1 << "_" << jj - 2 << "_D(3) or patternI_" << tt + 3 << "_" << jj - 2 << "_E(3) or patternI_" << tt + 1 << "_" <<
                     jj - 4 << "_G(3) or patternI_" << tt + 3 << "_" << jj - 4 << "_H(3);" << endl;
            outputpp << " " << endl;
          }
        }
        if (jj >= 3 && jj < 15) {
          if (jj == 3 && tt == 38) {
            outputpp << "Plus_row" << jj << "(" << tt + 1 << ")<=patternI_" << tt + 1 << "_" << jj << "_A(0) or patternI_" << tt + 1 << "_" <<
                     jj - 2 << "_D(0);" << endl;
            outputpp << "Plus_row" << jj << "(" << tt << ")<=patternI_" << tt + 1 << "_" << jj << "_A(1) or patternI_" << tt + 1 << "_" << jj -
                     2 << "_D(1);" << endl;
            outputpp << "Plus_row" << jj + 1 << "(" << tt + 1 << ")<=patternI_" << tt + 1 << "_" << jj << "_A(2) or patternI_" << tt + 1 << "_"
                     << jj - 2 << "_D(2);" << endl;
            outputpp << "Plus_row" << jj + 1 << "(" << tt << ")<=patternI_" << tt + 1 << "_" << jj << "_A(3) or patternI_" << tt + 1 << "_" <<
                     jj - 2 << "_D(3);" << endl;
          }
          if (jj == 3 && tt != 38) {
            outputpp << "Plus_row" << jj << "(" << tt + 1 << ")<=patternI_" << tt + 1 << "_" << jj << "_A(0) or patternI_" << tt + 3 << "_" <<
                     jj << "_B(0) or patternI_" << tt + 1 << "_" << jj - 2 << "_D(0) or patternI_" << tt + 3 << "_" << jj - 2 << "_E(0);" << endl;
            outputpp << "Plus_row" << jj << "(" << tt << ")<=patternI_" << tt + 1 << "_" << jj << "_A(1) or patternI_" << tt + 3 << "_" << jj <<
                     "_B(1) or patternI_" << tt + 1 << "_" << jj - 2 << "_D(1) or patternI_" << tt + 3 << "_" << jj - 2 << "_E(1);" << endl;
            outputpp << "Plus_row" << jj + 1 << "(" << tt + 1 << ")<=patternI_" << tt + 1 << "_" << jj << "_A(2) or patternI_" << tt + 3 << "_"
                     << jj << "_B(2) or patternI_" << tt + 1 << "_" << jj - 2 << "_D(2) or patternI_" << tt + 3 << "_" << jj - 2 << "_E(2);" << endl;
            outputpp << "Plus_row" << jj + 1 << "(" << tt << ")<=patternI_" << tt + 1 << "_" << jj << "_A(3) or patternI_" << tt + 3 << "_" <<
                     jj << "_B(3) or patternI_" << tt + 1 << "_" << jj - 2 << "_D(3) or patternI_" << tt + 3 << "_" << jj - 2 << "_E(3);" << endl;
            outputpp << " " << endl;
          }
          if (jj != 3 && tt == 38) {
            outputpp << "Plus_row" << jj << "(" << tt + 1 << ")<=patternI_" << tt + 1 << "_" << jj << "_A(0) or patternI_" << tt + 1 << "_" <<
                     jj - 2 << "_D(0) or patternI_" << tt + 1 << "_" << jj - 4 << "_G(0);" << endl;
            outputpp << "Plus_row" << jj << "(" << tt << ")<=patternI_" << tt + 1 << "_" << jj << "_A(1) or patternI_" << tt + 1 << "_" << jj -
                     2 << "_D(1) or patternI_" << tt + 1 << "_" << jj - 4 << "_G(1);" << endl;
            outputpp << "Plus_row" << jj + 1 << "(" << tt + 1 << ")<=patternI_" << tt + 1 << "_" << jj << "_A(2) or patternI_" << tt + 1 << "_"
                     << jj - 2 << "_D(2) or patternI_" << tt + 1 << "_" << jj - 4 << "_G(2);" << endl;
            outputpp << "Plus_row" << jj + 1 << "(" << tt << ")<=patternI_" << tt + 1 << "_" << jj << "_A(3) or patternI_" << tt + 1 << "_" <<
                     jj - 2 << "_D(3) or patternI_" << tt + 1 << "_" << jj - 4 << "_G(3);" << endl;
            outputpp << " " << endl;
          }
          if (jj != 3 && tt != 38) {
            outputpp << "Plus_row" << jj << "(" << tt + 1 << ")<=patternI_" << tt + 1 << "_" << jj << "_A(0) or patternI_" << tt + 3 << "_" <<
                     jj << "_B(0) or patternI_" << tt + 1 << "_" << jj - 2 << "_D(0) or patternI_" << tt + 3 << "_" << jj - 2 << "_E(0) or patternI_" <<
                     tt + 1 << "_" << jj - 4 << "_G(0) or patternI_" << tt + 3 << "_" << jj - 4 << "_H(0);" << endl;
            outputpp << "Plus_row" << jj << "(" << tt << ")<=patternI_" << tt + 1 << "_" << jj << "_A(1) or patternI_" << tt + 3 << "_" << jj <<
                     "_B(1) or patternI_" << tt + 1 << "_" << jj - 2 << "_D(1) or patternI_" << tt + 3 << "_" << jj - 2 << "_E(1) or patternI_" << tt + 1
                     << "_" << jj - 4 << "_G(1) or patternI_" << tt + 3 << "_" << jj - 4 << "_H(1);" << endl;
            outputpp << "Plus_row" << jj + 1 << "(" << tt + 1 << ")<=patternI_" << tt + 1 << "_" << jj << "_A(2) or patternI_" << tt + 3 << "_"
                     << jj << "_B(2) or patternI_" << tt + 1 << "_" << jj - 2 << "_D(2) or patternI_" << tt + 3 << "_" << jj - 2 << "_E(2) or patternI_"
                     << tt + 1 << "_" << jj - 4 << "_G(2) or patternI_" << tt + 3 << "_" << jj - 4 << "_H(2);" << endl;
            outputpp << "Plus_row" << jj + 1 << "(" << tt << ")<=patternI_" << tt + 1 << "_" << jj << "_A(3) or patternI_" << tt + 3 << "_" <<
                     jj << "_B(3) or patternI_" << tt + 1 << "_" << jj - 2 << "_D(3) or patternI_" << tt + 3 << "_" << jj - 2 << "_E(3) or patternI_" <<
                     tt + 1 << "_" << jj - 4 << "_G(3) or patternI_" << tt + 3 << "_" << jj - 4 << "_H(3);" << endl;
            outputpp << " " << endl;
          }
        }
      }
    }
  }


  outputp << "end Behavioral;" << endl;
  outputpp << "end Behavioral;" << endl;
  outputp << " " << endl;
  outputpp << " " << endl;
  return 0;
}
