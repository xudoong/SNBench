import os 
from collections import defaultdict
import numpy as np
import pandas as pd
from matplotlib import pyplot as plt

os.makedirs('./result/figure/csv', exist_ok=True)

INTEL_SIMD = ['scalar', 'sse_128', 'avx_256', 'avx_512']
ARM_SIMD = ['scalar', 'neon_128']

DEV_LIST = ['spr', 'gna', 'kp']
SIMD_LIST =  {
    'spr': INTEL_SIMD,
    'gna': INTEL_SIMD,
    'kp': ARM_SIMD

}

INTEL_CONFIG = ['gcc_scalar', 'aocc', 'icc10', 'icc40', 'sleef10', 'sleef40']
ARM_CONFIG = ['gcc_scalar', 'sleef10', 'sleef40', 'aml']
CONFIG_LIST = {
    'spr': INTEL_CONFIG,
    'gna': INTEL_CONFIG,
    'kp': ARM_CONFIG
}

CONFIG_REPLACE_DICT = {
    'gcc_scalar': 'glibm',
    'aocc': 'amdlibm',
    'icc10': 'svml10',
    'icc40': 'svml40',
}

def parse_simd_data(file_path, simd):
    if 'kp_' in file_path:
        col_idx = {'scalar': 2, 'neon_128': 3}[simd]
    else:
        col_idx = {'scalar': 2, 'sse_128': 3, 'avx_256': 4, 'avx_512':5}[simd]
    rv = {}
    with open(file_path, 'r') as f:
        for line in f.readlines():
            tokens = line.split(',')
            sn_config = tokens[0] + '-' + tokens[1]
            value = tokens[col_idx]
            rv[sn_config] = float(value)
    return rv 


def main():
    for dev in DEV_LIST:
        for simd in SIMD_LIST[dev]:
            df_dict = {}
            for config in CONFIG_LIST[dev]:
                if config == 'gcc_scalar' and simd != 'scalar':
                    continue
                file_path = f'./result/{dev}_{config}.csv'
                if config in CONFIG_REPLACE_DICT:
                    config = CONFIG_REPLACE_DICT[config]
                df_dict[config] = parse_simd_data(file_path, simd)
            df = pd.DataFrame(df_dict)
            print('==============')
            print(simd)
            print(df)
            df.to_csv(f'./result/figure/csv/{dev}_{simd}.csv')


if __name__ == '__main__':
    main()