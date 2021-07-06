#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Decay File Parser to check whether a decay file is correctly defined
"""

import sys
import settings
import os
import time
from colours import mesg, query, fail, done, warning
import descriptcheck

mesg('Starting the decfile check')
query('Opening decfile')
if len(sys.argv) < 2:
    fail(['Please pass the decfile as the first argument.'])
    sys.exit()

filename = sys.argv[1]
dkfilespath = os.path.dirname(os.path.abspath(sys.argv[0])) + '/' \
    + settings.dkfilespath

if not os.path.exists(filename):
    fail(['The file does not exist'])
    sys.exit()
elif not os.path.isfile(filename):

    fail(['The path does not point to a regular file.'])
    sys.exit()

file = open(filename)
if not file:
    fail(['Unknown error opening file.'])
    sys.exit()

done()

documentation_inprocess = False
date = ''
responsible = ''
eventtype = 0
descriptor = ''
nickname = ''
cuts = ''
fulleventcuts = ''
documentation = []
physicswg = ''
tested = ''
email = ''
extraopts = ''
particledefs = []
decay = {}
current_decay = []
order = []

decay_started = False
decay_inprocess = False

alias = {}
chargeconj = {}
cdecay = []
mother = ''

Ended = False
endcheck = False

linecount = 0


def getfield(line, string):
    tmp = ''
    if string + ':' in line:
        query('Now parsing: ' + string)
        tmp = line.partition(string + ':')[2]
        done()
        if tmp.startswith(' '):
            tmp = tmp.strip()
#            warning("Please leave a single space after the : sign. on line:"+str(linecount))
        if tmp == '':
            warning('Field empty on line ' + str(linecount))
    return tmp


for line in file:
    linecount += 1

    if line.strip() == '':
        continue

    if line.startswith('#') and not decay_started:

        if documentation_inprocess:
            if 'EndDocumentation' in line:
                documentation_inprocess = False
                documentation = ' '.join(documentation)
                continue
            documentation += [line.strip('#')]
            continue

        tmp = getfield(line, 'Documentation')
        if tmp:
            file2 = open(filename)
            for line2 in file2:
                if 'EndDocumentation' in line2:
                    documentation_inprocess = True
                    break
            file2.close()
            documentation += [tmp]

        tmp = getfield(line, 'EventType')
        if tmp:
            try:
                eventtype = int(tmp)
            except BaseException:
                fail(['Failed parsing eventtype on line ' + str(linecount),
                      'Not a number.'])
            order += ['EventType']
            mesg('Eventtype found: ' + str(eventtype))

        tmp = getfield(line, 'Descriptor')
        if tmp:
            descriptor = tmp
            order += ['descriptor']
            if '{,gamma}' in tmp:
                warning(
                    'Please do not include radiative photons in the descriptor.')

        tmp = getfield(line, 'NickName')
        if tmp:
            if tmp == '':
                fail(['NickName empty on line ' + str(linecount)])
            elif not tmp == os.path.basename(filename).partition('.dec')[0]:
                fail(['NickName not the same as filename!'])
            else:
                nickname = tmp
                order += ['nickname']

        tmp = getfield(line, 'Cuts')
        if tmp:
            cuts = tmp
            order += ['cuts']
            test_cuts = ['None', 'DaughtersInBelleII']
            if cuts not in test_cuts:
                warning(
                    'Unknown cuts <' +
                    cuts +
                    '> on line ' +
                    str(linecount) +
                    '. Please check.')

        tmp = getfield(line, 'FullEventCuts')
        if tmp:
            fulleventcuts = tmp
            order += ['fulleventcuts']

        tmp = getfield(line, 'ExtraOptions')
        if tmp:
            extraopts = tmp
            order += ['extraopts']

        tmp = getfield(line, 'PhysicsWG')
        if tmp:
            physicswg = tmp
            order += ['physicswg']

        tmp = getfield(line, 'Tested')
        if tmp:
            if tmp == 'No':
                warning('File not tested! Please test the file!')
            elif not tmp == 'Yes':
                warning('Unkown Tested state. Please use Yes or No.')
            else:
                tested = tmp
            order += ['tested']

        tmp = getfield(line, 'Responsible')
        if tmp:
            responsible = tmp
            order += ['responsible']

        tmp = getfield(line, 'Email')
        if tmp:
            if '@' not in tmp:
                warning('Please use a correct email format.')
            else:
                email = tmp
                order += ['email']

        tmp = getfield(line, 'ParticleValue')
        if tmp:
            tmp = tmp.split(',')
            tmp2 = {}
            for part in tmp:
                part = part.strip('"')
                part = part.split()
                tmp2[part[6]] = part[1]
            particledefs = tmp2

        tmp = getfield(line, 'Date')
        if tmp:
            try:
                date = int(tmp)
                if not date / 10000 == time.gmtime().tm_year:
                    warning('Date is not from this year. Please use YYYYMMDD for date field. YYYY parsed:' +
                            str(date / 10000) + ' vs. current year: ' + str(time.gmtime().tm_year))
                if date - 10000 * (date / 10000) > 1231:
                    warning('Cannot parse date. Please use YYYYMMDD for date field. MMDD parsed:' +
                            str(date - 10000 * (date / 10000)))
                if date - 100 * (date / 100) > 31:
                    warning('Cannot parse date. Please use YYYYMMDD for date field. DD parsed:' +
                            str(date - 100 * (date / 100)))
            except BaseException:
                warning('Cannot parse date. Please use YYYYMMDD for date field.'
                        )
    elif not line == '' and not decay_started and not Ended:

        mesg('End of header.')
        decay_started = True

    if Ended:
        endcheck = True

    if decay_started:
        if not line.startswith('#'):
            if 'Alias' in line:
                elements = line.partition('Alias')[2].strip().split()
                alias[elements[0].strip()] = elements[1].strip()
            if 'ChargeConj' in line:
                elements = line.partition('ChargeConj')[2].strip().split()
                chargeconj[elements[0].strip()] = elements[1].strip()

            if 'CDecay' in line:
                elements = line.partition('CDecay')[2].strip()
                cdecay += [elements]
            elif 'Decay' in line:

                mother = line.partition('Decay ')[2].strip()
                mesg('Found decay: ' + mother)
                decay_inprocess = True
                continue

            if 'End' in line and 'Enddecay' not in line:
                Ended = True
                decay_started = False

            if decay_inprocess:
                if 'Enddecay' in line:
                    decay[mother] = current_decay
                    decay_inprocess = False
                    # mesg("Found decay end of: "+mother)
                    current_decay = []
                else:

                    line = line.strip().split()
                    bf = line[0]
                    dec = []
                    try:
                        bf = float(bf)
                    except BaseException:
                        warning('Branching fraction not a number on line: ' +
                                str(linecount) + '. Skipping.')
                        continue
                    for daug in line[1:-1]:
                        if daug.strip(';') in settings.terminators:
                            # mesg("Terminator found, ending decay line.")
                            break
                        elif daug.endswith(';'):
                            warning(
                                'A new terminator found: ' +
                                daug.strip(';') +
                                '. Adding to list')
                            settings.terminators += [daug.strip(';')]
                            break
                        if daug in alias:
                            # daug = alias[daug]
                            pass
                        elif daug in chargeconj:
                            # daug = chargeconj[daug]
                            if daug in alias:
                                daug = alias[daug]
                            else:
                                warning(
                                    "You defined a charge conjugation without either particle being an alias. "
                                    "Are you sure you know what you're doing on line " + str(linecount) + '?')
                        else:
                            for k in alias:
                                if daug == alias[k]:
                                    warning(
                                        'You defined an alias to particle ' +
                                        daug +
                                        ' called ' +
                                        k +
                                        ' but on line ' +
                                        str(linecount) +
                                        ' you use the original particle. Is this what you want?')
                        dec += [daug]
                    if not line[-1].endswith(';'):
                        warning(
                            'Line ' +
                            str(linecount) +
                            ' does not end with a ;')
                    current_decay += [(bf, dec)]

mesg('File parsed successfully.')
file.close()

if not eventtype:
    warning("Cannot proceed without eventtype. Please fix the eventtype so it's recognisable."
            )
    sys.exit()

newevtype = ''
parttype = eventtype
query('Checking general flag')
general = parttype / 10000000
flag = 0
parttype -= general * 10000000
mother = ''
for daug in decay:
    if 'sig' in daug:
        mother = daug.partition('sig')[0]
        # mesg("Found mother: "+mother)
        break
else:
    if nickname.startswith('uubar'):
        flag = 3
    elif nickname.startswith('ddbar'):
        flag = 3
    elif nickname.startswith('ssbar'):
        flag = 3
    elif nickname.startswith('mixed'):
        flag = 1
    elif nickname.startswith('charged'):
        flag = 1
    elif nickname.startswith('charm'):
        flag = 2
    else:
        fail(['Cannot find the signal particle and cannot determine the inclusive mode.'])
        sys.exit()
    done()
if mother:
    if 'B' in mother or 'b0' in mother or 'Upsilon' in mother or 'chi_b' in mother:
        flag = 1
    elif 'D' in mother or 'psi' in mother or 'chi_c' in mother or 'c+' in mother:
        flag = 2
    elif 'K_S0' in mother or 'Lambda' in mother or 'Sigma' in mother or 'tau' in mother:
        flag = 3
    else:
        warning("Didn't recognise the mother particle. Check general flag manually.")
        flag = general
    if not flag == general:
        fail(['General flag not compliant. Should be ' + str(flag) + '.Please check.'])
    else:
        done()
newevtype += str(flag)

query('Checking selection flag')
selection = parttype / 1000000
parttype -= selection * 1000000
flag = selection
if not mother:
    flag = 0
elif mother == 'D-' or mother == 'B0' or mother == 'D+' or mother == 'anti-B0':
    flag = 1
elif mother == 'D0' or mother == 'anti-D0' or mother == 'B+' or mother == 'B-':
    flag = 2
elif mother == 'B_s0' or mother == 'D_s-' or mother == 'D_s+':
    flag = 3
elif mother == 'J/psi':
    flag = 4
elif mother == 'Lambda_b0' or mother == 'Lambda_c+':
    flag = 5
elif ('Sigma_b' in mother or 'chi_b' in mother or 'Omega_b' in mother) and general == 1:
    flag = 6
elif ('Upsilon' in mother or 'chi_b' in mother) and general == 1:
    flag = 8
elif ('D' in mother and '*' in mother or 'D_s1' in mother) and general == 2:
    flag = 7
elif ('psi(2S)' in mother or 'X_1(3872)' in mother or 'h_c' in mother or
      'chi_c' in mother or mother == 'eta_c') and general == 2:

    flag = 8
elif general == 3:

    if mother == 'tau+' or mother == 'tau-':
        flag = 1
    elif 'Lambda' in mother:
        flag = 3
    elif 'Sigma' in mother:
        flag = 2
    elif mother == 'K_S0':
        flag = 4
    else:
        warning(
            'General flag is 3 but mother particle is not recogniced - assuming minbias.')
        flag = 0
else:
    warning('Cannot determine selection flag. Please check manually.')
    flag = selection
if not flag == selection:
    warning(
        'Selection flag is not compliant, should be ' +
        str(flag) +
        '. Please check.')
else:
    done()
newevtype += str(flag)

query('Unfolding decay.')

current_decay = []
if mother:
    current_decay = (decay[mother + 'sig'])[:]

for (bf2, dec2) in current_decay:
    for daug in dec2:
        if daug in decay:
            newdecay = dec2[:]
            newdecay.remove(daug)
            for (bf2, dau2) in decay[daug]:
                norm = 0.0
                for (bf3, dau3) in decay[daug]:
                    norm += float(bf3)
                newbf = bf * bf2 / norm
                newdecay2 = sorted(newdecay[:] + dau2[:])
                current_decay += [(newbf, newdecay2)]
            break


def getmax(dec):
    maximumbf = 0
    toret = []
    for (bf, dec2) in dec:
        if bf > maximumbf:
            maximumbf = bf
            toret = dec2[:]
    return toret


main_decay = []
if mother:
    main_decay = [getmax(decay[mother + 'sig'])]

clean = False
while not clean and mother:
    clean = True
    olddec = (main_decay[-1])[:]
    for daug in main_decay[-1]:
        if daug in alias:
            olddec.remove(daug)
            if daug not in decay:
                for k in chargeconj:
                    if k == daug:
                        daug = chargeconj[k]
                    elif chargeconj[k] == daug:
                        daug = k
            if daug not in decay:
                warning('Aliased particle but cannot find its decay!: ' + daug)
                newdec = [alias[daug]]
            else:
                newdec = getmax(decay[daug])
            olddec += newdec
            clean = False
    if not clean:
        main_decay += [olddec]
done()

query('Checking the decay flag')

decayflag = parttype / 100000
parttype -= decayflag * 100000
neutrinos = False
nFinal = 0
nCommon = 0

final = []

for (bf, dec) in current_decay:
    common = 0
    for (bf2, dec2) in current_decay:
        test_dec1 = dec
        test_dec2 = dec2
        test_dec1.sort()
        test_dec2.sort()
        if test_dec2 == test_dec1:
            common += 1
    for daug in dec:
        if 'nu_' in daug:
            neutrinos = True
    if common >= 2:
        nCommon += 1
    for daug in dec:
        if daug in alias:
            break
    else:
        if not final:
            final = sorted(dec)
        else:
            dec.sort()
            if final == dec:
                continue
        nFinal += 1

flag = 0
if mother:
    flag += 1
if nFinal > 1:
    flag += 2
if nCommon > 0:
    flag += 1
if neutrinos:
    flag += 4

if not decayflag == flag:
    fail(['Decay flag is not compliant. Should be ' + str(flag) + '. Please check'])
else:
    done()
newevtype += str(flag)

query('Checking charm and lepton flag')
electron = False
muon = False
opencharm = False
closedcharm = False
doubleopen = False

charmflag = parttype / 10000
parttype -= charmflag * 10000

caughtopen = False
if not mother:
    for field in [extraopts, cuts, fulleventcuts]:
        if 'Electron' in field or 'electron' in field:
            electron = True
        if 'mu' in field or 'Mu' in field:
            muon = True
        if 'D0' in field or 'Dmu' in field or 'Ds' in field or 'DS' in field or 'DMass' in field or 'DMu' in field:
            opencharm = True
        if 'Jpsi' in field:
            closedcharm = True

for dec in main_decay:
    caughtopen = False
    for daug in dec:
        if daug in alias:
            daug = alias[daug]
        # if "D" in mother or "Lambda_c" in mother:
        #    opencharm=True
        if daug == 'e-' or daug == 'e+':
            electron = True
            continue
        if daug == 'mu-' or daug == 'mu+':
            muon = True
            continue
        if ('D' in daug or '_c' in daug) and 'chi_c' not in daug:
            if caughtopen:
                doubleopen = True
                caughtopen = False
                continue
            caughtopen = True
            opencharm = True
            continue
        if 'psi' in daug or 'chi_c' in daug:
            closedcharm = True
            continue
flag = 0
if opencharm:
    flag += 6
elif closedcharm:
    flag += 3
if electron:
    flag += 2
elif muon:
    flag += 1
if doubleopen:
    flag = 9

if not flag == charmflag:
    fail(['Charm flag is not compliant. Should be :' + str(flag) + '. Please check'])
else:
    done()
newevtype += str(flag)

query('Checking track flag.')
trackflag = parttype / 1000
parttype -= trackflag * 1000

maxbf = 0
maxtracks = 0
if not mother:
    warning('Inclusive decay: Problem with settings the track flag. Check manually.'
            )
    if 'DiLepton' in fulleventcuts or 'DiLepton' in cuts or 'DiLepton' in extraopts:
        maxtracks = 2
for dec in main_decay:
    for daug in dec:
        if daug in decay:
            break
    else:
        tracks = 0
        for daug in dec:
            if daug in settings.longlived:
                if tracks <= 9:
                    tracks += 1
        if tracks > maxtracks:
            maxtracks = tracks

if not trackflag == maxtracks:
    fail(['Track flag not compliant. Should be: ' +
          str(maxtracks) + '. Please check.'])
else:
    done()
newevtype += str(maxtracks)

query('Checking neutrals flag.')
neutrals = parttype / 100
parttype -= neutrals * 100

pi0eta = False
gamma = False
Kslambda = False
klong = False

for dec in main_decay:
    for daug in dec:
        if daug in alias:
            daug = alias[daug]
        if daug == 'K_S0' or daug == 'Lambda0' or mother == 'K_S0' or mother == 'Lambda0':
            Kslambda = True
        elif daug == 'pi0' or daug == 'eta':
            pi0eta = True
        elif daug == 'K_L0':
            Klong = True

previous_pi = False
for i in range(len(main_decay)):
    if 'gamma' in main_decay[i]:
        toBreak = False
        if i != 0:
            for daug in main_decay[i - 1]:
                daug2 = daug
                if daug in alias:
                    daug2 = alias[daug]
                if daug2 == 'pi0' or daug2 == 'eta':
                    if daug not in main_decay[i]:
                        toBreak = True
        if toBreak:
            continue
        gamma = True

flag = 0
if Kslambda:
    flag += 1
if klong:
    flag += 8
else:
    if gamma:
        flag += 2
    if pi0eta:
        flag += 4

if not flag == neutrals:
    fail(['Neutrals flag not compliant. Should be ' + str(flag) + '. Please check.'])
else:
    done()
newevtype += str(flag)

query('Checking the extra and user for duplicity .')

if settings.use_url:
    if not zippednos:
        warning('Cannot parse decfiles webpage')
        settings.use_url = False
    else:
        for (k, v) in zippednos:
            if filename.partition('=')[0] == v.partition(
                    '=')[0] and not eventtype / 10 == k / 10:
                warning(
                    'The decfile: ' +
                    v +
                    ':' +
                    str(k) +
                    ' should contain the same decay, therefore the first 7 '
                    'digits of the eventtype should match. Please check and use the same extra flag.')
                failed = True
            if k == eventtype:
                warning('Error: ' + v + ' has this eventtype already.')
                failed = True
            if k / 10 == eventtype / \
                    10 and not os.path.basename(filename).partition('=')[0] == v.partition('=')[0]:
                warning(
                    'The decfile: ' +
                    v +
                    ':' +
                    str(k) +
                    ' uses this extra flag, but the decay seems different. '
                    'Please check and use a unique extra flag.')
                failed = True

    if settings.obs_url:

        if not obsnos:
            warning('Cannot parse obsoletes trac file.')
            settings.use_url = False
        else:
            if str(eventtype) in obsnos:
                warning('The eventtype is obsolete on the line ' +
                        str(obsnos.index(str(eventtype)) + 1) + ' in: ' +
                        settings.obs_url)
                failed = True

if not settings.use_url:
    filelist = os.listdir(dkfilespath)
    newtype = 0
    failed = False
    for filen in filelist:
        if filen.endswith('.dec'):
            file = open(dkfilespath + '/' + filen)
            for line in file:
                if 'EventType: ' in line:
                    try:
                        newtype = int(line.partition('EventType: ')[2].strip())
                        break
                    except BaseException:
                        break
            if filen.partition('=')[0] == filename.partition('='):
                if not newtype / 10 == eventtype / 10:
                    warning(
                        'The decfile: ' +
                        filen +
                        ':' +
                        str(newtype) +
                        ' should contain the same decay, therefore the first 7 '
                        'digits of the eventtype should match. Please check and use the same extra flag.')
                    failed = True
#            if newtype == eventtype and not os.path.basename(filename) == v:
            if newtype == eventtype:
                warning('Error: ' + filen + ' has this eventtype already.')
                failed = True

    if settings.obsoletepath:
        obsfile = open(settings.obsoletepath + '/table_obsolete.sql')
        if obsfile:
            for line in obsfile:
                if int(line.partition('EVTTYPEID = ')[2].partition(
                        ', DESCRIPTION')[0]) == eventtype:
                    warning(
                        'The eventtype is obsolete on the following line in: ' +
                        settings.obsoletepath +
                        '/table_obsolete.sql')
                    mesg(line)
                    failed = True

extraflag = eventtype / 10
extraflag = eventtype - extraflag * 10
if 'DaughtersInBelleII' not in cuts and extraflag and cuts == 0:
    warning(
        'Your cuts are not empty, please set your userflag to greater or equal to 1.')
    failed = True
if cuts == ['None'] and extraflag != 0:
    warning("Your cuts are empty, your user flag should be 0 unless that's taken. Please check."
            )

if failed:
    fail([])
else:
    done()
mesg('Eventtype constructed: ' + newevtype + 'XX')

query('Checking nickname.')
faillist = []
failed = False
if not nickname == os.path.basename(filename).partition('.dec')[0]:
    faillist += ['Filename not the same as nickname! Fix.']
    failed = True
nick = nickname.partition('=')
if not nick[1] == '=' and not cuts == []:
    faillist += \
        ['The = sign not found in the nickname while cuts are present. Check nickname conventions and fix.'
         ]
    warning('The = sign not found in the nickname while cuts are present. Check nickname conventions and fix.'
            )
    # failed = True
if 'DaughtersInBelleII' in cuts and 'DecProdCut' not in nick[2].split(','):
    faillist += \
        ['You have decay angular acceptance cut in Cuts but not in the nickname.'
         ]
    failed = True

if failed:
    fail(faillist)
else:
    done()

if not len(nick[0].split(',')) > 1 and mother:
    warning('Found only one decay level in the nickname - highly unusual, you should probably use at least two.'
            )

if len(nick[2].split(',')) < len(cuts.split(',')):
    warning('You have more cuts than declared in the nickname. Please check.')

query('Checking the Physics WG.')
if physicswg not in settings.groups:
    fail(['The group /' + physicswg +
          '/ is not known. Please use one of the following:', settings.groups])
else:
    done()

if mother:
    query('Building descriptor.')
    max_bf = 0
    curdec = []
    for (bf, dec) in decay[mother + 'sig']:
        if bf > max_bf:
            max_bf = bf
    for (bf, dec) in decay[mother + 'sig']:
        if bf == max_bf:
            curdec = dec[:]
    descript = [mother] + ['->'] + curdec

    notclean = True
    while notclean:
        for daug in descript:
            notclean = False
            if daug in decay:
                notclean = True
                max_bf = 0
                for (bf, dec) in decay[daug]:
                    if bf > max_bf:
                        max_bf = bf
                for (bf, dec) in decay[daug]:
                    if bf == max_bf:
                        curdec = dec[:]
                ind = descript.index(daug)
                descript.remove(daug)
                if daug not in alias:
                    warning('You decay a particle: [' + daug +
                            '] without aliasing it first. Step aborted.')
                    notclean = False
                    break
                else:
                    daug = alias[daug]
                toadd = ['(', daug, '->'] + curdec + [')']
                toadd.reverse()
                for el in toadd:
                    descript.insert(ind, el)

#    Obsolete piece of code
#    for i in range(len(descript)):
#        if descript[i] in settings.descripslation:
#            descript[i] = settings.descripslation[descript[i]]

    descript = '[' + ' '.join(descript) + ']'
    descript = descript.replace('( ', '(')
    descript = descript.replace(' )', ')')
    for daug in cdecay:
        if 'sig' in daug:
            descript += 'cc'
            break

    done()

    query('Checking descriptor.')

    decr_old = descriptor.partition('[')[2].partition(']')[0].split()
    decr_new = descript.partition('[')[2].partition(']')[0].split()

    listA = descriptcheck.convertToList(descript)
    listB = descriptcheck.convertToList(descriptor)

    if descriptcheck.compareList(listB, listA):
        done()
    else:
        fail(['Descriptor not matched. Please check the old one:', descriptor,
              '\nAnd the one built by the parser: ', descript])
        for mes in descriptcheck.mesgdict:
            warning(mes)

    for daug in decay:
        print(daug, decay[daug])

    print('Main decay chain:')
    for dec in main_decay:
        print(dec)

sys.exit('Decfile check complete.')
