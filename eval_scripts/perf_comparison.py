#!/usr/bin/env python3

import sys
import os
import argparse
import socket
from datetime import datetime
import shutil
from subprocess import call,DEVNULL,STDOUT,check_output,CalledProcessError
import shlex
import yaml
import itertools
import math

####################
## some useful functions

def stringTime():
    return datetime.now().strftime("%Y-%m-%d-%H:%M:%S")


##### other config options
NUMA_FT="-l"
NUMA_INTERLEAVE="-i all"

ALLOC_PT="0"

# about the local env
HOME=os.getcwd()
HOSTNAME=socket.gethostname()
STARTDATE=stringTime()


with open("perf_config.yml", 'r') as ymlfile:
    cfg = yaml.load(ymlfile)

if cfg['test']['test_name'] == "test":
    PREFIXMAKE="test"
    PREFIXEXEC="bench_LI"
else:
    PREFIXMAKE="alt"
    PREFIXEXEC="bench_LRI"


print("%s: Running exp with %d runs"%(stringTime(),int(cfg['test']['nb_runs'])))

print("%s: Creating dir %s to store results"%(stringTime(),cfg['config']['result_dir_tmp']))
shutil.rmtree(cfg['config']['result_dir_tmp'],ignore_errors=True)
os.mkdir(cfg['config']['result_dir_tmp'])

print("%s: Setting up working dir %s "%(stringTime(),cfg['config']['work_dir']))
shutil.rmtree(cfg['config']['work_dir'],ignore_errors=True)
os.mkdir(cfg['config']['work_dir'])
call(["cp", "-r", cfg['config']['code_dir']+'/'+cfg['config']['code_dirname'], cfg['config']['work_dir']])

os.chdir(cfg['config']['code_dir']+'/'+cfg['config']['code_dirname'])

experiment_counter=0

nb_cores_per_cpu = str(cfg['parameters']['nb_cores_per_cpu'])

with open(cfg['config']['result_dir']+"/perf_compare_"+PREFIXMAKE+"_"+str(cfg['parameters']['max_items']).replace(' ','-')+"_r"+str(cfg['test']['random'])+"_"+STARTDATE+".csv", 'w') as csvfile:
    csvfile.write("test_id,algo,nbthreads,max_key,load,throughput\n")
    for max_key in str(cfg['parameters']['max_items']).split(' '):
        print("%s: starting tests with 2^%s tests"%(stringTime(),max_key))

        for load in str(cfg['parameters']['load']).split(' '):
            for algo in str(cfg['parameters']['algo']).split(' '):

                for nb_threads in str(cfg['parameters']['nb_threads']).split(' '):

                                print("config: %s %s %s %s"%(algo, str(cfg['parameters']['nb_threads']), max_key, load))

                                command_to_execute= cfg['config']['code_dir'] + cfg['config']['code_dirname'] + "/" + algo+" --N_THREADS="+nb_threads+ " --N_CORES_PER_CPU=" + nb_cores_per_cpu + " --MAX_KEY="+max_key+" --NLOOKUP="+load+ " --DURATION="+str(cfg['test']['duration'])

                                print(command_to_execute)
                                call(shlex.split(command_to_execute),stdout=DEVNULL,stderr=STDOUT)

                                result=check_output(shlex.split(command_to_execute), universal_newlines=True)

                                for line in result.split('\n'):
                                    if "throughput" in line:
                                        throughput=line.replace('\t',' ').split(' ')[1]

                                print("Throughtput: ", throughput)

                                csvfile.write("{0},{1},{2},{3},{4},{5}\n".format(experiment_counter, algo, nb_threads, max_key, load, throughput))

                                experiment_counter += 1
