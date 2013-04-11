import os, sys, string, re, Queue

# result type defined by users
user_result_type = 'undefined'

# depth in function call
depth = 0;

# book keeping
global_var_list = [];
local_var_list = [];
func_list = []

class Var:
    def __init__(self, name, lower, upper):
        self.name = name
        self.lower = lower
        self.upper = upper


#-----------------  Main  -----------------#
for line in sys.stdin:
    sline = re.split(': | |,|;', line.rstrip('\n').lstrip(' '))

    # check if user defines result type
    if sline[0] == '#define' and sline[1] == 'RESULT_TYPE':
        user_result_type = sline[2]

    if sline[len(sline)-1] == '{':
        depth += 1
    elif sline[len(sline)-1] == '}':
        depth -= 1
        # if reach another function
        if depth == 0:
            print local_var_list
            local_var_list = []
            print func_list
            func_list = []

    # find Vars
    if 'Var' in sline:
        base_index = sline.index('Var')+1
        while base_index != len(sline):
            if sline[base_index] == '':
                base_index += 1
            # determine if Var is local or global
            else:
                if depth == 0:
                    global_var_list.append(sline[base_index])
                else:
                    local_var_list.append(sline[base_index])
                base_index += 1

    # find Func
    if 'Func' in sline and depth >= 1:
        base_index = sline.index('Func')+1
        while base_index != len(sline):
            if sline[base_index] == '':
                base_index += 1
            else:
                func_list.append(sline[base_index])
                base_index += 1
