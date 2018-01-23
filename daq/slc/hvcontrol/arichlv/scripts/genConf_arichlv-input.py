#!/usr/bin/python
# generator script for arichlv-input.conf

crateid = 0

for crateid in range(0, 2):
    channelid = 0
    cratenames = ['arich-mpod2.b2nsm.kek.jp', 'arich-mpod3.b2nsm.kek.jp']
    print("crate[{}].name : {}".format(crateid, cratenames[crateid]))
    print("crate[{}].port : {}".format(crateid, 25))
    for slotid in range(0, 6):
        for ch in range(0, 8):
            vset = [1.5, 3.8, 2, -2, 1.5, 3.8, 2, -2]
            vlimit = [2.5, 5, 2.5, -2.5, 2.5, 5, 2.5, -2.5]
            prefix = 'crate[{}].channel[{}].'.format(crateid, channelid)
            channelid = channelid + 1
            print('{}crate    : {}'.format(prefix, crateid))
            print('{}slot     : {}'.format(prefix, slotid))
            print('{}channel  : {}'.format(prefix, ch))
            print('{}rampuup  : 1.0'.format(prefix))
            print('{}rampdown : 0.0'.format(prefix))
            print('{}vlimit   : {}'.format(prefix, vlimit[ch]))
            print('{}climit   : {}'.format(prefix, 1))
            print('{}vdemand  : {}'.format(prefix, vset[ch]))
            print('{}turnon   : true'.format(prefix))
            print('')
