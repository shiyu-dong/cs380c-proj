import os, sys, string, re, Queue

# depth in function call
depth = 0

# book keeping
global_var_list = {}
local_var_list = {}
func_list = {}
func_def_line = {}

ifile = []

class Var:
    def __init__(self):
        self.lower = 0
        self.upper = 0

class Func:
    def __init__(self):
        self.var_list = []
        self.exp = ''

def generate_code():
    ln = 0
    space = '    '

    for line in ifile:
        pline = line.rstrip('\n').lstrip(' ')
        sline = re.split(' |,|;', pline)

        # generate code for "for" loop
        if ln in func_def_line:
            func_name = func_def_line[ln]
            # for(arg=0; arg<upper; arg++)
            for arg in func_list[func_name].var_list:
                sys.stdout.write(space+'for(unsigned int '+arg+'=0; ')
                if arg in local_var_list:
                    sys.stdout.write(arg+'<'+str(local_var_list[arg].upper)+'; ')
                elif arg in global_var_list:
                    sys.stdout.write(arg+'<'+str(global_var_list[arg].upper)+'; ')
                sys.stdout.write(arg+'++) {\n')
                space += '  '

            # print inner most expression
            sys.stdout.write(space + ifile[ln].lstrip(' ')),

            # print back brackets
            space = space[:len(space)-2] 
            for arg in func_list[func_name].var_list:
                sys.stdout.write(space + '}\n')
                space = space[:len(space)-2] 
            space += '  '

        # delete lines for Var declaraion
        elif not 'Var' in sline:
            print line,
        ln += 1


#-----------------  Main  -----------------#
## Parse Input
for line in sys.stdin:
    ifile.append(line)
    pline = line.rstrip('\n').lstrip(' ')
    sline = re.split(' |,|;', pline)

    # check code block boundary
    if sline[len(sline)-1] == '{':
        depth += 1
    elif sline[len(sline)-1] == '}':
        depth -= 1
        # if reach another function
        if depth == 0:
            generate_code()
            for var in local_var_list:
                print var, local_var_list[var].upper
            local_var_list = {}
            for func in func_list:
                sys.stdout.write(func + ' = ' + func_list[func].exp + ' --> ')
                for arg in func_list[func].var_list:
                    sys.stdout.write(arg + ' ')
                sys.stdout.write('\n')
            func_list = {}
            func_def_line = {}

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
                if depth == 0:
                    global_var_list[sline[index]] = Var()
                else:
                    local_var_list[sline[index]] = Var()
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
                func_list[sline[index]] = Func()
                index += 1
        continue

    # Func definition
    if re.match('\w\(.*', pline) != None:
        [func_call, exp] = re.split('=', pline.replace(' ', ''))
        parameters = re.split('\(|,|\)', func_call)
        func_list[parameters[0]].var_list = parameters[1:len(parameters)-1]
        func_list[parameters[0]].exp = exp
        func_def_line[len(ifile)-1] = parameters[0]
        # TODO: boundary inference from exp

    # Function invocation
    if re.match('.*\w\.realize\(', pline) != None:
        func_name = re.findall('\w\.realize', pline.replace(' ', ''))
        func_name = func_name[0][:func_name[0].index('.')]

        index = pline.rfind('(')
        dimensions = pline[index+1:len(pline)-2].replace(' ', '')
        dimensions = re.split(',', dimensions)

        i = 0
        for num in dimensions:
            var_name = func_list[func_name].var_list[i]
            if var_name in local_var_list:
                local_var_list[var_name].upper += int(num)
            elif var_name in global_var_list:
                global_var_list[var_name].upper += int(num)
            i+=1
