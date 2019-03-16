#! /usr/bin/env python

import os
import sys
import signal
import subprocess
import time

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
    print("Cleaning Up Collector")
    sys.stdout.flush()
    os.kill(procrb2mrb.pid, signal.SIGINT)
    procrb2mrb.wait()
    os.kill(procmrb2rb.pid, signal.SIGINT)
    procmrb2rb.wait()

    for i in range(0, ncol):
        #        os.kill(procbasf2[i].pid, signal.SIGKILL)
        print("killing basf2 " + str(procbasf2[i].pid))
        try:
            os.kill(procbasf2[i].pid, signal.SIGINT)
            time.sleep(1)
            os.kill(procbasf2[i].pid, signal.SIGINT)
            time.sleep(1)
        except BaseException:
            print("error to kill basf2 " + str(procbasf2[i]))
        giveup_time = time.time() + 15
        while os.waitpid(procbasf2[i].pid, os.WNOHANG) == (0, 0):
            print("Watiting for process to be completed")
            time.sleep(1)
            if time.time() > giveup_time:
                try:
                    os.system("killall -9 basf2 python")
                    print("!!!!!! Gave up, basf2 all killed")
                except BaseException:
                    printf("Error to killall -9")
#                    break
            print("!!!One more time")

        subprocess.call(["removerb", inrblist[i]])
        subprocess.call(["removerb", outrblist[i]])

        print("Completed basf2 clean-up")

    exit()

# main


print("Starting output collector")

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
    subprocess.call("removerb " + x, shell='True')
for x in outrblist:
    subprocess.call("removerb " + x, shell='True')

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

# time.sleep(5)

procbasf2 = []
# run basf2
for i in range(0, ncol):
    basf2cmd = "basf2 --no-stats " + col_script + " " + inrblist[i] + " " + outrblist[i]
    print basf2cmd
    procbasf2.append(subprocess.Popen(basf2cmd, shell='True'))

# Sleep
while True:
    signal.pause()
