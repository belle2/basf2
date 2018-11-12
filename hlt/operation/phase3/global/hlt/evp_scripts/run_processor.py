#! /usr/bin/env python

import os
import sys
import signal
import subprocess

global proc


# Signal Handler
def cleanup(num, frame):
    #    os.kill(proc.pid, signal.SIGKILL)
    #    os.kill(proc.pid, signal.SIGINT)
    os.killpg(proc.pid, signal.SIGINT)
    exit()

# main


# Signal Handler
signal.signal(signal.SIGINT, cleanup)
signal.signal(signal.SIGTERM, cleanup)

# Obtain input/output RingBuffer names
argvs = sys.argv
argc = len(argvs)

inputrb = argvs[1]
outputrb = argvs[2]
histport = argvs[3]
# ncore = argvs[4]
ncore = "16"

# script = "processor.py"
# script = "roitest.py"
# script = "hlt_dhp_test.py"
# script = "hlt_dhp_test_d180116.py"
# script = "hlt_dhp_test_d180220.py"
script = "roitest_cosmic.py"


# run basf2
proc = subprocess.Popen("basf2 --no-stats " + script + " " + inputrb + " " + outputrb +
                        " " + histport + " " + ncore, shell=True, preexec_fn=os.setsid)

# Just sleep
proc.wait()
