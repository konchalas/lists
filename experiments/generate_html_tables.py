import csv
import HTML

experiment_columns = 15
list1 = []
list2 = []
exp_counter = 0;
with open('results/perf_compare_test_10_r1_2018-05-17-13:15:27.csv') as csvfile:
    reader = csv.DictReader(csvfile)
    for row in reader:
        if(int(row['test_id']) % experiment_columns == 0):
            exp_counter += 1
            list1 = []
            list2 = []
            print('<h1>{0}. {1} load, {2} keys</h1>'.format(exp_counter, row['load'], row['max_key']))

        if (row['algo'] == 'algo_w_flush'):
            algo1 = 'algo_w_flush'
            list1.append(row['throughput'])

        if (row['algo'] == 'algo_wo_flush'):
            algo2 = 'algo_wo_flush'
            list2.append(row['throughput'])

        if ((int(row['test_id']) + 1) % (experiment_columns) == 0):
            table_data = ( list(zip([1, 2, 4, 8, 16], list1, list2)))
            htmlcode = HTML.table(table_data, header_row=['nr_threads', algo1, algo2])
            print (htmlcode)

        
