import os, sys, string, re, Queue, copy

# depth in function call
depth = 0

# book keeping
global_var_list = {}
local_var_list = {}
local_rdom_list = {}
global_rdom_list = {}
image_list = []

func_list = {} # map func name to Func
rfunc_list = {} # map func name to Func
func_def_line = {} # map line number to func name
rfunc_def_line = {} # map line number to rfunc name

# for vector code generation
op_num = 0
pt_num = 0
result_type = ''
op_dict = {
    '+': 'add_',
    '-': 'sub_',
    '*': 'mul_',
    '/': 'div_',
    '&': 'and_',
    '|': 'or_',
    '^': 'xor_',
}

cfunc_list = ['cos', 'sin', 'tan', 'acos', 'asin', 'atan', 'atan2', 'cosh', 'sinh', 'tanh', 'exp',
        'log', 'log10', 'pow', 'sqrt', 'ceil', 'abs', 'floor',
        'min', 'max']

ifile = []
space = '    '

class Var:
    def __init__(self):
        self.lower = '0'
        self.upper = '0'
        self.step = '1'
        self.parallel = False

    def __ne__(self, other):
        return self.lower != other.lower or self.upper != other.upper and self.step != other.step and self.parallel != other.parallel

class Func:
    def __init__(self):
        self.var_list = []
        self.var_offset = []
        self.rvar_list = [] # regular variables used in reduction step
        self.var = {}
        self.it_var_list = []
        self.exp = ''
        self.vec = False
        self.root = False

    def __eq__(self, other):
        return self.var_list == other.var_list and self.var_offset == other.var_offset

class RFunc:
    def __init__(self):
        self.var_list = []
        self.exp = ''

class RDom:
    def __init__(self):
        self.dimensions = []

def get_exp(line):
    global op_num, pt_num, space
    number = re.match('[0-9\.]+', line)
    var = re.match('\w+', line)
    func = re.match('\w+\(.*?\)', line)
    if number != None:
        #print(str(op_num) + 'num: ' + line[:number.end()])
        if result_type == 'float':
          sys.stdout.write(space + '__m256 op' + str(op_num) + ' = _mm256_set1_ps(' + line[:number.end()] + ');\n')
        elif result_type == 'double':
          sys.stdout.write(space + '__m256d op' + str(op_num) + ' = _mm256_set1_pd(' + line[:number.end()] + ');\n')
        elif '.' not in line[:number.end()] or result_type == 'int':
          sys.stdout.write(space + '__m256i op' + str(op_num) + ' = _mm256_set1_epi32(' + line[:number.end()] + ');\n')
        op_num += 1
        return line[number.end():]

    elif func != None:
        op_list = []
        func_name = line[:var.end()]
        # Halide function:
        if func_name in func_list or func_name in image_list:
          # computer pointer
          sys.stdout.write(space + 'RESULT_TYPE* pt' + str(pt_num) + ' = ')
          sys.stdout.write(func_name + '.getP(')
          count = 1
          i = var.end()+1
          while count != 0:
            sys.stdout.write(line[i])
            if line[i] == '(':
              count += 1
            elif line[i] == ')':
              count -= 1
            i += 1
          sys.stdout.write(';\n')
          # load variable
          if result_type == 'int':
            sys.stdout.write(space + '__m256i op' + str(op_num) + ' = _mm256_load_si256(pt' + str(pt_num) + ');\n')
          elif result_type == 'float':
            sys.stdout.write(space + '__m256 op' + str(op_num) + ' = _mm256_load_ps(pt' + str(pt_num) + ');\n')
          elif result_type == 'double':
            sys.stdout.write(space + '__m256d op' + str(op_num) + ' = _mm256_load_pd(pt' + str(pt_num) + ');\n')
          pt_num += 1
          op_num += 1
          return line[i:]

        # C++ Function:
        elif func_name in cfunc_list:
            line = line[var.end()+1:]
            while(line[0] != ')'):
                line = get_exp(line)
                op_list.append(op_num - 1)
                if line[0] == ',':
                    line = line[1:]

            if result_type == 'int':
              sys.stdout.write(space + '__m256i op' + str(op_num) + ' = _mm256_')
              sys.stdout.write(func_name+'_')
              sys.stdout.write('si256(')
            elif result_type == 'float':
              sys.stdout.write(space + '__m256 op' + str(op_num) + ' = _mm256_')
              sys.stdout.write(func_name+'_')
              sys.stdout.write('ps(');
            elif result_type == 'double':
              sys.stdout.write(space + '__m256d op' + str(op_num) + ' = _mm256_')
              sys.stdout.write(func_name+'_')
              sys.stdout.write('pd(');

            i = 0
            for operand in op_list:
              i += 1
              sys.stdout.write('op' + str(operand))
              if i != len(op_list):
                  sys.stdout.write(', ')
            sys.stdout.write(');\n')
            op_num += 1
            return line[1:]
            
    elif var != None:
        #print(str(op_num) + 'var: ' + line[:var.end()])
        var_name = line[:var.end()]
        if var_name in local_var_list or var_name in global_var_name or result_type == 'int':
          sys.stdout.write(space + '__m256i op' + str(op_num) + ' = _mm256_set1_epi32(' + var_name + ');\n')
        elif result_type == 'float':
          sys.stdout.write(space + '__m256 op' + str(op_num) + ' = _mm256_set1_ps(' + var_name + ');\n')
        elif result_type == 'double':
          sys.stdout.write(space + '__m256d op' + str(op_num) + ' = _mm256_set1_pd(' + var_name + ');\n')
        op_num += 1
        return line[var.end():]

    elif line[0] == '(':
        line = get_exp(line[1:])
        op1 = op_num-1

        if line[0] == ')':
            line = line[1:]
            return line

        op = line[0]
        line = get_exp(line[1:])
        op2 = op_num-1

        #print(str(op_num) + op + ' ' + str(op1) + ' ' + str(op2))
        op_str = op_dict[op]
        if result_type == 'int':
          sys.stdout.write(space + '__m256i op' + str(op_num) + ' = _mm256_')
          sys.stdout.write(op_str)
          sys.stdout.write('si256(op' + str(op1) + ', op' + str(op2) + ');\n')
        elif result_type == 'float':
          sys.stdout.write(space + '__m256 op' + str(op_num) + ' = _mm256_')
          sys.stdout.write(op_str)
          sys.stdout.write('ps(op' + str(op1) + ', op' + str(op2)  + ');\n')
        elif result_type == 'double':
          sys.stdout.write(space + '__m256d op' + str(op_num) + ' = _mm256_')
          sys.stdout.write(op_str)
          sys.stdout.write('pd(op' + str(op1) + ', op' + str(op2)  + ');\n')

        op_num += 1
        return line[1:]

    return ''

