#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import hashlib


def file_checksum(filename):
    """Calculate md5 hash of file"""
    md5hash = hashlib.md5()
    with open(filename, "rb") as data:
        md5hash.update(data.read())
    return md5hash.hexdigest()
