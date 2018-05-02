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
period = 1  # data clock period is 32 (ns), but for the sake of convenience, let's use 1

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
                if not sig:
                    print('continuing')
                    continue
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


def unpack(meta, headers, triggers, atlas, writer, evtsize):
    """
    transform into VCD signals from clock-seperated data and hitmap
    """
    unpackwith = ', '.join(['bin:{}'.format(sig[4]) for sig in atlas])
    vcdVars = [writer.register_var(
        sig[5], sig[0] + '[{}:{}]'.format(sig[1], sig[2]), 'wire', size=sig[4]) if
        not isUnamed(sig) else None for sig in atlas]
    event = writer.register_var('m', 'event', 'event')
    eventNo = writer.register_var('m', 'eventNo', 'integer')
    subrun = writer.register_var('m', 'subrun', 'integer')
    run = writer.register_var('m', 'run', 'integer')
    delays = [writer.register_var('m', 'delay{}'.format(i), 'wire', size=9)
              for i in range(len(evtsize))]
    lastvalues = [None] * len(atlas)
    iteration = 0
    timestamp = 0
    power = period.bit_length() - 1  # (marginal and ugly) speed optimization
    print('converting to waveform ...')

    null = BitArray([0] * sum(evtsize))

    change = writer.change
    for trgInd, trg in enumerate(triggers):
        # skip empty (dummy) buffers
        # if trg[:16].hex == 'dddd':
        #     continue
        # print(trgInd)
        # printHex(trg[0])
        # printHex(trg)

        timestamp = iteration << power
        writer.change(event, timestamp, '1')
        # writer.change(eventNo, timestamp, trgInd)
        writer.change(eventNo, timestamp, meta[trgInd][0])
        writer.change(run, timestamp, meta[trgInd][1])
        writer.change(subrun, timestamp, meta[trgInd][2])
        for i, d in enumerate(delays):
            if len(headers[trgInd][i]) <= 0:
                continue
            writer.change(d, timestamp, headers[trgInd][i][64 + 11: 64 + 20].uint)
        clocks = list(trg.cut(sum(evtsize)))
        # recover clock shifts
        if len(clocks) > 47:
            clocks = clocks[1:48] + [clocks[0]]
        # clocks = clocks[2:48] + clocks[0:2]
        # for clock in trg.cut(sum(evtsize)):
        for clock in clocks:
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
                    change(vcdVars[i], timestamp, values[i])
            lastvalues = values

        # filling 16clk gaps
        # timestamp = iteration << power
        # iteration += 6
        # values = null.unpack(unpackwith)
        # for i, sig in enumerate(atlas):
        #     if isUnamed(sig):
        #         continue
        #     writer.change(vcdVars[i], timestamp, values[i])
    print('')


def writeVCD(meta, data, atlas, fname, evtsize, combine=False):
    """
    evt: 2D list of HSLB buffers
    atlas: list of iterates
    """
    # combine data in different buffers into a single block of event size
    samples = len(list(filter(None, data[0]))[0]) // list(filter(None, evtsize))[0]
    NullArray = BitArray([])
    # clocks = BitArray([])
    clocks = []
    headers = []
    for evt in data:
        clocks.append(BitArray([]).join(
            itertools.chain.from_iterable(zip(
                # skip b2l header (3 words)
                *[evt[buf][32 * 3:].cut(evtsize[buf])
                    if evtsize[buf] > 0 else [NullArray] * samples
                    # for buf in range(4)]))))
                    for buf in range(len(evtsize))]))))
        headers.append([evt[buf][:32 * 3] for buf in range(len(evtsize))])
    with open(fname, 'w') as fout:
        with VCDWriter(fout, timescale='1 ns', date='today') as writer:
            if combine:
                # combVCD(clocks, atlas, writer)
                raise Exception('Combine has not been updated. Use literal instead.')
            else:
                # literalVCD(clocks, atlas, writer)
                unpack(meta, headers, clocks, atlas, writer, evtsize)


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
    meta = pickle.load(rfp)
    rfp.close()

    if args.nocheck:
        dummycheck = False
    fname = args.output if args.output else pica[:pica.rfind('.')] + '.vcd'
    writeVCD(meta, data, atlas, fname, evtsize)