def print_vec(line):
      global op_num, pt_num
      op_num = 0
      pt_num = 0
      [func_call, exp] = re.split('=', line.replace(' ', ''), 1)
      #print '@@' + exp
      get_exp(exp)

      # store result
      image_name = func_call[:func_call.index('(')]
      sys.stdout.write(space + 'RESULT_TYPE* pt' + str(pt_num) + ' = ')
      sys.stdout.write(image_name + '.getP(')
      count = 1
      i = re.match('\w+', line).end()+1
      while count != 0:
        sys.stdout.write(line[i])
        if line[i] == '(':
          count += 1
        elif line[i] == ')':
          count -= 1
        i += 1
      sys.stdout.write(';\n')

      if result_type == 'int':
        sys.stdout.write(space + '_mm256_store_si256(pt' + str(pt_num) + ', op' + str(op_num-1) + ');\n')
      elif result_type == 'float':
        sys.stdout.write(space + '_mm256_store_ps(pt' + str(pt_num) + ', op' + str(op_num-1) + ');\n')
      elif result_type == 'double':
        sys.stdout.write(space + '_mm256_store_pd(pt' + str(pt_num) + ',op' + str(op_num-1) + ');\n')
      pt_num += 1

def loop_coalesce():
    func_lines = []
    for line1 in func_def_line:
        func_lines.append(line1)
        for line2 in func_def_line:
            if line1 == line2 or line2 in func_lines:
                continue

            # check if there is a reduction in between
            func1 = func_def_line[line1]
            func2 = func_def_line[line2]

            if func_list[func1].root or func_list[func2].root:
                continue

            # check if any of the function has been removed already
            if ifile[line1] == 'REMOVED' or ifile[line2] == 'REMOVED':
                continue

            rf_inbetween = False

            for rf in rfunc_def_line:
                if (rfunc_def_line[rf] < line1 and rfunc_def_line[rf] > line2) or (rfunc_def_line[rf] < line2 and rfunc_def_line[rf] > line1):
                    rf_inbetween = True
                    break

            if not rf_inbetween and len(func_list[func1].it_var_list) == len(func_list[func2].it_var_list):
                i = 0
                merge = True
                while i < len(func_list[func1].it_var_list):
                    if func_list[func1].it_var_list[i] != func_list[func2].it_var_list[i]:
                        merge = False
                        break

                    arg = func_list[func1].it_var_list[i]

                    if arg not in func_list[func1].var and arg not in func_list[func2].var:
                        i += 1
                        continue
                    if arg in func_list[func1].var and arg not in func_list[func2].var:
                        merge = False
                        break
                    elif func_list[func1].var[arg] != func_list[func2].var[arg]:
                        merge = False
                        break

                    i += 1

                if merge:
                    if line1 > line2:
                        ifile[line1] = ifile[line2] + ifile[line1]
                        ifile[line2] = 'REMOVED'
                        return True
                    else:
                        ifile[line2] = ifile[line1] + ifile[line2]
                        ifile[line1] = 'REMOVED'
                        return True
    return False

