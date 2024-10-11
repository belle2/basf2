##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Validation of KLM channel status calibration.
'''

# import math
import numpy
import ROOT
# from ROOT.Belle2 import KLMChannelStatus
from prompt import ValidationSettings
import argparse
import matplotlib.pyplot as plt

#: Tells the automated system some details of this script
settings = ValidationSettings(name='KLM channel status',
                              description=__doc__,
                              download_files=['stdout'],
                              expert_config={
                                  "chunk_size": 100
                              })

parser = argparse.ArgumentParser()
parser.add_argument('job_path')
parser.add_argument('input_data_path')
parser.add_argument('requested_iov')
parser.add_argument('expert_config')
args = parser.parse_args()
job_path = args.job_path
input_data_path = args.input_data_path
requested_iov = args.requested_iov
expert_config = args.expert_config

database_file = f'{job_path}/KLMChannelStatus/outputdb/database.txt'

rev = []
ini = []
fin = []

# if len(sys.argv) != 2:
#     print('Usage: basf2 generateValidationRoot.py dirName')
#     print('')
#     sys.exit(1)
# dirName = str(sys.argv[1])
# caliDir = dirName

iovfile = open(database_file, 'r')
for line in iovfile:
    lst = line.split()
    rev.append(lst[1])
    iov = lst[2].split(',')
    ini.append(int(iov[1]))
    fin.append(int(iov[3]))
iovfile.close()

rev_num = numpy.zeros(1, dtype=str)
run_num = numpy.zeros(1, dtype=int)

eklm_normal = numpy.zeros(1, dtype=int)
eklm_dead = numpy.zeros(1, dtype=int)
eklm_hot = numpy.zeros(1, dtype=int)

bklm_normal = numpy.zeros(1, dtype=int)
bklm_dead = numpy.zeros(1, dtype=int)
bklm_hot = numpy.zeros(1, dtype=int)

tree = ROOT.TTree('tree', '')
tree.Branch('rev_num', rev_num, 'rev_num/S')
tree.Branch('run_num', run_num, 'run_num/I')
tree.Branch('eklm_normal', eklm_normal, 'eklm_normal/I')
tree.Branch('eklm_dead', eklm_dead, 'eklm_dead/I')
tree.Branch('eklm_hot', eklm_hot, 'eklm_hot/I')
tree.Branch('bklm_normal', bklm_normal, 'bklm_normal/I')
tree.Branch('bklm_dead', bklm_dead, 'bklm_dead/I')
tree.Branch('bklm_hot', bklm_hot, 'bklm_hot/I')

# store all entries for plot
run_num_arry = []
eklm_dead_arry = []
eklm_hot_arry = []
bklm_dead_arry = []
bklm_hot_arry = []

for x in range(0, len(rev)):
    filename = f'{job_path}/KLMChannelStatus/outputdb/dbstore_KLMChannelStatus_rev_'+str(rev[x])+'.root'
    channel_status_file = ROOT.TFile(filename)
    channel_status_obj = channel_status_file.Get("KLMChannelStatus")
    eklm_normal[0] = 0
    eklm_dead[0] = 0
    eklm_hot[0] = 0
    bklm_normal[0] = 0
    bklm_dead[0] = 0
    bklm_hot[0] = 0
    for i in range(1, 65536):
        status = channel_status_obj.getChannelStatus(i)
        if (status == 1):
            if (i <= 15600):
                eklm_normal[0] += 1
            else:
                bklm_normal[0] += 1
        elif (status != 1 and status != 0):
            if (i <= 15600):
                if (status == 2):
                    eklm_dead[0] += 1
                elif (status == 3):
                    eklm_hot[0] += 1
            else:
                if (status == 2):
                    bklm_dead[0] += 1
                elif (status == 3):
                    bklm_hot[0] += 1
    channel_status_file.Close()
    rev_num[0] = rev[x]
    run_num[0] = ini[x]
    tree.Fill()
    myIni = int(ini[x])
    myFin = int(fin[x])
    '''
    #undesired behavior because this crowds the histogram too much
    while(myIni<myFin):
        myIni+=1
        run_num[0]=myIni
        rev_num[0]=rev[x]
        tree.Fill()
    '''
    run_num_arry.append(run_num[0])
    eklm_dead_arry.append(eklm_dead[0])
    eklm_hot_arry.append(eklm_hot[0])
    bklm_dead_arry.append(bklm_dead[0])
    bklm_hot_arry.append(bklm_hot[0])

outfile = ROOT.TFile('validation.root', 'recreate')
tree.Write()
outfile.Close()

fig, axs = plt.subplots(2, 2, sharex=True)
axs = axs.ravel()

axs[0].plot(run_num_arry, eklm_dead_arry, '.', color='C0')
axs[0].set_ylabel("EKLM dead channels")
axs[0].set_xlabel("Run numbers")

axs[1].plot(run_num_arry, eklm_hot_arry, '.', color='C1')
axs[1].set_ylabel("EKLM hot channels")
axs[1].set_xlabel("Run numbers")

axs[2].plot(run_num_arry, bklm_dead_arry, '.', color='C0')
axs[2].set_ylabel("BKLM dead channels")
axs[2].set_xlabel("Run numbers")

axs[3].plot(run_num_arry, bklm_hot_arry, '.', color='C1')
axs[3].set_ylabel("BKLM hot channels")
axs[3].set_xlabel("Run numbers")

drawLims = False
if drawLims:
    axs[0].axhline(300, ls='--', color='r')
    axs[1].axhline(8, ls='--', color='r')
    axs[2].axhline(300, ls='--', color='r')
    axs[3].axhline(8, ls='--', color='r')

fig.savefig('plots.pdf')
