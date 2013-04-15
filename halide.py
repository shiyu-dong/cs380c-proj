import os, sys, string, re, Queue

# depth in function call
depth = 0

# book keeping
global_var_list = {}
local_var_list = {}
local_rdom_list = {}
global_rdom_list = {}

func_list = {}
rfunc_list = {}
func_def_line = {}
rfunc_def_line = {}

ifile = []

class Var:
    def __init__(self):
        self.lower = 0
        self.upper = 0

class Func:
    def __init__(self):
        self.var_list = []
        self.exp = ''

class RFunc:
    def __init__(self):
        self.var_list = []
        self.exp = ''

class RDom:
    def __init__(self):
        self.lower = 0
        self.upper = 0

def generate_code():
    ln = 0
    space = '    '

    for line in ifile:
        pline = line.rstrip('\n').lstrip(' ')
        sline = re.split(' |,|;', pline)

        if ln in func_def_line:
            func_name = func_def_line[ln]

            # f.base = new RESULT_TYPE[SIZE*SIZE]
            sys.stdout.write(space + func_name + '.base = new RESULT_TYPE[')
            count = 0;
            for arg in func_list[func_name].var_list:
                count += 1
                if arg in local_var_list:
                    sys.stdout.write(str(local_var_list[arg].upper))
                if arg in global_var_list:
                    sys.stdout.write(str(global_var_list[arg].upper))
                if (count != len(func_list[func_name].var_list)):
                    sys.stdout.write('*')
            sys.stdout.write('];\n')

            # f.s0 = SIZE
            # TODO: assume 2D image
            sys.stdout.write(space + func_name+'.s0 = ')
            arg = func_list[func_name].var_list[0]
            if arg in local_var_list:
                sys.stdout.write(str(local_var_list[arg].upper))
            if arg in global_var_list:
                sys.stdout.write(str(global_var_list[arg].upper))
            sys.stdout.write(';\n\n')

            # generate code for "for" loop
            # for(arg=0; arg<upper; arg++)
            for arg in func_list[func_name].var_list:
                sys.stdout.write(space+'for(unsigned int '+arg)
                if arg in local_var_list:
                    sys.stdout.write('='+str(local_var_list[arg].lower)+'; ')
                    sys.stdout.write(arg+'<'+str(local_var_list[arg].upper)+'; ')
                elif arg in global_var_list:
                    sys.stdout.write('='+str(glbcal_var_list[arg].lower)+'; ')
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
            sys.stdout.write('\n')

        # delete lines that calls realize
        elif re.match('.*\w+\.realize\(', pline) != None:
            index = line.rfind('.')
            line = line[:index] + ';\n'
            sys.stdout.write(line)


        # delete lines for Var declaraion
        elif not 'Var' in sline:
            sys.stdout.write(line)
        ln += 1

#-----------------  Main  -----------------#
## Parse Input
for line in sys.stdin:
    ifile.append(line)
    pline = line.rstrip('\n').lstrip(' ')
    sline = re.split(' |,|;', pline)

    # check code block boundary
    if re.match('.*{.*', pline) != None:
        depth += 1
    if re.match('.*}.*', pline) != None:
        depth -= 1
        # if reach another function
        if depth == 0:
            generate_code()
            #for var in local_var_list:
            #    print var, local_var_list[var].upper
            local_var_list = {}
            local_rdom_list = {}
            #for func in func_list:
            #    sys.stdout.write(func + ' = ' + func_list[func].exp + ' --> ')
            #    for arg in func_list[func].var_list:
            #        sys.stdout.write(arg + ' ')
            #    sys.stdout.write('\n')
            func_list = {}
            func_def_line = {}
            rfunc_list = {}
            rfunc_def_line = {}

    # add Func define
    if '#define RESULT_TYPE' in line:
        ifile.append('#define Func Image<RESULT_TYPE>\n')

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

    # RDom declaration
    if 'RDom' in sline:
        index = pline.index('RDom')+4
        pline = pline.replace(' ', '')
        rdoms = re.split('\(|\)|\,', pline[index:])

        index = 0
        while(index+1 < len(rdoms)):
            #FIXME: RDom range expression cannot use '(|)'
            while rdoms[index] == '' or rdoms[index] == ';':
                index += 1
            rdom_name = rdoms[index]
            index += 1
            new_rdom = RDom()
            while rdoms[index] == '' or rdoms[index] == ';':
                index += 1
            new_rdom.lower = rdoms[index]
            index += 1
            while rdoms[index] == '' or rdoms[index] == ';':
                index += 1
            new_rdom.upper = rdoms[index]

            if depth == 0:
                global_rdom_list[rdom_name] = new_rdom
            else:
                local_rdom_list[rdom_name] = new_rdom
            index += 1

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
    if re.match('\w+\(.*\)\s*=', pline) != None:
        [func_call, exp] = re.split('=', pline.replace(' ', ''))
        parameters = re.split('\(|,|\)', func_call)
        if parameters[0] in func_list:
            if func_list[parameters[0]].exp == '':
                func_list[parameters[0]].var_list = parameters[1:len(parameters)-1]
                func_list[parameters[0]].exp = exp
                func_def_line[len(ifile)-1] = parameters[0]
            else:
                rfunc_list[parameters[0]].var_list = parameters[1:len(parameters)-1]
                rfunc_list[parameters[0]].exp = exp
                rfunc_def_line[len(ifile)-1] = parameters[0]
            # TODO: boundary inference from exp

    # Function invocation
    if re.match('.*\w+\.realize\(', pline) != None:
        func_name = re.findall('\w+\.realize', pline.replace(' ', ''))
        func_name = func_name[0][:func_name[0].index('.')]

        index = pline.rfind('(')
        dimensions = pline[index+1:len(pline)-2].replace(' ', '')
        dimensions = re.split(',', dimensions)

        i = 0
        for num in dimensions:
            var_name = func_list[func_name].var_list[i]
            # TODO: check if num is larger than upper..?
            if var_name in local_var_list:
                local_var_list[var_name].upper = int(num)
            elif var_name in global_var_list:
                global_var_list[var_name].upper = int(num)
            i+=1
