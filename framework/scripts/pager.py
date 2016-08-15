#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import tempfile
import sys
import subprocess


class Pager(object):

    def __enter__(self):
        self.tmp_file = tempfile.NamedTemporaryFile(mode='w')
        sys.stdout = self.tmp_file

    def __exit__(self, exc_type, exc_val, exc_tb):
        sys.stdout.flush()
        p = subprocess.Popen(['less', '-R', self.tmp_file.name])
        p.communicate()
        self.tmp_file.close()
        sys.stdout = sys.__stdout__


if __name__ == '__main__':
    with Pager():
        print("This is an example on how to use the pager.")
        print("Should be pretty easy.")
