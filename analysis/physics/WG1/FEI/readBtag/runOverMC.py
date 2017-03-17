#!/usr/bin/env python3
# -*- coding: utf-8 -*-


import glob
import os
import sys

from makeChunks import chunks

if len(sys.argv) != 7:
    sys.exit(
        'Not enough arguments: '
        '[location of MC files] [output location] [BASF2 script name to be executed] [project name]'
        ' [number of files per job] [max number of jobs]')

mcLocation = sys.argv[1]
outputLocation = sys.argv[2]
basf2ScriptName = sys.argv[3]
projectName = sys.argv[4]
filesPerJob = int(sys.argv[5])
maxNumberOfJobs = int(sys.argv[6])

# create a log directory
logDirectory = outputLocation + '/log/'
if not os.path.exists(logDirectory):
    os.makedirs(logDirectory)

# create a root directory
rootDirectory = outputLocation + '/root/'
if not os.path.exists(rootDirectory):
    os.makedirs(rootDirectory)

mcFiles = glob.glob(mcLocation + '/*.root')

fileChunks = chunks(mcFiles, filesPerJob)

i = 0
for fileChunk in fileChunks:
    print(i)

    if maxNumberOfJobs > 0:
        if i >= maxNumberOfJobs:
            break

    logFile = logDirectory + projectName + '-' + str(i) + '.log'
    outputRootFileName = rootDirectory + projectName + '-' + str(i) + '.root'
    # print(
    os.system(
        'bsub -q l -o ' +
        logFile +
        ' basf2 ' +
        basf2ScriptName +
        ' ' +
        outputRootFileName +
        ' ' +
        str(i) +
        ' ' +
        str(filesPerJob) +
        ' ' +
        mcLocation +
        ' -l ERROR')
    i = i + 1
