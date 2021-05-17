#!/usr/bin/env python3


def fFileExist(filename):
    '''Test if file exists'''

    try:
        oFile = open(filename)
    except OSError:
        return 0
    else:
        oFile.close()
        return 1


def read(optlevel):
    '''read time from output files'''

    filename = 'output/output-' + optlevel + '.dat'
    if fFileExist(filename) == 0:
        print('missing ' + filename)
    else:
        CDCLegendreTracking = []
        fobj = open(filename)
        # read output file
        for line in fobj:
            if line.startswith('CDCLegendreTracking'):
                words = line.split()
                CDCLegendreTracking.append(float(words[6]))
        fobj.close()
        print(optlevel + ' read')
        # write CDCLegendreTracking times
        fobj = open('out/' + optlevel + '.out', 'w')
        for i in range(0, len(CDCLegendreTracking)):
            fobj.write(str(CDCLegendreTracking[i]) + '\n')
            i = i + 1
        fobj.close()


optlevel = ['gcc-O0', 'gcc-O3', 'gcc-O3-native']
for i in optlevel:
    read(i)
