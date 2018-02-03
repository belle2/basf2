#!/usr/bin/env python3

import sys
# By default "import ROOT" parses the arguments *sigh* so the most convenient
# way to make sure that basf2 does not import ROOT is to add a --help and make
# sure the program does not stop there
sys.argv.append("--help")
import basf2

# if the program is still alive make sure that the constants we use in basf2.py
# are still the same
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True
import ROOT
assert ROOT.kIsStatic == basf2._ROOT_kIsStatic, "kIsStatic has changed its value"
assert ROOT.kIsConstMethod == basf2._ROOT_kIsConstMethod, "kIsConstMethod has changed its value"
