#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from ROOT import Belle2

import numpy as np

numberOfLayer = 6
numberOfLadders = [8, 12, 7, 10, 12, 16]
numberOfSensors = [2, 2, 2, 3, 4, 5]
numberOfParameters = 6
payload = Belle2.VXDAlignment()

"""
-------------------------
Misalignment of for Phase II
-------------------------
"""

# 0.01 -> 100 um

for i in range(1, 7):
    for j in range(1, numberOfLadders[i - 1] + 1):
        for h in range(1, numberOfParameters + 1):
            payload.setGlobalParam(0.0, Belle2.VxdID(i, j, 0).getID(), h)
            if j == 1 and h < 4:
                number = np.random.normal(0.0, 0.01, 1)
                payload.setGlobalParam(number, Belle2.VxdID(i, j, 0).getID(), h)

        for k in range(1, numberOfSensors[i - 1] + 1):
            for h in range(1, numberOfParameters + 1):
                payload.setGlobalParam(0.0, Belle2.VxdID(i, j, k).getID(), h)
                if h < 4:
                    number = np.random.normal(0.0, 0.01, 1)
                    payload.setGlobalParam(number, Belle2.VxdID(i, j, k).getID(), h)

# 1 -> x; 2 -> y; 3 -> z

# misalignment of Yang
payload.setGlobalParam(np.random.normal(0.0, 0.01, 1), Belle2.VxdID(1, 0, 0, 1).getID(), 1)
payload.setGlobalParam(np.random.normal(0.0, 0.01, 1), Belle2.VxdID(1, 0, 0, 1).getID(), 2)
payload.setGlobalParam(np.random.normal(0.0, 0.01, 1), Belle2.VxdID(1, 0, 0, 1).getID(), 3)

# misalignment of Ying
payload.setGlobalParam(np.random.normal(0.0, 0.01, 1), Belle2.VxdID(1, 0, 0, 2).getID(), 1)
payload.setGlobalParam(np.random.normal(0.0, 0.01, 1), Belle2.VxdID(1, 0, 0, 2).getID(), 2)
payload.setGlobalParam(np.random.normal(0.0, 0.01, 1), Belle2.VxdID(1, 0, 0, 2).getID(), 3)

# misalignment of Pat
payload.setGlobalParam(np.random.normal(0.0, 0.01, 1), Belle2.VxdID(3, 0, 0, 1).getID(), 1)
payload.setGlobalParam(np.random.normal(0.0, 0.01, 1), Belle2.VxdID(3, 0, 0, 1).getID(), 2)
payload.setGlobalParam(np.random.normal(0.0, 0.01, 1), Belle2.VxdID(3, 0, 0, 1).getID(), 3)

# misalignment of Mat
payload.setGlobalParam(np.random.normal(0.0, 0.01, 1), Belle2.VxdID(3, 0, 0, 2).getID(), 1)
payload.setGlobalParam(np.random.normal(0.0, 0.01, 1), Belle2.VxdID(3, 0, 0, 2).getID(), 2)
payload.setGlobalParam(np.random.normal(0.0, 0.01, 1), Belle2.VxdID(3, 0, 0, 2).getID(), 3)

Belle2.Database.Instance().storeData('VXDAlignment', payload, Belle2.IntervalOfValidity(0, 0, -1, -1))
