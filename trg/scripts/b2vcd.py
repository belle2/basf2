# python >= 2.7

# =============================== b2vcd ===============================
# Convert Belle2Link data into human-readable Value Change Dump file
# =====================================================================

# 2017 Belle II Collaboration
# author: Tzu-An Sheng
# tasheng@hep1.phys.ntu.edu.tw


from __future__ import print_function
import pickle
import re
from writer import VCDWriter
from bitstring import BitArray
import sys
from operator import itemgetter
import itertools
import ast
import operator as op

if sys.version_info[0] < 3:
    from itertools import izip as zip
evtsize = [2048, 2048, 0, 0]  # example B2l data width of each FINESSE (HSLB)
period = 32  # data clock period (ns)

# checking whether all dummies don't contain any data. Turn 'False' to speed up
dummycheck = True


def printBin(evt, wordwidth=8, linewidth=8, paraheight=4):
    words = [evt[word:word + wordwidth].bin for word in range(0, len(evt), wordwidth)]
    lines = ([' '.join(words[n:n + linewidth]) for n in range(0, len(words), linewidth)])
    paras = (['\n'.join(lines[n:n + paraheight]) for n in range(0, len(lines), paraheight)])
    print('\n\n'.join(paras))


def printHex(evt, wordwidth=32, linewidth=8, paraheight=4):
    words = [evt[word:word + wordwidth].hex for word in range(0, len(evt), wordwidth)]
    lines = ([' '.join(words[n:n + linewidth]) for n in range(0, len(words), linewidth)])
    paras = (['\n'.join(lines[n:n + paraheight]) for n in range(0, len(lines), paraheight)])
    print('\n\n'.join(paras))


# example signal assignments
signalstsf2 = """
dddd(15 downto 0) & cntr125M(15 downto 0) &
hitMapTsf(191 downto 0) & valid_tracker(0 downto 0) &
unamed(23 downto 7) &
tracker_out[0](428 downto 210) & ccSelf(8 downto 0) &
unamed (77 downto 36) &
mergers[5](255 downto 236) & mergers[5](235 downto 0) &
mergers[4](255 downto 236) & mergers[4](235 downto 0) &
mergers[3](255 downto 236) & mergers[3](235 downto 0) &
mergers[2](255 downto 236) & mergers[2](235 downto 0) &
mergers[1](255 downto 236) & mergers[1](235 downto 0) &
mergers[0](255 downto 236) & mergers[0](235 downto 0)
"""

signalsnk = """
ddd(15 downto 0) & cntr125M2D(15 downto 0) &
"000" & TSF0_input(218 downto 210) &
"000" & TSF2_input(218 downto 210) &
"000" & TSF4_input(218 downto 210) &
"000" & TSF6_input(218 downto 210) &
"000" & TSF8_input(218 downto 210) &
"0000" &
""" + \
    ''.join(["""TSF{sl:d}_input({high:d} downto {low:d}) &
TSF{sl:d}_input({high2:d} downto {low2:d}) &
""".format(sl=sl, high=h, low=h - 7, high2=h - 8, low2=h - 20) for sl in range(0, 9, 2) for h in range(209, 0, -21)]) + \
    """unamed(901 downto 0)
"""

signalsall = signalsnk + signalstsf2


# supported operators
operators = {ast.Add: op.add, ast.Sub: op.sub, ast.Mult: op.mul,
             ast.Div: op.truediv, ast.Pow: op.pow, ast.USub: op.neg}


def eval_expr(expr):
    return eval_(ast.parse(expr, mode='eval').body)


def eval_(node):
    if isinstance(node, ast.Num):  # <number>
        return node.n
    elif isinstance(node, ast.BinOp):  # <left> <operator> <right>
        return operators[type(node.op)](eval_(node.left), eval_(node.right))
    elif isinstance(node, ast.UnaryOp):  # <operator> <operand> e.g., -1
        return operators[type(node.op)](eval_(node.operand))
    else:
        raise TypeError(node)


