#!/usr/bin/env python3
# -*- coding: utf-8 -*-


byteorder = 'big'


class RawRootFile:
    """
    Interface to binary root file content
    """

    def __init__(self, filename):
        """
        Open a root file and read its header
        """

        self.rootfile = open(filename, 'rb')
        root = self.rootfile.read(4)
        if root != b'root':
            raise Exception("%s is not a root file" % filename)
        self.version = int.from_bytes(self.rootfile.read(4), byteorder)
        self.large = (self.version >= 1000000)
        self.wordlen = 8 if self.large else 4
        self.begin = int.from_bytes(self.rootfile.read(4), byteorder)
        self.end = int.from_bytes(self.rootfile.read(self.wordlen), byteorder)
        self.seekfree = int.from_bytes(self.rootfile.read(self.wordlen), byteorder)
        self.nbytesfree = int.from_bytes(self.rootfile.read(4), byteorder)
        self.nfree = int.from_bytes(self.rootfile.read(4), byteorder)
        self.nbytesname = int.from_bytes(self.rootfile.read(4), byteorder)
        self.units = int.from_bytes(self.rootfile.read(1), byteorder)
        self.compress = int.from_bytes(self.rootfile.read(4), byteorder)
        self.seekinfo = int.from_bytes(self.rootfile.read(self.wordlen), byteorder)
        self.nbytesinfo = int.from_bytes(self.rootfile.read(self.wordlen), byteorder)
        self.uuid = self.rootfile.read(18)
        self.rootfile.seek(0)
        self.header = self.rootfile.read(self.begin)
        self.rootfile.seek(self.begin)
        self.seekkeys = 0

    def __del__(self):
        """
        Close file
        """

        if hasattr(self, 'rootfile'):
            self.rootfile.close()

    def normalize(self, end=None, seekfree=None, nbytesfree=None, nbytesname=None, seekinfo=None):
        """
        Set UUID to zero and adjust pointers that are given as arguments
        """

        if end:
            self.end = end
        if seekfree:
            self.seekfree = seekfree
        if nbytesfree:
            self.nbytesfree = nbytesfree
        if nbytesname:
            self.nbytesname = nbytesname
        if seekinfo:
            self.seekinfo = seekinfo
        self.uuid = b'\x00' * 18

        self.header = b'root' \
            + self.version.to_bytes(4, byteorder) \
            + self.begin.to_bytes(4, byteorder) \
            + self.end.to_bytes(self.wordlen, byteorder) \
            + self.seekfree.to_bytes(self.wordlen, byteorder) \
            + self.nbytesfree.to_bytes(4, byteorder) \
            + self.nfree.to_bytes(4, byteorder) \
            + self.nbytesname.to_bytes(4, byteorder) \
            + self.units.to_bytes(1, byteorder) \
            + self.compress.to_bytes(4, byteorder) \
            + self.seekinfo.to_bytes(self.wordlen, byteorder) \
            + self.nbytesinfo.to_bytes(self.wordlen, byteorder) \
            + self.uuid
        nzero = self.begin - len(self.header)
        self.header += b'\x00' * nzero

    def __iter__(self):
        """
        iterator interface
        """

        return self

    class Key:
        """
        Root file key
        """

        def __init__(self, keydata, large=False):
            """
            Obtain key header and data payload from given data and extract header information
            """

            self.large = large
            self.wordlen = 8 if self.large else 4
            pos = 0
            self.nbytes, pos = self.get_int(keydata, pos)
            self.version, pos = self.get_int(keydata, pos, 2)
            self.objlen, pos = self.get_int(keydata, pos)
            self.datime, pos = keydata[pos:pos+4], pos+4
            self.keylen, pos = self.get_int(keydata, pos, 2)
            self.cycle, pos = self.get_int(keydata, pos, 2)
            self.seekkey, pos = self.get_int(keydata, pos, self.wordlen)
            self.seekpdir, pos = self.get_int(keydata, pos, self.wordlen)
            self.classname, pos = self.get_string(keydata, pos)
            self.name, pos = self.get_string(keydata, pos)
            self.title, pos = self.get_string(keydata, pos)
            self.header = keydata[:pos]
            self.data = keydata[pos:]
            self.showname = self.classname

        def normalize(self, pos=None, offset=None):
            """
            Set the key datime to zero adjust the pointer to itself if given as argument
            """

            self.datime = b'\x00' * 4
            if self.seekkey > 0:
                if pos:
                    self.seekkey = pos
                elif offset:
                    self.seekkey += offset
            self.recreate_header()

        def recreate_header(self):
            """
            Build the binary header information from the data members
            """

            self.header = self.nbytes.to_bytes(4, byteorder) \
                + self.version.to_bytes(2, byteorder) \
                + self.objlen.to_bytes(4, byteorder) \
                + self.datime \
                + self.keylen.to_bytes(2, byteorder) \
                + self.cycle.to_bytes(2, byteorder) \
                + self.seekkey.to_bytes(self.wordlen, byteorder) \
                + self.seekpdir.to_bytes(self.wordlen, byteorder) \
                + len(self.classname).to_bytes(1, byteorder) + self.classname \
                + len(self.name).to_bytes(1, byteorder) + self.name \
                + len(self.title).to_bytes(1, byteorder) + self.title

        def get_int(self, data, pos, wordlen=4):
            """
            Helper function to read an int from binary data
            """

            return (int.from_bytes(data[pos:pos+wordlen], byteorder), pos+wordlen)

        def get_string(self, data, pos):
            """
            Helper function to read a string from binary data
            """

            strlen = data[pos]
            return (data[pos+1:pos+1+strlen], pos+1+strlen)

    def __next__(self):
        """
        Iterate to next key
        """

        pos = self.rootfile.tell()
        large = pos > 0x80000000
        nbytes = int.from_bytes(self.rootfile.read(4), byteorder)
        if (nbytes == 0):
            raise StopIteration()
        self.rootfile.seek(-4, 1)
        result = self.Key(self.rootfile.read(nbytes), large)
        if pos == self.seekfree:
            result.showname = b'FreeSegments'
        elif pos == self.seekinfo:
            result.showname = b'StreamerInfo'
        elif pos == self.seekkeys:
            result.showname = b'KeysList'

        if result.showname == b'TFile':
            result.filename, pos = result.get_string(result.data, 0)
            result.filetitle, pos = result.get_string(result.data, pos)
            result.version = int.from_bytes(result.data[pos:pos+2], byteorder)
            result.nbyteskeys = int.from_bytes(result.data[pos+10:pos+14], byteorder)
            result.nbytesname = int.from_bytes(result.data[pos+14:pos+18], byteorder)
            if result.version > 1000:
                result.seekdir = int.from_bytes(result.data[pos+18:pos+26], byteorder)
                result.seekparent = int.from_bytes(result.data[pos+26:pos+34], byteorder)
                self.seekkeys = int.from_bytes(result.data[pos+34:pos+42], byteorder)
            else:
                result.seekdir = int.from_bytes(result.data[pos+18:pos+22], byteorder)
                result.seekparent = int.from_bytes(result.data[pos+22:pos+26], byteorder)
                self.seekkeys = int.from_bytes(result.data[pos+26:pos+30], byteorder)
        return result
