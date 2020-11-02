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
# IMPORTANT: default paths are for KEKCC only!!
#
# examples are in svd/examples/runOverlay.py
#
######

import basf2 as b2


def prepare_svd_overlay(path, inputFiles, outputFileTag="overlay"):
    """
    This function reads a list of input files and prepare them to be used in overlay_svd_data
    @param inputFiles: list of input files to be processed
    @param outputFileTag: tag added just before the .root
    """

    for inputfile in inputFiles:

        b2.conditions.reset()
        b2.conditions.override_globaltags()
        b2.conditions.globaltags = ['svd_basic', "online"]
        splittext = ""
        if(str(outputFileTag) == "ZS3"):
            splittext = inputfile.split(".root")
        else:
            splittext = inputfile.split("_ZS3.root")
        outputfile = splittext[0]+"_"+str(outputFileTag)+".root"
        main = b2.create_path()
        main.add_module("RootInput", inputFileNames=inputfile)
        if(str(outputFileTag) == "ZS3"):
            main.add_module("SVDUnpacker", svdShaperDigitListName="SVDShaperDigitsZS3")

        zs = 3
        if (str(outputFileTag) == "overlayZS5"):
            zs = 5

        if not (str(outputFileTag) == "ZS3"):
            main.add_module("SVDZeroSuppressionEmulator",
                            SNthreshold=zs,
                            ShaperDigits='SVDShaperDigitsZS3',
                            ShaperDigitsIN='SVDShaperDigits',
                            FADCmode=True)
            main.add_module("RootOutput", branchNames=["SVDEventInfo", "SVDShaperDigitsZS3"], outputFileName=outputfile)
        else:
            main.add_module("RootOutput", branchNames=["SVDShaperDigits"], outputFileName=outputfile)
        b2.print_path(main)
        b2.process(main)


def overlay_svd_data(path, datatype="randomTrigger", overlayfiles=""):
    """
    This function overlay events from data to the standard simulation
    @param datatype: must be chosen among {xTalk, cosmics,randomTrigger, randomTriggerZS5,  user-defined}
    @param overlayfiles: if the datatype is user-defiled, the user can specify rootfiles to be overlaied to simulation
    """

    if not (str(datatype) == "xTalk" or str(datatype) == "cosmics" or str(datatype) ==
            "randomTrigger" or str(datatype) == "randomTriggerZS5" or str(datatype) == "user-defined"):
        print("ERROR in SVDOverlay: the specified datatype ("+str(datatype) +
              ") is not recognized, choose among: xTalk, cosmics or user-defined")
        return

    overlayDir = "/gpfs/fs02/belle2/group/detector/SVD/overlayFiles/"

    if str(datatype) == "xTalk" or str(datatype) == "cosmics" or str(datatype) == "randomTrigger":
        overlayfiles = str(overlayDir)+str(datatype)+"/*_overlay.root"

    if str(datatype) == "randomTriggerZS5":
        overlayfiles = str(overlayDir)+"randomTrigger/*_overlayZS5.root"

    print(" ** SVD OVERLAY UTIL CALLED **")
    print(" -> overlaying the following files to simulation: ")
    print(str(overlayfiles))

    bkginput = b2.register_module('BGOverlayInput')
    bkginput.set_name('BGOverlayInput_SVDOverlay')
    bkginput.param('bkgInfoName', 'BackgroundInfoSVDOverlay')
    bkginput.param('extensionName', "_SVDOverlay")
    bkginput.param('inputFileNames', overlayfiles)
    path.add_module(bkginput)

    bkgexecutor = b2.register_module('BGOverlayExecutor')
    bkgexecutor.set_name('BGOverlayExecutor_SVDOverlay')
    bkgexecutor.param('bkgInfoName', 'BackgroundInfoSVDOverlay')
    path.add_module(bkgexecutor)

    path.add_module("SVDShaperDigitSorter")
