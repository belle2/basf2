# !/usr/bin/env python
# -*- coding: utf-8 -*-

##
# contributors: G. Casarosa
##

######
# utils to overlay DATA to simulation
#
# in order to use this function:
# import it:
#     import svd.overlay_utils as svdou
# and then use it to:
#     1) prepare the input files
#     2) actually overlay the data
#
# examples are in svd/examples/overlayUtils.py
#
######

from basf2 import *
from ROOT import Belle2


def prepare_svd_overlay(path, inputFiles, outputFileTag="overlay"):
    """
    This function reads a list of inout files and prepare them to be used in overlay_svd_data
    @param inputFiles: list of input files to be processed
    @param outputFileTag: tag added just before the .root
    """

    for inputfile in inputFiles:

        splittext = inputfile.split(".root")
        outputfile = splittext[0]+"_"+str(outputFileTag)+".root"
        main = create_path()
        main.add_module("RootInput", inputFileNames=inputfile)
        main.add_module("RootOutput", branchNames="SVDShaperDigits", outputFileName=outputfile)
        print_path(main)
        process(main)


def overlay_svd_data(path, datatype="cosmics", overlayfiles=""):
    """
    This function overlay events from data to the standard simulation
    @param datatype: must be chosen among {xTalk, cosmics, user-defined}
    @param overlayfiles: if the datatype is user-defiled, the user can specify rootfiles to be overlaied to simulation
    """

    if not (str(datatype) == "xTalk" or str(datatype) == "cosmics" or str(datatype) == "user-defined"):
        print("ERROR in SVDOverlay: the specified datatype ("+str(datatype) +
              ") is not recognized, choose among: xTalk, cosmics or user-defined")
        return

    overlayDir = "/gpfs/fs02/belle2/group/detector/SVD/overlayFiles/"

    if str(datatype) == "xTalk":
        overlayfiles = str(overlayDir)+str(datatype)+"/physics*.root"
    if str(datatype) == "cosmics":
        overlayfiles = str(overlayDir)+str(datatype)+"/reco*_1.root"

    print(" ** SVD OVERLAY UTIL CALLED **")
    print(" -> overlaying the following files to simulation: ")
    print(str(overlayfiles))

    bkginput = register_module('BGOverlayInput')
    bkginput.set_name('BGOverlayInput_SVDOverlay')
    bkginput.param('bkgInfoName', 'BackgroundInfoSVDOverlay')
    bkginput.param('extensionName', "_SVDOverlay")
    bkginput.param('inputFileNames', overlayfiles)
    path.add_module(bkginput)

    bkgexecutor = register_module('BGOverlayExecutor')
    bkgexecutor.set_name('BGOverlayExecutor_SVDOverlay')
    bkgexecutor.param('bkgInfoName', 'BackgroundInfoSVDOverlay')
    path.add_module(bkgexecutor)

    path.add_module("SVDShaperDigitSorter")
