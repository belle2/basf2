import glob
import subprocess

'''
Simple script submitting jobs to process all data.
Usage:
basf2 submitAll.py
'''

# Experimantl number
exp = 'exp00'
# Data directory
dir = '/ghi/fs01/belle2/bdata/group/detector/CDC/unpacked/'

files = glob.glob(dir + exp + '/cr.cdc.*.root')

if not files:
    print("No data are found.")
    quit()

for f in files:
    input = f  # input file name.
    name = f.split('/')
    output = 'output_' + name[-1]  # output file name.
    #    print('basf2 runReconstruction.py '+'input='+input+' output='+output)
    subprocess.run(["bsub", "-q", "l", "basf2", "runReconstruction.py", input, output])