def makeAtlas(signals, evtsize):
    """
    Make bitmap from VHDL signal assignments.
    input: string containing b2l signal designations.
    output: list of lists
        [ [ name, start, stop, pos, size, module ], ... ]
    """
    atlas = []
    pos = 0
    finesses = re.findall(r'(.+?)<=(.+?);', signals, re.DOTALL)
    if len(finesses) != sum(1 if width > 0 else 0 for width in evtsize):
        raise Exception('Number of valid signal assignments does not match HSLB data dimension: ' + str(evtsize))
    for finesse in finesses:
        for item in re.split(r'[\s\n]?&[\s\n]*', finesse[1].strip()):
            item = re.sub(r'--.+', '', item)
            dummy = re.match(r'"([01]+)"', item)
            if not dummy:
                sig = re.match(r'(.+)\s*\((.+) downto (.+)\)', item.strip())
                print('signal: ' + item.strip())
                start = eval_expr(sig.group(2))
                stop = eval_expr(sig.group(3))
                size = start - stop + 1
                if size < 1:
                    raise ValueError('Vector size cannot be 0 or negative.')
                name = sig.group(1)
            else:
                size = len(dummy.group(1))
                name = 'unamed'
                start, stop = size - 1, 0
            atlas.append([name, start, stop, pos, size, finesse[0]])
            pos += size
    if pos != sum(evtsize):
        raise ValueError(
            'Size of atlas:{} does not match event size:{}'.format(pos, evtsize))
    return atlas


def isUnamed(signal):
    return signal[0] == "unamed"


def unpack(triggers, atlas, writer):
    """
    transform into VCD signals from clock-seperated data and hitmap
    """
    unpackwith = ', '.join(['bin:{}'.format(sig[4]) for sig in atlas])
    vcdVars = [writer.register_var(
        sig[5], sig[0] + '[{}:{}]'.format(sig[1], sig[2]), 'wire', size=sig[4]) if
        not isUnamed(sig) else None for sig in atlas]
    event = writer.register_var('m', 'event', 'event')
    lastvalues = [None] * len(atlas)
    iteration = 0
    timestamp = 0
    power = period.bit_length() - 1  # (marginal and ugly) speed optimization
    print('converting to waveform ...')
    for trg in triggers:
        timestamp = iteration << power
        writer.change(event, timestamp, '1')
        for clock in trg.cut(sum(evtsize)):
            if timestamp & 1023 == 0:
                print('\r{} us converted'.format(str(timestamp)[:-3]), end="")
                sys.stdout.flush()
            timestamp = iteration << power
            iteration += 1
            values = clock.unpack(unpackwith)
            for i, sig in enumerate(atlas):
                if isUnamed(sig):
                    if dummycheck and '1' in values[i] and '0' in values[i]:
                        print('[Warning] non-uniform dummy value detected at {}ns: {}'.format(
                            timestamp, sig[3] % sum(evtsize)))
                        print(values[i])
                    continue
                if values[i] != lastvalues[i]:
                    writer.change(vcdVars[i], timestamp, values[i])
            lastvalues = values
    print('')


