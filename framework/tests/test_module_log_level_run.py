#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import subprocess

for i in ['INFO', 'WARNING', 'ERROR', 'DEBUG']:
    i_aux = f'EventInfoPrinter:{i}'
    result = subprocess.run(['basf2', 'framework/tests/test_module_log_level.py',
                            '--module_log_level', f'{i_aux}'], capture_output=True, text=True)

    result = result.stdout

    start = 'Log-level for module EventInfoPrinter: '
    end = '\n[INFO]'
    idx1 = result.find(start)
    idx2 = result.find(end, idx1+len(start))
    res = result[idx1+len(start):idx2]

    print(i, res)
    if i == res:
        print('Module log-level set correctly')
