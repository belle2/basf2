#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import glob
import logging
from ROOT import TFile, TH1F


# checks if the run can be analyzed based on the HLT histograms, returns -1 if not and the number of events otherwise
def checkIfRunUsable(file):
    # consider only runs identified as physics runs (possible since experiment 8)
    h = file.FindObject('DQMInfo/rtype')
    if not h:
        logging.warning(fileName + ' ... histogram runtype not found')
        return -1
    if not h.GetTitle() == "physics":
        logging.info(fileName + ' ... not a physics run, skipping')
        return -1

    # check if we have enough events for meaningful masking
    h = file.FindObject('TOP/good_hits_per_event1')
    if not h:
        logging.warning(fileName + ' ... histogram good_hits_per_event1 not found')
        return -1
    nev = int(h.GetEntries())
    if nev < 10000:
        logging.warning(fileName + 'run =' + str(run) + 'events =' + str(nev) + ' ... skipped, not enough events')
        return -1

    # check if we have the necessary histogram to determine masking
    h = file.FindObject('TOP/good_hits')
    if not h:
        logging.warning(fileName + ' ... histogram good_hits not found')
        return -1
    if h.GetEntries() == 0:
        logging.warning(fileName + ' ... histogram good_hits has no entries ... skipped')
        return -1
    return nev


# creates a file with histograms for masked channels and returns the total number of masked channels in a run
def makeChannelMasks(file, outFileName):

    masks = [TH1F('slot_' + str(slot), 'Channel mask for slot ' + str(slot),
                  512, 0.0, 512.0) for slot in range(1, 17)]
    masked = 0

    # dead/hot channels: <10% or >1000% of the average in this slot
    for slot in range(1, 17):
        h = file.FindObject('TOP/good_channel_hits_' + str(slot))
        if not h:
            logging.error('no good_channel_hits found for slot'+str(slot))
            continue
        # calculate average number of hits per channel
        mean = 0
        n = 0
        for chan in range(h.GetNbinsX()):
            y = h.GetBinContent(chan+1)
            if y > 0:
                mean += y
                n += 1
        if n > 0:
            mean /= n
        # define threshold for dead and hot
        deadCut = mean / 10
        hotCut = mean * 10
        for chan in range(h.GetNbinsX()):
            y = h.GetBinContent(chan+1)
            if y <= deadCut:
                masks[slot-1].SetBinContent(chan+1, 1)  # 1 = dead
                masked += 1
            elif y > hotCut:
                masks[slot-1].SetBinContent(chan+1, 2)  # 2 = noisy
                masked += 1

    # asics with window corruption
    for slot in range(1, 17):
        h = file.FindObject('TOP/window_vs_asic_' + str(slot))
        if not h:
            logging.error('Error: no window_vs_asic found for slot' + str(slot))
            continue
        h0 = h.ProjectionX()
        h1 = h.ProjectionX('_tmp', 222, 245)
        for asic in range(h.GetNbinsX()):
            if h0.GetBinContent(asic+1) > 0:
                r = 1 - h1.GetBinContent(asic+1) / h0.GetBinContent(asic+1)
                if r > 0.20:
                    for chan in range(8):
                        masks[slot-1].SetBinContent(asic*8+chan+1, 2)  # mark as noisy
                        masked += 1

    # save to file
    if masked < 3000:  # don't save for nonsense
        outfile = TFile(outFileName, 'recreate')
        for h in masks:
            h.Write()
        outfile.Close()
    return masked


experiment = 10
experimentstring = "{:04d}".format(experiment)
outdir = 'masks'
if not os.path.exists(outdir):
    os.makedirs(outdir)
fileNames = sorted(glob.glob('/group/belle2/phase3/dqm/dqmsrv1/e'+experimentstring+'/dqmhisto/hltdqm*.root'))

logging.basicConfig(level=logging.INFO, filename="channelmasking.log")
logging.info("Starting channelmasking from HLT histograms")
logging.info("Experiment: "+str(experiment))

numFiles = len(fileNames)
if numFiles == 0:
    logging.error('No files found, exiting')
    sys.exit()

for fileName in fileNames:
    run = ((fileName.split('/')[-1]).split('r')[1]).split('.')[0]  # exp7-8
    outFileName = outdir + '/channelMask_e' + experimentstring + '_r' + run + '.root'

    # skip run if we analyzed this run already
    if os.path.exists(outFileName) or os.path.exists(outFileName.replace('masks/', 'masks/imported/')):
        logging.debug('Output file exists for run ='+str(run)+', skipping')
        continue

    # open file and check if it is a good physics run
    file = TFile(fileName)
    if not file:
        logging.error(fileName + ' ... cannot open')
        continue
    file.ReadAll()
    nev = checkIfRunUsable(file)
    if nev == -1:
        file.Close()
        continue

    masked = makeChannelMasks(file, outFileName)
    logging.info(fileName + 'run =' + str(run) + 'events =' + str(nev) + 'masked channels =' + str(masked))
    if masked > 3000:
        logging.critical("Result looks completely wrong, over 3000 masked channels. Inspect run "+str(run))
    file.Close()
