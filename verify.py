#!/usr/bin/python3
import re

trace = ['astar', 'bzip2', 'mcf', 'perlbench']


for output in trace:
    print('##############################################################')
    print('Processing ' + output)
    proc = int(1)
    with open('outputs/' + output + '.out') as ref, open(output + '.log') as act:       
        for ref_line, act_line in zip(ref, act):
            ref_line = ref_line.strip()
            act_line = act_line.strip()
            if ref_line != act_line:
                print('\tMismatch @ line ' + str(proc))
                print('\t\tREF: ' + ref_line)
                print('\t\tACT: ' + act_line)
                #exit()
            proc += 1
    print('Line processed: ' + str(proc))
    print('##############################################################\n')