def generate_code():
    ln = 0
    global space
    space = '    '

    # Loop Coalescing
    working = True
    while working:
        working = loop_coalesce()


    # Code Generation
    for line in ifile:
        pline = line.rstrip('\n').lstrip(' ')
        sline = re.split(' |,|;', pline)

        # if this is a reduction step of a function
        if ln in rfunc_def_line:
            func_name = rfunc_def_line[ln]

            # generate code for "for" loop
            # for(arg=0; arg<upper; arg++)

            # generate reduction loop
            # generate outer loops if there is any rdom variabels
            for arg in func_list[func_name].var_list:
                this_list = {}
                if arg in local_rdom_list:
                    this_list = local_rdom_list
                elif arg in global_rdom_list:
                    this_list = global_rdom_list
                else:
                    continue

                ch = ord('x') + len(this_list[arg].dimensions) - 1

                for dim in this_list[arg].dimensions:
                    sys.stdout.write(space+'for('+arg+'.'+chr(ch))
                    sys.stdout.write('=' + dim.lower + '; ')
                    sys.stdout.write(arg+'.'+chr(ch)+'<'+dim.upper+'; ')
                    sys.stdout.write(arg+'.'+chr(ch)+'+='+dim.step+') {\n')
                    space += '  '
                    ch -= 1

            # generate inner loops if there is any regular variabels
            for arg in rfunc_list[func_name].rvar_list:
                if arg in local_var_list or arg in global_var_list:
                    sys.stdout.write(space+'for(int '+arg)
                    if arg in local_var_list:
                        sys.stdout.write('='+local_var_list[arg].lower+'; ')
                        sys.stdout.write(arg+'<'+local_var_list[arg].upper+'; ')
                        sys.stdout.write(arg+'+='+local_var_list[arg].step+') {\n')
                    elif arg in global_var_list:
                        sys.stdout.write('='+glbcal_var_list[arg].lower+'; ')
                        sys.stdout.write(arg+'<'+global_var_list[arg].upper+'; ')
                        sys.stdout.write(arg+'+='+global_var_list[arg].step+') {\n')
                    space += '  '

            # print inner most expression
            sys.stdout.write(space + ifile[ln].lstrip(' ')),

            # print back brackets
            space = space[:len(space)-2] 
            for arg in func_list[func_name].var_list:
                if arg in local_rdom_list or arg in global_rdom_list:
                    i=0
                    this_list = {}
                    if arg in local_rdom_list:
                        this_list = local_rdom_list
                    else:
                        this_list = global_rdom_list
                    while i < len(this_list[arg].dimensions):
                        sys.stdout.write(space + '}\n')
                        space = space[:len(space)-2] 
                        i += 1
            for arg in rfunc_list[func_name].rvar_list:
                sys.stdout.write(space + '}\n')
                space = space[:len(space)-2] 

            space += '  '
            sys.stdout.write('\n')

        # if this is a regular step of a function
        elif ln in func_def_line:
            func_name = func_def_line[ln]

            # f.base = new RESULT_TYPE[SIZE*SIZE]
            #sys.stdout.write(space + func_name + '.base = new RESULT_TYPE[')
            sys.stdout.write(space + func_name + '.base = (RESULT_TYPE*)memalign(32, sizeof(RESULT_TYPE)*')
            count = 0
            no_arg = True
            for arg in func_list[func_name].var_list:
                count += 1
                if arg in local_var_list:
                    sys.stdout.write(local_var_list[arg].upper)
                    no_arg = False
                elif arg in global_var_list:
                    sys.stdout.write(global_var_list[arg].upper)
                    no_arg = False
                if count != len(func_list[func_name].var_list) and not func_list[func_name].var_list[count] in local_rdom_list and not func_list[func_name].var_list[count] in global_rdom_list and not no_arg:
                    sys.stdout.write('*')
            sys.stdout.write(');\n')

            # f.s0 = SIZE
            sys.stdout.write(space + func_name+'.s0 = ')
            i = 0
            found = False
            while not found:
                arg = func_list[func_name].var_list[i]
                i += 1
                if arg in local_var_list:
                    found = True
                    sys.stdout.write(local_var_list[arg].upper)
                    sys.stdout.write(';\n')
                elif arg in global_var_list:
                    found = True
                    sys.stdout.write(global_var_list[arg].upper)
                    sys.stdout.write(';\n')

            # f.s1 = SIZE
            found = False
            while not found and i < len(func_list[func_name].var_list):
                arg = func_list[func_name].var_list[i]
                i += 1
                if arg in local_var_list:
                    found = True
                    sys.stdout.write(space + func_name+'.s1 = ')
                    sys.stdout.write(local_var_list[arg].upper)
                    sys.stdout.write(';\n')
                elif arg in global_var_list:
                    found = True
                    sys.stdout.write(space + func_name+'.s1 = ')
                    sys.stdout.write(global_var_list[arg].upper)
                    sys.stdout.write(';\n')

            if ifile[ln] == 'REMOVED':
                ln += 1
                continue

            # generate code for "for" loop
            i = 0
            # for(arg=0; arg<upper; arg++)
            for arg in func_list[func_name].it_var_list:
                if arg in func_list[func_name].var:

                    if func_list[func_name].var[arg].parallel == True:
                        sys.stdout.write(space + '#pragma omp parallel for\n')

                    sys.stdout.write(space+'for(int '+arg)
                    if func_list[func_name].var_offset[i] == '':
                        sys.stdout.write('='+func_list[func_name].var[arg].lower+'; ')
                        sys.stdout.write(arg+'<'+func_list[func_name].var[arg].upper+'; ')
                        sys.stdout.write(arg+'+='+func_list[func_name].var[arg].step+') {\n')
                    else:
                        sys.stdout.write('='+func_list[func_name].var_offset[i]+'; ')
                        sys.stdout.write(arg+'<'+func_list[func_name].var[arg].upper)
                        sys.stdout.write('-('+func_list[func_name].var_offset[i]+'); ')
                        sys.stdout.write(arg+'+='+func_list[func_name].var[arg].step+') {\n')
                    space += '  '

                elif arg in local_var_list or arg in global_var_list:
                    sys.stdout.write(space+'for(int '+arg)
                    if func_list[func_name].var_offset[i] == '':
                        if arg in local_var_list:
                            sys.stdout.write('='+local_var_list[arg].lower+'; ')
                            sys.stdout.write(arg+'<'+local_var_list[arg].upper+'; ')
                            sys.stdout.write(arg+'+='+local_var_list[arg].step+') {\n')
                        elif arg in global_var_list:
                            sys.stdout.write('='+glbcal_var_list[arg].lower+'; ')
                            sys.stdout.write(arg+'<'+global_var_list[arg].upper+'; ')
                            sys.stdout.write(arg+'+='+global_var_list[arg].step+') {\n')
                    else:
                        if arg in local_var_list:
                            sys.stdout.write('='+func_list[func_name].var_offset[i]+'; ')
                            sys.stdout.write(arg+'<'+local_var_list[arg].upper)
                            sys.stdout.write('-('+func_list[func_name].var_offset[i]+'); ')
                            sys.stdout.write(arg+'+='+local_var_list[arg].step+') {\n')
                        elif arg in global_var_list:
                            sys.stdout.write('='+func_list[func_name].var_offset[i]+'; ')
                            sys.stdout.write(arg+'<'+global_var_list[arg].upper)
                            sys.stdout.write('-('+func_list[func_name].var_offset[i]+'); ')
                            sys.stdout.write(arg+'+='+global_var_list[arg].step+') {\n')
                    space += '  '
                i += 1

            # print inner most expression
            if not func_list[func_name].vec:
              sys.stdout.write(space + ifile[ln].lstrip(' '))
            else:
              print_vec(pline)

            # print back brackets
            space = space[:len(space)-2] 
            for arg in func_list[func_name].it_var_list:
                if arg in local_var_list or arg in global_var_list or arg in func_list[func_name].var:
                    sys.stdout.write(space + '}\n')
                    space = space[:len(space)-2]
            space += '  '
            sys.stdout.write('\n')

        # delete lines that calls realize
        elif re.match('.*\w+\.realize\(', pline) != None:
            index = line.rfind('.')
            line = line[:index] + ';\n'
            sys.stdout.write(line)

        # change lines for Func delcaration
        elif 'Func' in sline:
            [line, n] = re.subn('\(.*?\)', '', line)
            sys.stdout.write(line)

        elif not 'Var' in sline and re.match('\w+\.tile\(', pline) == None \
            and re.match('\w+\.parallel\(', pline) == None and re.match('\w+\.vectorize\(', pline) == None \
            and re.match('\w+\.root\(', pline) == None:
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
        # if reach end of function
        if depth == 0:
            for f in func_list:
                if len(func_list[f].it_var_list) == 0:
                    for arg in func_list[f].var_list:
                        func_list[f].it_var_list.append(arg)
                # change inner loop step
                if func_list[f].vec == True:
                    arg_name = func_list[f].it_var_list[-1]
                    if arg_name in func_list[f].var:
                      print arg_name
                      func_list[f].var[arg_name].step = '32/sizeof(RESULT_TYPE)'
                    else:
                      if arg_name in local_var_list:
                          this_list = local_var_list
                      elif arg_name in global_var_list:
                          this_list = global_var_list
                      this_list[arg_name].step = '32/sizeof(RESULT_TYPE)'

            generate_code()
            #for var in local_rdom_list:
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
            ifile = []
            op_num = 0
            pt_num = 0
            image_list = []

    # add Func define
    if '#define RESULT_TYPE' in line:
        result_type = line[len('#define RESULT_TYPE '):len(line)-1]
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
        index = pline.replace(' ', '').index('RDom')+4
        rdoms = re.split('(\w+\(.*?\)[,;])', pline.replace(' ','')[index:])

        for rdom in rdoms:
            if rdom == '':
                continue
            srdom = re.split('\(|,|\)|;', rdom)
            rdom_name = srdom[0]
            this_list = {}
            new_rdom = RDom()
            index = 1
            while index != len(srdom):
                new_var = Var();
                while srdom[index] == '':
                    index += 1
                new_var.lower = srdom[index]
                index += 1

                while srdom[index] == '':
                    index += 1
                new_var.upper = srdom[index]
                index += 1

                new_rdom.dimensions.insert(0, new_var)
                while srdom[index] == '':
                    index += 1
                    if index == len(srdom):
                        break

            if depth == 0:
                global_rdom_list[rdom_name] = new_rdom
            else:
                local_rdom_list[rdom_name] = new_rdom

    # Func declaration
    # Collect information about variables used and their offsets
    if 'Func' in sline and depth >= 1:
        index = pline.replace(' ','').index('Func')+4
        funcs = re.split('(\w+\(.*?\)[,;])', pline.replace(' ','')[index:])
        for func in funcs:
            if func == '':
                continue
            sfunc = re.split('\(|,|\)|;', func)
            func_name = sfunc[0]
            func_list[func_name] = Func()
            index = 1
            while index != len(sfunc):
                if sfunc[index] != '':
                    j = sfunc[index].find('[')
                    if j == -1:
                        func_list[func_name].var_list.insert(0, sfunc[index])
                        func_list[func_name].var_offset.insert(0, '')
                    else:
                        func_list[func_name].var_list.insert(0, sfunc[index][:j])
                        func_list[func_name].var_offset.insert(0, sfunc[index][j+1:sfunc[index].find(']')])
                index += 1

    # Func definition
    # Collect information about the expression of the function by remembering the line number of the expression
    if re.match('\w+\(.*\)\s*=', pline) != None:

        [func_call, exp] = re.split('=', pline.replace(' ', ''), 1)
        parameters = re.split('\(|,|\)', func_call)

        if parameters[0] in func_list:
            if func_list[parameters[0]].exp == '':
                func_list[parameters[0]].exp = exp
                func_def_line[len(ifile)-1] = parameters[0]
            else:
                rfunc_list[parameters[0]] = Func()
                rfunc_list[parameters[0]].exp = exp
                rfunc_def_line[len(ifile)-1] = parameters[0]
                i = 1
                while i != len(parameters):
                    # if this is a reduction function, remember which general variables it used in reduction step
                    if parameters[i] in local_var_list or parameters[i] in global_var_list:
                        rfunc_list[parameters[0]].rvar_list.append(parameters[i])
                    i += 1

    # Function invocation
    # Collect information about dimensions of iteration in each function
    if re.match('.*\w+\.realize\(', pline) != None:
        func_name = re.findall('\w+\.realize', pline.replace(' ', ''))
        func_name = func_name[0][:func_name[0].index('.')]

        index = pline.rfind('(')
        dimensions = pline[index+1:len(pline)-2].replace(' ', '')
        dimensions = re.split(',', dimensions)

        i = 0
        for num in dimensions:
            var_name = func_list[func_name].var_list[i]
            while var_name in local_rdom_list or var_name in global_rdom_list:
                i += 1
                var_name = func_list[func_name].var_list[i]

            if var_name in local_var_list:
                local_var_list[var_name].upper = num
            elif var_name in global_var_list:
                global_var_list[var_name].upper = num

            for f in func_list:
                if var_name+'Tile' in func_list[f].var:
                    func_list[f].var[var_name+'Tile'].upper = num
                elif var_name in func_list[f].var:
                    func_list[f].var[var_name].upper = num
            i+=1

    # Image Declaration
    if re.match('Image', pline) != None:
        pline = pline.replace(' ', '')
        pline = pline[pline.index('>')+1:]
        while len(pline) > 0:
          im_match = re.match('\w+\(.+?\)[,;]', pline)
          if im_match == None:
            break

          image_list.append(pline[:pline.index('(')])
          pline = pline[im_match.end():]

    # Parse optmizations
    if re.match('\w+\.tile\(', pline) != None:
        this_line = pline.replace(' ','')
        func_name = re.findall('\w+\.tile', this_line)
        func_name = func_name[0][:func_name[0].index('.')]

        if len(func_list[func_name].it_var_list) == 0:
            for arg in func_list[func_name].var_list:
                func_list[func_name].it_var_list.append(arg)

        args = this_line[this_line.index('(')+1:this_line.index(')')]
        args = re.split(',', args)
        i = 0
        while i != len(args):
            # args[i] original variable name
            # args[i+1] tile variable name
            # args[i+2] tile variable step size

            # offset changes
            orig_index = func_list[func_name].it_var_list.index(args[i])
            orig_offset = func_list[func_name].var_offset[orig_index]
            func_list[func_name].var_offset[orig_index] = ''

            # insert tile variable
            func_list[func_name].it_var_list.insert(0, args[i+1])
            func_list[func_name].var_offset.insert(0, orig_offset)

            func_list[func_name].var[args[i+1]] = Var()
            func_list[func_name].var[args[i+1]].step = args[i+2]

            func_list[func_name].var[args[i]] = Var()
            func_list[func_name].var[args[i]].lower = args[i+1] 
            func_list[func_name].var[args[i]].upper = args[i+1] + '+' + args[i+2]
            func_list[func_name].var[args[i]].step = '1'

            i+=3

    if re.match('\w+\.parallel\(', pline) != None:
        this_line = pline.replace(' ', '')
        tmp = re.findall('\w+\.parallel\(.*\)', this_line)
        func_name = tmp[0][:tmp[0].index('.')]
        arg_name = tmp[0][tmp[0].index('(')+1:tmp[0].index(')')]

        if len(func_list[func_name].it_var_list) == 0:
            for arg in func_list[func_name].var_list:
                func_list[func_name].it_var_list.append(arg)

        if arg_name not in func_list[func_name].var:
            this_list = {}
            if arg_name in local_var_list:
                this_list = local_var_list
            elif arg_name in global_var_list:
                this_list = global_var_list
            func_list[func_name].var[arg_name] = Var()
            func_list[func_name].var[arg_name].lower = this_list[arg_name].lower
            func_list[func_name].var[arg_name].upper = this_list[arg_name].upper
            func_list[func_name].var[arg_name].step = this_list[arg_name].step

        func_list[func_name].var[arg_name].parallel = True

    if re.match('\w+\.vectorize', pline) != None:
        this_line = pline.replace(' ', '')
        tmp = re.findall('\w+\.vectorize\(.*\)', this_line)
        func_name = tmp[0][:tmp[0].index('.')]
        func_list[func_name].vec = True

    if re.match('\w+\.root', pline) != None:
        this_line = pline.replace(' ', '')
        tmp = re.findall('\w+\.root\(.*\)', this_line)
        func_name = tmp[0][:tmp[0].index('.')]
        func_list[func_name].root = True

