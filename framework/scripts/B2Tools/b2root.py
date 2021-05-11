#!/usr/bin/env python3

import os
import sys
import tempfile
import shutil
from basf2 import B2ERROR


byteorder = 'big'


class RawRootFile:
    """
    Interface to binary root file content
    """

    def __init__(self, filename):
        """
        Open a root file and read its header
        """

        #: binary root file
        self.rootfile = open(filename, 'rb')
        root = self.rootfile.read(4)
        if root != b'root':
            raise Exception("%s is not a root file" % filename)
        #: root/file format version
        self.version = int.from_bytes(self.rootfile.read(4), byteorder)
        #: are file location pointers 4 or 8 bits?
        self.large = (self.version >= 1000000)
        #: size of file location pointers
        self.wordlen = 8 if self.large else 4
        #: pointer to first data record
        self.begin = int.from_bytes(self.rootfile.read(4), byteorder)
        #: pointer to first free word at the EOF
        self.end = int.from_bytes(self.rootfile.read(self.wordlen), byteorder)
        #: pointer to FREE data record
        self.seekfree = int.from_bytes(self.rootfile.read(self.wordlen), byteorder)
        #: number of bytes in FREE data record
        self.nbytesfree = int.from_bytes(self.rootfile.read(4), byteorder)
        #: number of free data records
        self.nfree = int.from_bytes(self.rootfile.read(4), byteorder)
        #: number of bytes in TNamed at creation time
        self.nbytesname = int.from_bytes(self.rootfile.read(4), byteorder)
        #: number of bytes for file pointers
        self.units = int.from_bytes(self.rootfile.read(1), byteorder)
        #: compression level and algorithm
        self.compress = int.from_bytes(self.rootfile.read(4), byteorder)
        #: pointer to TStreamerInfo record
        self.seekinfo = int.from_bytes(self.rootfile.read(self.wordlen), byteorder)
        #: number of bytes in TStreamerInfo record
        self.nbytesinfo = int.from_bytes(self.rootfile.read(self.wordlen), byteorder)
        #: universal unique ID
        self.uuid = self.rootfile.read(18)
        self.rootfile.seek(0)
        #: binary header
        self.header = self.rootfile.read(self.begin)
        self.rootfile.seek(self.begin)
        #: pointer to KeysList record
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

            #: are file location pointers 4 or 8 bits?
            self.large = large
            #: size of file location pointers
            self.wordlen = 8 if self.large else 4
            pos = 0
            #: length of compressed object (in bytes)
            self.nbytes, pos = self.get_int(keydata, pos)
            #: TKey version identifier
            self.version, pos = self.get_int(keydata, pos, 2)
            #: length of uncompressed object
            self.objlen, pos = self.get_int(keydata, pos)
            #: date and time when object was written to file
            self.datime, pos = keydata[pos:pos+4], pos+4
            #: length of the key structure (in bytes)
            self.keylen, pos = self.get_int(keydata, pos, 2)
            #: cycle of key
            self.cycle, pos = self.get_int(keydata, pos, 2)
            #: pointer to record itself (consistency check)
            self.seekkey, pos = self.get_int(keydata, pos, self.wordlen)
            #: pointer to directory header
            self.seekpdir, pos = self.get_int(keydata, pos, self.wordlen)
            #: object Class Name
            self.classname, pos = self.get_string(keydata, pos)
            #: name of the object
            self.name, pos = self.get_string(keydata, pos)
            #: title of the object
            self.title, pos = self.get_string(keydata, pos)
            #: binary header
            self.header = keydata[:pos]
            #: data bytes associated to the object
            self.data = keydata[pos:]
            #: name to show in the list of keys
            self.showname = self.classname

        def normalize(self, pos=None, offset=None):
            """
            Set the key datime to zero adjust the pointer to itself if given as argument
            """

            #: date and time when object was written to file
            self.datime = b'\x00' * 4
            if self.seekkey > 0:
                if pos:
                    #: pointer to record itself (consistency check)
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


