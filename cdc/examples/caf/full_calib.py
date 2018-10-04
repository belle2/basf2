import os
import sys
import subprocess
import time
import glob
import shutil
# import ndf_and_pval

col = 'submit_collector.py'
algo = {'t0': ['t0_cal_algo.py', 'histT0.root'],
        'xt': ['xt_cal_algo.py', 'histXT.root'],
        'sigma': ['space_reso_cal_algo.py', 'histSigma.root'],
        'tw': ['time_walk_cal_algo.py', 'histTW.root']
        }

if os.path.exists('calib_result') is False:
    os.mkdir('calib_result')

seq = ['t0', 't0', 't0', 't0', 't0', 't0', 't0', 't0',
       'xt', 'sigma', 'xt', 'sigma',
       'tw', 't0']

nSeq = len(seq)
# print(nSeq)
# sys.exit()

# t0
for i, s in enumerate(seq):
    command = ' '.join(['basf2', col, '0.00001']) if i < 2 else ' '.join(['basf2 ', col, '0.001'])
    print(command)
    subprocess.run(command, shell=True)

    dlist = glob.glob('rootfile/*/centraldb')
    # Delete centraldb cache
    for d in dlist:
        #        print(d)
        shutil.rmtree(d)
    command_alg = ' '.join(['basf2', algo[s][0]])
    print(command_alg)
    subprocess.run(command_alg, shell=True)
    cdir = '/'.join(['calib_result', str(i)])
    os.mkdir(cdir)
    hist = algo[s][1]
    if os.path.exists('rootfile') is True:
        shutil.move('rootfile', cdir)
    if os.path.exists(hist) is True:
        shutil.move(hist, cdir)


# final reconstruction after all calibration
command = ' '.join(['basf2 ', col, '0.001'])
print(command)
subprocess.run(command, shell=True)
cdir = '/'.join(['calib_result', str(nSeq)])
os.mkdir(cdir)
if os.path.exists('rootfile') is True:
    shutil.move('rootfile', cdir)

# ndf_and_pval.plot()
