#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Test SeqRootInputModule. Input data is the same as used by chain_input.py
# (both files)

from basf2 import *
from ROOT import Belle2
from contextlib import contextmanager
import tempfile
import gzip
import glob
import struct
import multiprocessing

set_random_seed("something important")
# simplify logging output to just the type and the message
logging.enable_summary(False)
for level in LogLevel.values.values():
    logging.set_info(level, LogInfo.LEVEL | LogInfo.MESSAGE)


class TestModule(Module):

    """Test to read relations in the input files."""

    def event(self):
        """reimplementation of Module::event().

        prints PXD true and simhit indices, using relations
        """

        truehits = Belle2.PyStoreArray('PXDTrueHits')
        for truehit in truehits:
            relations = truehit.getRelationsTo("PXDSimHits")
            print("truehit %d => hits " % truehit.getArrayIndex(), end="")
            for hit in relations:
                print(hit.getArrayIndex(), end=", ")
            print()


@contextmanager
def clean_working_directory():
    """Context manager to create a temporary directory and directly us it as
    current working directory"""
    dirname = os.getcwd()
    try:
        with tempfile.TemporaryDirectory() as tempdir:
            os.chdir(tempdir)
            yield tempdir
    finally:
        os.chdir(dirname)


def fork_process(*args, target=process):
    """Run function in forked child to eliminate side effects"""
    sub = multiprocessing.Process(target=target, args=args)
    sub.start()
    sub.join()

# ============================================================================
# Now lets create the necessary modules to perform a simulation

filename = Belle2.FileSystem.findFile('framework/tests/seqroot_input.sroot')

main = create_path()
seqinput = main.add_module("SeqRootInput", logLevel=LogLevel.WARNING)
main.add_module("EventInfoPrinter")
main.add_module("PrintCollections")
main.add_module(TestModule())

with clean_working_directory():
    content = open(filename, "rb").read()
    first_record = struct.unpack("@i", content[:4])[0]
    second_record = struct.unpack("@i", content[first_record:first_record+4])[0] + first_record
    # write the file twice, we want to check that we will read the sroot and the
    # sroot-1. But we need to omit the first record in the second one
    open("01-ok.sroot", "wb").write(content)
    open("01-ok.sroot-1", "wb").write(content[first_record:])
    # write the first record + 1 byte which will cause an invalid read on the
    # record size
    open("02-1+1b.sroot", "wb").write(content[:first_record+1])
    # now just write the first record +  5 bytes: this will be enough for the
    # next record size but will not contain the record
    open("03-1+5b.sroot", "wb").write(content[:first_record+5])
    # write the second record + 1 byte which will cause an invalid read on the
    # record size
    open("04-2+1b.sroot", "wb").write(content[:second_record+1])
    # now just write the second record +  5 bytes: this will be enough for the
    # next record size but will not contain the record
    open("05-2+5b.sroot", "wb").write(content[:second_record+5])
    gzip.open("05-ok.sroot.gz", "wb").write(content)
    gcontent = open("05-ok.sroot.gz", "rb").read()
    gzip.open("05-ok.sroot-1.gz", "wb").write(content[first_record:])
    gzip.open("06-1+1b.sroot.gz", "wb").write(content[:first_record+1])
    gzip.open("07-1+5b.sroot.gz", "wb").write(content[:first_record+5])
    # write the uncompressed file with compression suffix to trigger
    # decompression error
    open("08-nogz.sroot.gz", "wb").write(content)
    # only write half the compressed file, this will give an decompression error
    open("09-cut.sroot.gz", "wb").write(gcontent[:len(gcontent)//2])
    # and one where the record size is extremely large
    open("10-max.sroot", "wb").write(struct.pack("@i", 0x7fffffff))

    files = sorted(glob.glob("*.sroot") + glob.glob("*.sroot.gz"))

    for filename in files:
        print("Trying", filename, "...")
        seqinput.param("inputFileName", filename)
        fork_process(main)