def normalize_file(filename, output=None, in_place=False, name=None, root_version=None):
    """
    Reset the non-reproducible root file metadata: UUID and datimes.
    It can also reset the initial file name stored in the file itself, but
    (WARNING!) this may corrupt the root file.
    """

    # open input file
    rootfile = RawRootFile(filename)

    # adjust root version number
    if root_version:
        rootfile.version = root_version

    # create output file
    if output:
        newrootfile = open(output, 'wb')
    elif in_place:
        newrootfile = tempfile.TemporaryFile()
    else:
        basename, ext = os.path.splitext(filename)
        newrootfile = open(basename + '_normalized' + ext, 'wb')

    # write output file header
    newrootfile.write(rootfile.header)

    # file name in the metadata
    if name:
        newname = name.encode()
    else:
        newname = None

    # bookkeeping of offsets, positions, and keys
    offset = 0
    seekfree = rootfile.seekfree
    nbytesfree = rootfile.nbytesfree
    nbytesname = rootfile.nbytesname
    seekinfo = rootfile.seekinfo
    keylist = []
    keyskey = None
    infokey = None
    swap = False

    for key in rootfile:
        # reset datime and adjust position of key
        key.normalize(pos=newrootfile.tell())

        # Special treatment of key containing the TFile information
        if key.showname == b'TFile':

            # if a new name is given change the name and determine the offset caused by the change of name length
            namelen = len(key.name)
            if newname:
                key.name = key.filename = newname
            offset = len(key.name) - namelen

            # apply the offset to total (2x), object, and key length
            key.nbytes += 2*offset
            key.objlen += offset
            key.keylen += offset

            # recreate the header from updated data members
            key.recreate_header()

            # recreate the key data with new name and lengths
            buffer = len(key.name).to_bytes(1, byteorder) + key.name
            buffer += len(key.title).to_bytes(1, byteorder) + key.title
            buffer += key.version.to_bytes(2, byteorder)
            buffer += (0).to_bytes(8, byteorder)  # reset datimeC and datimeM
            buffer += (key.nbyteskeys + offset).to_bytes(4, byteorder)
            buffer += (key.nbytesname + 2*offset).to_bytes(4, byteorder)
            wordlen = 8 if key.version > 1000 else 4
            buffer += key.seekdir.to_bytes(wordlen, byteorder)
            buffer += key.seekparent.to_bytes(wordlen, byteorder)
            seekkeyspos = newrootfile.tell() + len(key.header) + len(buffer)
            buffer += (rootfile.seekkeys + 2*offset).to_bytes(wordlen, byteorder)
            buffer += (0).to_bytes(18, byteorder)  # reset UUID
            if key.version <= 1000:
                buffer += (0).to_bytes(12, byteorder)
            key.data = buffer

        else:
            # check whether we break pointers in TTrees
            if key.classname == b'TTree' and offset != 0:
                B2ERROR('Changing the name of root files containing a tree is not supported.')
                if not in_place:
                    os.remove(newrootfile.name)
                sys.exit(1)

            # update key data in KeysList: number of keys and key headers
            # and remember key position
            if key.showname == b'KeysList':
                seekkeys = newrootfile.tell()
                buffer = len(keylist).to_bytes(4, byteorder)
                for filekey in keylist:
                    buffer += filekey.header
                key.data = buffer
                keyskey = key
                swap = (infokey is None)

            # update free segments pointer and remember key position
            if key.showname == b'FreeSegments':
                seekfree = newrootfile.tell()
                pointer = int.from_bytes(key.data[2:6], byteorder) + 4*offset
                key.data = key.data[:2] + pointer.to_bytes(4, byteorder) + key.data[6:]

            # update name in KeysList and FreeSegments
            if key.showname in [b'KeysList', b'FreeSegments'] and newname:
                key.name = newname
                key.nbytes += offset
                key.keylen += offset
                key.recreate_header()

            # keep track of all keys for the KeysList
            elif key.showname not in [b'StreamerInfo', b'']:
                keylist.append(key)

            # remember streamer info key and position
            if key.showname == b'StreamerInfo':
                seekinfo = newrootfile.tell()
                infokey = key

        # write the updated key, making sure the KeysList come after the StreamerInfo
        if swap and key.showname == b'KeysList':
            pass
        elif swap:
            seekinfo = newrootfile.tell()
            infokey.normalize(pos=seekinfo)
            newrootfile.write(infokey.header)
            newrootfile.write(infokey.data)
            seekkeys = newrootfile.tell()
            keyskey.normalize(pos=seekkeys)
            newrootfile.write(keyskey.header)
            newrootfile.write(keyskey.data)
            swap = False
        else:
            newrootfile.write(key.header)
            newrootfile.write(key.data)

    # write the new file header
    rootfile.normalize(end=newrootfile.tell(), seekfree=seekfree, nbytesfree=nbytesfree+offset,
                       nbytesname=nbytesname+2*offset, seekinfo=seekinfo)
    newrootfile.seek(0)
    newrootfile.write(rootfile.header)

    # update pointer to keyslist
    newrootfile.seek(seekkeyspos)
    newrootfile.write(seekkeys.to_bytes(wordlen, byteorder))

    # replace in the input file if the in-place option is used
    if in_place:
        del rootfile
        newrootfile.seek(0)
        shutil.copyfileobj(newrootfile, open(filename, 'wb'))

    newrootfile.close()
