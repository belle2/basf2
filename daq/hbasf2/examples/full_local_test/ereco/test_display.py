#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2


if __name__ == "__main__":
    path = basf2.Path()
    path.add_module("ReceiveEvent", Host="erctl", Port=4002)

    # path.add_module("PrintCollections", printForEvent=0)
    path.add_module("EventInfoPrinter")

    basf2.process(path)
