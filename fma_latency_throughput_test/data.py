from collections import defaultdict

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

x86_file_path = ['./result/spr_latency.csv', './result/spr_throughput.csv', './result/gna_latency.csv', './result/gna_throughput.csv']
arm_file_path = ['./result/kp_latency.csv', './result/kp_throughput.csv']
MAX_THRESHOLD = 10000

def get_min_max_std(data_list):
    if len(data_list) == 0:
        return 0, 0, 0
    data_list = np.array(data_list)
    try:
        data_list = data_list[np.abs(data_list) < MAX_THRESHOLD]
    except:
        print(data_list)
        exit()
    return np.min(data_list), np.max(data_list), round(np.std(data_list), 3)

def tuple_to_csv_list(t):
    return ','.join([str(v) for v in t])

def x86():
    for file_path in x86_file_path:
        try:
            df = pd.read_csv(file_path, header=None, names=['op', 'fmt', 'i1', 'i2', 'i3', 'cpi-64', 'cpi-128', 'cpi-256', 'cpi-512'])
            df = df.drop(columns=['i1', 'i2', 'i3'])
            data = defaultdict(lambda: {'cpi-64': [], 'cpi-128': [], 'cpi-256': [], 'cpi-512': []})
            for index, row in df.iterrows():
                sn_config = row['op'].strip() + '-' + row['fmt'].strip()
                data[sn_config]['cpi-64'].append(row['cpi-64'])
                data[sn_config]['cpi-128'].append(row['cpi-128'])
                data[sn_config]['cpi-256'].append(row['cpi-256'])
                data[sn_config]['cpi-512'].append(row['cpi-512'])

            with open(file_path.replace('result/', 'result/stat_'), 'w') as f:
                pass
            for sn_config, cpi_data in data.items():
                cpi_64_stat = get_min_max_std(cpi_data['cpi-64'])
                cpi_128_stat = get_min_max_std(cpi_data['cpi-128'])
                cpi_256_stat = get_min_max_std(cpi_data['cpi-256'])
                cpi_512_stat = get_min_max_std(cpi_data['cpi-512'])
                print(sn_config, cpi_64_stat, cpi_128_stat, cpi_256_stat, cpi_512_stat)
                with open(file_path.replace('result/', 'result/stat_'), 'a') as f:
                    f.write(f'{sn_config},{tuple_to_csv_list(cpi_64_stat)},{tuple_to_csv_list(cpi_128_stat)},{tuple_to_csv_list(cpi_256_stat)},{tuple_to_csv_list(cpi_512_stat)}\n')
            print()
            print()
        except:
            pass

def arm():
    for file_path in arm_file_path:
        df = pd.read_csv(file_path, header=None, names=['op', 'fmt', 'i1', 'i2', 'i3', 'cpi-64', 'cpi-128'])
        df = df.drop(columns=['i1', 'i2', 'i3'])
        data = defaultdict(lambda: {'cpi-64': [], 'cpi-128': []})
        for index, row in df.iterrows():
            sn_config = row['op'].strip() + '-' + row['fmt'].strip()
            data[sn_config]['cpi-64'].append(row['cpi-64'])
            data[sn_config]['cpi-128'].append(float(row['cpi-128']))

        with open(file_path.replace('result/', 'result/stat_'), 'w') as f:
            pass
        for sn_config, cpi_data in data.items():
            cpi_64_stat = get_min_max_std(cpi_data['cpi-64'])
            cpi_128_stat = get_min_max_std(cpi_data['cpi-128'])
            print(sn_config, cpi_64_stat, cpi_128_stat)
            with open(file_path.replace('result/', 'result/stat_'), 'a') as f:
                f.write(f'{sn_config},{tuple_to_csv_list(cpi_64_stat)},{tuple_to_csv_list(cpi_128_stat)}\n')

x86()
arm()