#!/usr/bin/env python3
# -*- coding: utf-8 -*-


def chunks(l, n):
    """Yield successive n-sized chunks from l."""
    for i in range(0, len(l), int(n)):
        yield l[i:i + int(n)]
