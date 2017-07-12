import glob
import subprocess

'''
Simple script submitting jobs to process all data.
Usage:
basf2 submitAll.py
'''

# Experimantl number
exp = 'exp01'
# Data directory
dir = '/ghi/fs01/belle2/bdata/group/detector/CDC/unpacked/'

files = glob.glob(dir + exp + '/gcr.cdc.*.root')

if not files:
    print("No data are found.")
    quit()

for f in files:
    input = f  # input file name.
    name = f.split('/')
    logfile = name[-1].replace('.root', '.log')  # input file name.
    output = 'output_' + name[-1]  # output file name.
    command = ' '.join(['basf2 runReconstruction.py', input, output, '>&', logfile])
#    print('bsub -q s \"' + command + ' \"')
#    subprocess.run('bsub -q s \"'+command+' \"')
    subprocess.run(["bsub", "-q", "l", "basf2", "runReconstruction.py", input, output])
