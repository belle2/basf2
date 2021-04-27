#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np
import matplotlib.pyplot as plt


def fFileExist(filename):
    """Test if file exists"""

    try:
        oFile = open(filename, "r")
    except IOError:
        return 0
    else:
        oFile.close()
        return 1


def read(optlevel, b, name):
    """read time from out/*.out and save plots to plots/. The cut deletes obviously wrong times."""

    z = 0
    for i in range(0, len(optlevel)):
        if fFileExist("out/" + optlevel[i - z] + ".out") == 0:
            del optlevel[i - z]
            z = z + 1
    cut = True
    value = [0] * len(optlevel)
    sigma = [0] * len(optlevel)
    valuenormed = [0] * len(optlevel)
    sigmanormed = [0] * len(optlevel)
    n = [0] * len(optlevel)
    t = list(range(0, len(optlevel)))
    for i in t:
        fobj = open("out/" + optlevel[i] + ".out", "r")
        readvalue = []
        # read file
        for line in fobj:
            words = line.split()
            readvalue.append(1 / float(words[b]))
        fobj.close()
        # Calculation
        value[i] = np.mean(readvalue)
        sigma[i] = np.std(readvalue)
        u = list(range(0, len(readvalue)))
        z = 0
        if cut:
            for j in u:
                if readvalue[j - z] > 1.5 * value[i] or readvalue[j - z] < 0.5 * value[i]:
                    del readvalue[j - z]
                    z = z + 1
            sigma[i] = np.std(readvalue)
            value[i] = np.mean(readvalue)
        n[i] = len(readvalue)
    x = 1 / value[0]
    for i in range(0, len(value)):
        valuenormed[i] = value[i] * x
        sigmanormed[i] = sigma[i] * x
    plt.errorbar(
        t,
        valuenormed,
        xerr=0,
        yerr=sigmanormed,
        color="black",
        fmt="_",
        ecolor="black",
        label="normed time",
    )
    (locs, labels) = plt.xticks(t, optlevel)
    plt.setp(labels, rotation=90)
    plt.xlim([-0.5, len(optlevel) - 0.5])
    plt.ylabel("performance")
    fig = plt.gcf()
    fig.subplots_adjust(bottom=0.65)
    plt.savefig("plots/" + name + ".png")
    plt.close()
    fobj = open("plots/" + name + ".out", "w")
    for i in t:
        fobj.write(
            optlevel[i]
            + "&"
            + str(n[i])
            + "&"
            + str("%.3f" % value[i])
            + " & "
            + str("%.3f" % sigma[i])
            + "&"
            + str("%.4f" % valuenormed[i])
            + "&"
            + str("%.4f" % sigmanormed[i])
            + "\\\\\n"
        )
        fobj.write(r"\hline\n")


name = "CDCLegendreTracking"
optlevel = ["gcc-O0", "gcc-O3", "gcc-O3-native"]
read(optlevel, 0, name)
