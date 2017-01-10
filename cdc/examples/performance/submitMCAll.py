import subprocess

'''
Simple script submitting jobs to process all data.
Usage:
python3 submitAll.py
'''


for i in range(100):
    subprocess.run(["bsub", "-q", "l", "basf2", "runSimulation+Reconstruction.py",
                    str(i), '10000', '201609'])
