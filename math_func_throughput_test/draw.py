import os 
import pandas as pd
from matplotlib import pyplot as plt
import numpy as np


FIGURE_DIR = './result/figure'
INPUT_DIR = './result/figure/csv'

COLORS = {'glibm': 'white', 'amdlibm': 'white', 'svml10': 'white', 'svml40': 'white', 'sleef10': 'white', 'sleef40': 'white', 'aml': 'white'}
COLORS = {'glibm': '#BDBDBD', 'amdlibm': '#9CCEA8', 'svml10': '#FFB963', 'svml40': '#FFE1B8', 'sleef10': '#9EC9E2', 'sleef40': '#C6E1EF', 'aml': '#E1C37E'}
SN_CONFIGS = ['pow-nnn', 'pow-nsn', 'pow-nns', 'pow-nss', 'pow-snn', 'pow-ssn', 'exp-nn', 'exp-ns', 'exp-sn', 'log-nn', 'log-ns', 'sin-nn', 'sin-ss', 'cos-nn', 'cos-ns', 'tan-nn', 'tan-ss']
NICE_SUBNORMAL_SN_CONFIGS = ['pow-nns', 'pow-nss', 'exp-ns', 'sin-ss', 'cos-ns', 'tan-ss']
NORMAL_SN_CONFIGS = ['pow-nnn', 'exp-nn', 'log-nn', 'sin-nn', 'cos-nn', 'tan-nn']

SN_CONFIG_TO_IDX_DICT = {SN_CONFIGS[i]: i for i in range(len(SN_CONFIGS))}

# FIGURE_FILTER = ['spr_scalar', 'spr_avx_512', 'gna_scalar', 'gna_avx_512', 'kp_scalar', 'kp_neon_128']

LEGEND_MAPPING = {
    'glibm': 'glibc',
    'amdlibm': 'aocl',
    'svml10': 'SVML-1.0-ULP',
    'svml40': 'SVML-4.0-ULP',
    'sleef10': 'SLEEF-1.0-ULP',
    'sleef40': 'SLEEF-4.0-ULP',
    'aml': 'AML',
}
# plt.style.use('seaborn')

def get_container_slowdown(container):
    value_list = [p.get_height() for p in container.patches]
    value_list
    normal_value_dict = {
        'pow': value_list[SN_CONFIG_TO_IDX_DICT['pow-nnn']],
        'exp': value_list[SN_CONFIG_TO_IDX_DICT['exp-nn']],
        'log': value_list[SN_CONFIG_TO_IDX_DICT['log-nn']],
        'sin': value_list[SN_CONFIG_TO_IDX_DICT['sin-nn']],
        'cos': value_list[SN_CONFIG_TO_IDX_DICT['cos-nn']],
        'tan': value_list[SN_CONFIG_TO_IDX_DICT['tan-nn']],
    }

    # slowdown = [value_list[i] / (normal_value_dict[SN_CONFIGS[i].split('-')[0]] + 1e-8) for i in range(len(value_list))]
    # for i in range(len(slowdown)):
    #     v = slowdown[i]
    #     t = ''
    #     if v > 10:
    #         t = f'{round(v)}x'
    #     elif v > 1.05:
    #         t = f'{round(v, 1)}x'
    #     slowdown[i] = t
    # return slowdown
    delta_list = [int(value_list[i]) - int(normal_value_dict[SN_CONFIGS[i].split('-')[0]]) for i in range(len(value_list))]
    faster_list = []
    slower_list = []
    for i in range(len(delta_list)):
        delta = int(delta_list[i])
        if delta > 0:
            slower_list.append(f'+{delta}')
            faster_list.append('')
        elif delta < 0:
            faster_list.append(str(delta))
            slower_list.append('')
        else:
            faster_list.append('')
            slower_list.append('')
    return faster_list, slower_list


def draw_one(file_path):
    output_path = os.path.join(FIGURE_DIR, os.path.basename(file_path).replace('.csv', '.png'))
    df = pd.read_csv(file_path)
    df.index = df['Unnamed: 0']
    df.index.name = 'sn-config'
    df = df.drop(columns=['Unnamed: 0'])
    print(df)
    ax = plt.figure(figsize=(80, 10), dpi=100).add_subplot(111)
    df.plot.bar(ax=ax, legend=True, logy=True, xlabel='', color=COLORS, width=0.95)

    # set bar value
    for container in ax.containers:
        faster_list, slower_list = get_container_slowdown(container)
        ax.bar_label(container, fontsize=45, fmt='%d', color='black', rotation=70)
        ax.bar_label(container, labels=faster_list, label_type='center', fontsize=40, color='blue', rotation=90, padding=23, fontweight='bold')
        ax.bar_label(container, labels=slower_list, label_type='center', fontsize=40, color='red', rotation=90, padding=0, fontweight='bold')
        for bar in container:
            bar.set_edgecolor('black')

    # set hatch
    # bars = ax.patches
    # hatches = ''.join(h*len(df) for h in ['/', 'o', '\\', 'x', '-', 'O'])
    # if 'glibm' not in df.columns:
    #     hatches = hatches[len(df):]
    # for bar, hatch in zip(bars, hatches):
    #     bar.set_hatch(hatch)

    # draw vertical split line
    op_split_list = [5, 8, 10, 12, 14]
    for i in op_split_list:
        plt.axvline(x=ax.get_xticks()[i]+0.5, color='black', linestyle='--', linewidth=3)
    for i in range(16):
        if i not in op_split_list:
            plt.axvline(x=ax.get_xticks()[i]+0.5, color='black', linestyle='-.', linewidth=3, ymax=0.66)


    plt.xticks(fontsize=40, rotation=0) 
    idx_of_nice_sn_configs = [SN_CONFIG_TO_IDX_DICT[c] for c in NICE_SUBNORMAL_SN_CONFIGS]
    x_tick_labels = ax.get_xticklabels()
    # for i, label in enumerate(x_tick_labels):
    #     if i in idx_of_nice_sn_configs:
    #         label.set_color('blue')

    xlabel_texts = [item.get_text() for item in x_tick_labels]
    # for c in NORMAL_SN_CONFIGS:
    #     xlabel_texts[SN_CONFIG_TO_IDX_DICT[c]] += '*'
    # for c in NICE_SUBNORMAL_SN_CONFIGS:
    #     xlabel_texts[SN_CONFIG_TO_IDX_DICT[c]] += '#'
    ax.set_xticklabels(xlabel_texts)

    plt.yticks(fontsize=40) 
    plt.ylabel('Reciprocal Throughput (CPE)', fontsize=35)

    # 替换legend中的标签
    legend =  plt.legend(fontsize=30, loc='upper right', bbox_to_anchor=(1, 1), ncol=10)
    for text in legend.get_texts():
        original_label = text.get_text()
        new_label = LEGEND_MAPPING.get(original_label, original_label)  # 使用映射字典，如果没有映射则保持原始标签
        text.set_text(new_label)
        
    np_df = df.to_numpy()
    np_df = np_df[~np.isnan(np_df)]
    if 'spr_scalar' in output_path:
        ymax = np_df.max() * 5
    else:
        ymax = np_df.max() * 3
    plt.ylim(0, ymax)
    plt.savefig(output_path, bbox_inches='tight')


def main():
    for file in sorted(os.listdir(INPUT_DIR)):
        file_path = os.path.join(INPUT_DIR, file)
        draw_one(file_path)


if __name__ == '__main__':
    main()