def literalVCD(triggers, atlas, writer):
    """
    This is slower than unpack(). Use that instead.
    write a VCD file from clock-seperated data and hitmap
    """
    vcdVars = [writer.register_var(
        'm', sig[0] + '[{}:{}]'.format(sig[1], sig[2]), 'wire', size=sig[4]) if
        sig[0] != 'unamed' else 0 for sig in atlas]
    event = writer.register_var('m', 'event', 'event')
    lastvalue = None
    iteration = 0
    timestamp = 0
    power = period.bit_length() - 1  # (marginal) speed optimization
    for trg in triggers:
        timestamp = iteration << power
        writer.change(event, timestamp, '1')
        for value in trg.cut(sum(evtsize)):
            if timestamp & 1023 == 0:
                print('\r{} us completed'.format(str(timestamp)[:-3]),)
                sys.stdout.flush()
            timestamp = iteration << power
            iteration += 1
            for i, sig in enumerate(atlas):
                sigvalue = value[sig[3]:sig[3] + sig[4]]
                lastsigvalue = lastvalue[sig[3]:sig[3] + sig[4]] if lastvalue else None
                if sig[0] == 'unamed':
                    if dummycheck and sigvalue.any(1) and sigvalue.any(0):
                        with sys.stderr as fout:
                            fout.write(
                                '[Warning] non-uniform dummy value detected at {}ns: {}'.format(
                                    timestamp, sig[3] % sum(evtsize)))
                            fout.write(values[i])
                    continue
                if not lastvalue or sigvalue != lastsigvalue:
                    writer.change(vcdVars[i], timestamp, sigvalue.bin)
            lastvalue = value


def combVCD(clocks, atlas, writer):
    """
    obsolete
    """
    comAtlas = []
    for i in range(len(atlas)):
        if atlas[i][0] == 'unamed' or atlas[i][0] in [x[0] for x in comAtlas]:
            continue
        signal = [atlas[i]]
        for j in range(i + 1, len(atlas)):
            if atlas[j][0] == atlas[i][0]:
                signal.append(atlas[j])
        signal = sorted(signal, key=itemgetter(1), reverse=True)
        for l in signal[1:]:
            signal[0].extend(l)
        comAtlas.append(signal[0])
    vars = [writer.register_var(
        'm', sig[0] + '[{}:{}]'.format(sig[1], sig[-3]), 'wire',
        size=sum(sig[4::5])) for sig in comAtlas]
    for timestamp, value in enumerate(clocks):
        for i, sig in enumerate(comAtlas):
            writer.change(vars[i], 32 * timestamp,
                          BitArray([]).join([value[sig[n]:sig[n] + sig[n + 1]] for n in range(3, len(sig), 5)]).bin)


def writeVCD(data, atlas, fname, size, combine=False):
    """
    evt: 2D list of HSLB buffers
    atlas: list of iterates
    """
    # combine data in different buffers into a single block of event size
    samples = len(list(filter(None, data[0][0]))[0]) // list(filter(None, evtsize))[0]
    NullArray = BitArray([])
    # clocks = BitArray([])
    clocks = []
    for evt in data:
        for entry in evt:
            clocks.append(BitArray([]).join(
                itertools.chain.from_iterable(zip(
                    *[entry[buf].cut(size[buf])
                      if size[buf] > 0 else [NullArray] * samples
                      for buf in range(4)]))))
    with open(fname, 'w') as fout:
        with VCDWriter(fout, timescale='1 ns', date='today') as writer:
            if combine:
                # combVCD(clocks, atlas, writer)
                raise Exception('Combine has not been updated. Use literal instead.')
            else:
                # literalVCD(clocks, atlas, writer)
                unpack(clocks, atlas, writer)


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--pickle", help="input pickle file")
    parser.add_argument("-s", "--signal", help="input signal file")
    parser.add_argument("-o", "--output", help="output VCD file")
    parser.add_argument("-nc", "--nocheck", help="disable dummy variable check",
                        action="store_true")
    args = parser.parse_args()
    if args.signal:
        with open(args.signal) as fin:
            evtsize = [int(width) for width in fin.readline().split()]
            print('interpreting B2L data with dimension ' + str(evtsize))
            atlas = makeAtlas(fin.read(), evtsize)
    else:
        atlas = makeAtlas(signalsall, evtsize)

    pica = args.pickle
    rfp = open(pica, 'rb')
    data = pickle.load(rfp)
    rfp.close()

    if args.nocheck:
        dummycheck = False
    fname = args.output if args.output else pica[:pica.rfind('.')] + '.vcd'
    writeVCD(data, atlas, fname, evtsize)
