##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import os

# run as "python beast/scripts/phase2/goall_SAD.py" from the main directory

# == que list ==
# s <6h
# l <48h
# b_a <48h
# b_l <720h


phase = '2'  # geometry for phase2
outdir_base = '/group/belle2/BGcampaigns/g4sim/'  # output base directory
outdir_sub = 'phase2.1.4_collimators_1_upd'  # output sub directory

fileSAD = 'phase2.1.4_collimators_1_upd'  # sad filename from /group/belle2/BGcampaigns/SAD/forG4/
# check the directory for existing SAD samples
# 28.2.2019
# early_phase3_1_step5, early phase 3; use 31 for phase!
# phase2.1.7_Touschek_step5
# phase2.1.3_june_Touschek_EM_0
# phase2.1.3_june_Touschek_EM_1
# phase2.1.3_june_Touschek_EM_2
# phase2.1.4_collimators_1_upd ; should be same as phase2.1.3_june_Touschek_EM_0

outdir = outdir_base + outdir_sub

bkgs = ['Touschek LER', 'Coulomb LER', 'Brems LER', 'Touschek HER', 'Coulomb HER', 'Brems HER']  # bacground types

bkgs_flag = [1, 1, 1, 1, 1, 1]  # skip backgrounds with flag 0

time = [500, 500, 10000, 1000, 1000, 10000]  # simulation time in us (per job)

files = 400  # number of jobs to be commited for each background type

q = ['l', 'l', 's', 's', 's', 's']  # queue for jobs of given background

i = 0
skp = 0
for f in bkgs:
    if bkgs_flag[i] != 1:
        i += 1
        continue
    if not os.path.isdir(outdir):
        os.mkdir(outdir)
        print("Made directory " + outdir)
    if not os.path.isdir(outdir + '/output'):
        os.mkdir(outdir + '/output')
        print("Made directory " + outdir + '/output')
    if not os.path.isdir(outdir + '/log'):
        os.mkdir(outdir + '/log')
        print("Made directory " + outdir + '/log')
    for ifile in range(0, files):
        logname = f + '_' + str(ifile) + '.log'
        logname = "_".join(logname.split())
        com = 'bsub -q ' + q[i] + ' -o /dev/null \"basf2 beast/scripts/phase2/generateSADBg.py ' + f + ' ' + str(time[i]) + ' ' + \
            str(ifile) + ' study ' + phase + ' ' + fileSAD + ' ' + outdir + '/output/ >& ' + outdir + '/log/' + logname + '\"'
# test locally
#        com = 'basf2 beast/scripts/phase2/generateSADBg.py ' + f + ' ' + str(time[i]) + ' ' + str(ifile) + ' study ' + phase + \
#    ' ' + fileSAD + ' ' + outdir + '/output/'
        print(com)
        if os.path.isfile(outdir + '/log/' + logname) and skp == 0:
            answ = input("Output file already exists. Overwrite for all (y), skip existing (n): ")
            if answ == 'y':
                skp = 2
            elif answ == 'n':
                skp = 1
            else:
                continue
        if skp == 1:
            print("Skipped!")
            continue
        os.system(com)
    i += 1
