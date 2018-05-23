import matplotlib.pyplot as plt
import csv
import sys
import pandas as pd
import yaml

x = []
y = []

with open("perf_config.yml", 'r') as ymlfile:
    cfg = yaml.load(ymlfile)


data = pd.read_csv(cfg['config']['result_dir']+ '/' + sys.argv[1])
data.set_index("test_id", inplace= True)

data_algo_0_5 = data.loc[(data['load'] == 0.5)]
data_algo_w_flush_load_0_5 = data.loc[(data['algo'] == 'algo_w_flush') & (data['load'] == 0.5)]
data_algo_wo_flush_load_0_5 = data.loc[(data['algo'] == 'algo_wo_flush') & (data['load'] == 0.5)]
data_algo_lazy_list_load_0_5 = data.loc[(data['algo'] == 'lazy_list') & (data['load'] == 0.5)]

print(data_algo_w_flush_load_0_5.head())
print(data_algo_wo_flush_load_0_5.head())
print(data_algo_lazy_list_load_0_5.head())

color_dict = {'algo_w_flush': '#FF0000', 'algo_wo_flush': '#0000FF'}


data_algo_0_5.plot(x='nbthreads', y='throughput')
data_algo_w_flush_load_0_5.plot(x='nbthreads', y='throughput')
data_algo_wo_flush_load_0_5.plot(x='nbthreads', y='throughput')
data_algo_lazy_list_load_0_5.plot(x='nbthreads', y='throughput')
plt.show()




