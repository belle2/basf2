#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
For SectorMaps running on data it significantly improves performance if the SectorMap is trained
on the Geometry as it is in data. This script reads alignment data from a GT and compares the aligned
Geometry with the not aligned Geomtry. The differences are written are written out in xml file
format which can be directly used to create new Geometry payloads for SVD and PXD.
Note you need to provide one or more global tags (or local DB) which contain the default geometry
and alignement payloads to be used. Be careful that the IOVs do need to match (e.g. by editing the DB
txt file for a local DB). See for example svd/scripts/dbImporters/create_SVDGeometryPar_payload.py
'''

import basf2
from ROOT import Belle2
import ROOT

import argparse


parser = argparse.ArgumentParser(description='Script that produces a xml file with  \
              sensor displacements to be used to create a DB payload. ')


parser.add_argument(
  '--localDB',
  type=str,
  default='',
  dest="localDB",
  help="The txt file defining  a local DB (optional)."
)

parser.add_argument(
  '--listOfGT',
  nargs="*",
  default=[],
  dest="listOfGT",
  help="List of global tag names separated by space (optional)."
)

parser.add_argument(
  '--expNum',
  type=int,
  default='0',
  dest="expNum",
  help="The experiment number needed to pick correct IOV. (optional)."
)

parser.add_argument(
  '--runNum',
  type=int,
  default='0',
  dest="runNum",
  help="The run number needed to pick correct IOV. (optional)"
)

args = parser.parse_args()


if not args.localDB == '':
    basf2.B2INFO('Prepending local DB: ' + args.localDB)
    basf2.conditions.prepend_testing_payloads(args.localDB)


for aGT in args.listOfGT:
    basf2.B2INFO('Prepending global tag: ' + aGT)
    basf2.conditions.prepend_globaltag(aGT)


class Reader(basf2.Module):
    """ Module to create misalignment (first reads real sensor positions, then applies misalignment and creates a xml file
      suited for creating a DB payload). The DB payload itself is created by using the Geometry module by setting the
      createPayloads to True.
    """

    def __init__(self):
        ''' Init '''

        #: dictionary to map the displacements for each sensor
        self.sensors = dict()

        super().__init__()

    def event(self):
        ''' event function '''
        maxDiff = 0.0

        for sensor in Belle2.VXD.GeoCache.getInstance().getListOfSensors():

            info = Belle2.VXD.GeoCache.getInstance().get(sensor)

            center = ROOT.TVector3(0., 0., 0.)

            centerN = info.pointToGlobal(center, False)
            centerA = info.pointToGlobal(center, True)

            centerDiff = centerA - centerN

            centerDiff = info.vectorToLocal(centerDiff, reco=False)

            # it is easy to run without alignment (e.g. specifying wrong IOV or tag) try to catch that by
            # checking that the displacementa are not 0
            if (maxDiff < centerDiff.R()):
                maxDiff = centerDiff.R()

            # for now do not use rotations for sensors
            phi = 0
            psi = 0
            theta = 0

            self.sensors[f"{sensor.getLayerNumber()}.{sensor.getLadderNumber()}.{sensor.getSensorNumber()}"] = [
                                                  centerDiff.X(), centerDiff.Y(), centerDiff.Z(), phi, theta, psi]

        if (maxDiff < 1e-9):
            basf2.B2ERROR(f"Max occured displacements are smaller then 1e-9! Are you sure alignments constants \
                          are loaded correctly? max={maxDiff} cm")


# reading displacements from the DB into a dictionary
reader = Reader()

path = basf2.create_path()
path.add_module("EventInfoSetter", expList=[args.expNum], runList=[args.runNum], evtNumList=[1])
path.add_module("Gearbox")
path.add_module("Geometry")
path.add_module(reader)
basf2.process(path)

sensors = reader.sensors


allladders = [8, 12, 7, 10, 12, 16]
allsensors = [2, 2, 2, 3, 4, 5]


def a(xml, txt):
    ''' Writes a single line into the xml file
      @param xml the opened xml file to be written to
      @param txt the line to be written
    '''
    xml.writelines([txt + "\n"])


def c(xml, name, alig=None):
    '''
      Creates displacement information in the correct format to be written to the xml file.
      @param the open xml file to be written to
      @param name The name of the sensor which information is written
      @param alig
    '''
    if alig is None:
        alig = [0., 0., 0., 0., 0., 0.]
    a(xml, "<Align component=\"" + name + "\">")
    a(xml, f"<du     unit=\"cm\" >{alig[0]}</du>     <dv    unit=\"cm\" > {alig[1]}</dv>   <dw    unit=\"cm\" >{alig[2]}</dw>")
    a(xml, f"<alpha  unit=\"rad\">{alig[3]}</alpha>  <beta unit=\"rad\">{alig[4]}</beta> <gamma unit=\"rad\">{alig[5]}</gamma>")
    a(xml, "</Align>")


# write the PXD displacement into xml format
with open("PXD-Alignment.xml", "w") as xml:

    # write header
    a(xml, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>")
    a(xml, "<Alignment>")

    # write global and halfshell displacements (none)
    c(xml, "PXD")
    c(xml, "PXD.Ying")
    c(xml, "PXD.Yang")

    # write displacement data for each sensor
    for layer in [1, 2]:
        c(xml, f"{layer}")
        for ladder in range(1, allladders[layer - 1] + 1):
            c(xml, f"{layer}.{ladder}")
            for sensor in range(1, allsensors[layer - 1] + 1):
                if f"{layer}.{ladder}.{sensor}" in sensors:
                    c(xml, f"{layer}.{ladder}.{sensor}", [v for v in sensors[f"{layer}.{ladder}.{sensor}"]])

    # write outro
    a(xml, "</Alignment>")


# write all data for the SVD
with open("SVD-Alignment.xml", "w") as xml:

    # write header
    a(xml, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>")
    a(xml, "<Alignment>")

    # write global and half shell displacement  (none)
    c(xml, "SVD")
    c(xml, "SVD.Pat")
    c(xml, "SVD.Mat")

    # write displacement for each sensor
    for layer in [3, 4, 5, 6]:
        c(xml, f"{layer}")
        for ladder in range(1, allladders[layer - 1] + 1):
            c(xml, f"{layer}.{ladder}")
            for sensor in range(1, allsensors[layer - 1] + 1):
                if f"{layer}.{ladder}.{sensor}" in sensors:
                    c(xml, f"{layer}.{ladder}.{sensor}", [v for v in sensors[f"{layer}.{ladder}.{sensor}"]])

    # write outro
    a(xml, "</Alignment>")
