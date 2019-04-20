#!/usr/bin/env python
# -*- coding: utf-8 -*-

##################################################################
#
# DB Importer for the BeamSpot (position, position error, size)
# from the DQM plots of the IPDQMExpressReco module
#
# usage:
# > basf2 beamSpotImporter.py -- --exp [exp number] --run [run number] --dqm [DQM root file]
#
# the DQM root file must contain the IPMonitoring folder
#
# contributors (2019): G. De Marino, G. Casarosa
##################################################################

import basf2
from basf2 import *
import ROOT
from ROOT import TVector3, TMatrixDSym, TFile, TH1F
from ROOT import Belle2
from ROOT.Belle2 import BeamSpot
import argparse
from termcolor import colored
import sys


parser = argparse.ArgumentParser(description="BeamSpot Importer from DQM root file")
parser.add_argument('--exp', metavar='experiment', dest='exp', type=int, nargs=1, help='Experiment Number')
parser.add_argument('--run', metavar='run', dest='run', type=int, nargs=1, help='Run Number')
parser.add_argument('--dqm', metavar='dqm', dest='dqm', type=str, nargs=1, help='DQM ROOT file')

print('')

if(str(sys.argv[1]) == "help"):
    parser.print_help()
    exit(1)

args = parser.parse_args()

minVertices = 100

experiment = args.exp[0]
run = args.run[0]
dqmRoot = args.dqm[0]

GREEN = "\033[1;32m"
RESET = "\033[0;0m"
sys.stdout.write(GREEN)
print('|     experiment number = ' + str(experiment))
print('|first valid run number = ' + str(run))
print('|         DQM root file = ' + str(dqmRoot))
sys.stdout.write(RESET)
print('')


class beamSpotImporter(basf2.Module):

    def beginRun(self):

        iov = Belle2.IntervalOfValidity(experiment, run, -1, -1)

        dqmFile = TFile(str(dqmRoot), "read")

        if not dqmFile:
            print("file not found, check the arguments and retry.")
            return

        hVertexX = dqmFile.Get("IPMonitoring/Y4S_Vertex.X")

        entries = hVertexX.GetEntries()

        if entries < minVertices:
            print('')
            RED = "\033[1;31m"
            sys.stdout.write(RED)
            print(' xxx WARNING! not enough vertices in this run (< ' +
                  str(minVertices) + '), the beam spot position will NOT be updated.')
            sys.stdout.write(RESET)
            print('')

            return

        hVertexY = dqmFile.Get("IPMonitoring/Y4S_Vertex.Y")
        hVertexZ = dqmFile.Get("IPMonitoring/Y4S_Vertex.Z")

        vertexX = hVertexX.GetMean()
        vertexY = hVertexY.GetMean()
        vertexZ = hVertexZ.GetMean()

        # vertex position
        vertexPos = ROOT.TVector3(vertexX, vertexY, vertexZ)
        vertexPos.Print()

        # vertex position error
        hVarianceX = dqmFile.Get("IPMonitoring/Var.X")
        hVarianceY = dqmFile.Get("IPMonitoring/Var.Y")
        hVarianceZ = dqmFile.Get("IPMonitoring/Var.Z")
        hVarianceXY = dqmFile.Get("IPMonitoring/Covar.XY")
        hVarianceXZ = dqmFile.Get("IPMonitoring/Covar.XZ")
        hVarianceYZ = dqmFile.Get("IPMonitoring/Covar.YZ")
        vertexCov = ROOT.TMatrixDSym(3)
        vertexCov[0][0] = hVarianceX.GetMean()
        vertexCov[1][1] = hVarianceY.GetMean()
        vertexCov[2][2] = hVarianceZ.GetMean()
        vertexCov[0][1] = hVarianceXY.GetMean()
        vertexCov[0][2] = hVarianceXZ.GetMean()
        vertexCov[1][2] = hVarianceYZ.GetMean()
        vertexCov[1][0] = vertexCov[0][1]
        vertexCov[2][0] = vertexCov[0][2]
        vertexCov[2][1] = vertexCov[1][2]
        vertexCov.Print()

        # beam spot size
        vertexSize = ROOT.TMatrixDSym(3)
        vertexSize[0][0] = 0.1
        vertexSize[1][1] = 0.2
        vertexSize[2][2] = 0.3
        vertexSize.Print()

        # create and fill the payload
        payload = Belle2.BeamSpot()

        payload.setVertex(vertexPos, vertexCov)
        payload.setSize(vertexSize)

        Belle2.Database.Instance().storeData("BeamSpot", payload, iov)


use_local_database("localDB_BeamSpot/database.txt", "localDB_BeamSpot")

main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module(beamSpotImporter())

# Process events
process(main)
