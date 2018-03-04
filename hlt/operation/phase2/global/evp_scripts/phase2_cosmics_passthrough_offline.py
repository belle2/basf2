#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from softwaretrigger.path_functions import setup_basf2_and_db, create_hlt_path, add_hlt_dqm, finalize_hlt_path


args = setup_basf2_and_db(dbfile='/home/usr/hltdaq/database/LocalDB.rel0101/database.txt')
path = create_hlt_path(args, inputfile='INPUTFILE', dqmfile='dqmfile.root')

# no reconstruction or software trigger added at all

finalize_hlt_path(path, args, outputfile='OUTPUTFILE')
basf2.print_path(path)
basf2.process(path)
