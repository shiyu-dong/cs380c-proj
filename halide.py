import os, sys, string, re, Queue

# result type defined by users
user_result_type = 'undefined'

# depth in function call
depth = 0

# book keeping
global_var_list = []
local_var_list = []
func_list = []

class Var:
    def __init__(self, name):
        self.name = name
        self.lower = 0
        self.upper = 0

class Func:
    def __init__(self, name):
        self.name = name
        self.var_list = []
        self.exp = ''

#-----------------  Main  -----------------#
for line in sys.stdin:
    pline = line.rstrip('\n').lstrip(' ')
    sline = re.split(' |,|;', pline)

    # check if user defines result type
    if sline[0] == '#define' and sline[1] == 'RESULT_TYPE':
        user_result_type = sline[2]

    if sline[len(sline)-1] == '{':
        depth += 1
    elif sline[len(sline)-1] == '}':
        depth -= 1
        # if reach another function
        if depth == 0:
            for var in local_var_list:
                print var.name
            local_var_list = []
            for func in func_list:
                sys.stdout.write(func.name + ': ')
                for arg in func.var_list:
                    sys.stdout.write(arg + ' ')
                sys.stdout.write('\n')
            func_list = []

    # Vars declaration
    if 'Var' in sline:
        index = sline.index('Var')+1
        while index != len(sline):
            if sline[index] == '' or sline[index] == None:
                index += 1
            elif sline[index] == ';':
                break
            # determine if Var is local or global
            else:
                new_var = Var(sline[index])
                if depth == 0:
                    global_var_list.append(new_var)
                else:
                    local_var_list.append(new_var)
                index += 1
        continue

    # Func declaration
    if 'Func' in sline and depth >= 1:
        index = sline.index('Func')+1
        while index != len(sline):
            if sline[index] == '' or sline[index] == None:
                index += 1
            elif sline[index] == ';':
                break
            else:
                new_func = Func(sline[index])
                func_list.append(new_func)
                index += 1
        continue

    # Func definition
    if re.match('\w\(.*', pline) != None:
        [func_call, exp] = re.split('=', pline.replace(' ', ''))
        parameters = re.split('\(|,|\)', func_call)
        for func in func_list:
            if func.name == parameters[0]:
                func.var_list = parameters[1:len(parameters)-1]

    # Function invocation
    if re.match('.*\w\.realize\(', pline) != None:
        print line
