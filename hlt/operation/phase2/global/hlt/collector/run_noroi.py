#! /usr/bin/env python

import os
import sys
import signal
import subprocess

global inputrb
global outputrb

global procbasf2
global procrb2mrb
global procmrb2rb

global ncol

global inrblist
global outrblist

# Signal Handler


def cleanup(num, frame):
    os.kill(procrb2mrb.pid, signal.SIGINT)
    os.kill(procmrb2rb.pid, signal.SIGINT)
    for i in range(0, ncol):
        os.kill(procbasf2[i].pid, signal.SIGKILL)
        subprocess.call(["removerb", inrblist[i]])
        subprocess.call(["removerb", outrblist[i]])
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

# Number of collector basf2 processes
ncol = 3

# Collector script
col_script = "noroi.py"

inrblist = []
outrblist = []

for i in range(0, ncol):
    inrblist.append(inputrb + str(i + 1))
    outrblist.append(outputrb + str(i + 1))

print inrblist
print outrblist

for x in inrblist:
    subprocess.call("createrb " + x, shell='True')
for x in outrblist:
    subprocess.call("createrb " + x, shell='True')

# Run rb2mrb
cmdrb2mrb = "rb2mrb " + inputrb + " "
cmdmrb2rb = "mrb2rb "
for i in range(0, ncol):
    cmdrb2mrb += (inrblist[i] + " ")
    cmdmrb2rb += (outrblist[i] + " ")
cmdmrb2rb += outputrb

# Run rb2mrb
print cmdrb2mrb
procrb2mrb = subprocess.Popen(cmdrb2mrb, shell='True')
# os.system ("rb2mrb " + inputrb + " " + inrblist[0] + " &" )

# Run mrb2rb
print cmdmrb2rb
procmrb2rb = subprocess.Popen(cmdmrb2rb, shell='True')
# os.system ("mrb2rb " + outrblist[0] + " " + outputrb + " &" )

procbasf2 = []
# run basf2
for i in range(0, ncol):
    basf2cmd = "basf2 --no-stats " + col_script + " " + inrblist[i] + " " + outrblist[i]
    print basf2cmd
    procbasf2.append(subprocess.Popen(basf2cmd, shell='True'))

# Sleep
while True:
    signal.pause()
