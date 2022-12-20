import argparse
import sys
from typing import OrderedDict
import xml.etree.ElementTree as ET
import re
from xmlrpc.client import MAXINT

#Error codes
undefl_redefvar = 52
badtype = 53
nonexvar = 54
nonexframe = 55
novalue = 56
badvalue = 57
strerr = 58

#var must be a dictionary!
class Frame:
    def __init__(self):
        self.variables = {}
    #add or update item
    def set_var(self, var):
        self.variables.update({var.name : var})
    #get variable value
    def get_var(self, key):
        return self.variables.get(key)
    
    def get_all(self):
        return ["name: " + str(a.name) + "\ntype: " + str(a.type) + "\nvalue: " + str(a.value) for a in self.variables.values()]

class Variable:
    name = None
    type = None
    value = None

def get_from_frame(this):
    global global_frame
    global local_frame
    global temp_frame
    tmp = None
    
    if (this.text[0:2] == "GF"):
        tmp = global_frame.get_var(this.text[3:])
    elif (this.text[0:2] == "LF"):
        if (local_frame == None):
            exit(nonexframe)
        tmp = local_frame.get_var(this.text[3:])
    elif (this.text[0:2] == "TF"):
        if (temp_frame == None):
            exit(nonexframe)
        tmp = temp_frame.get_var(this.text[3:])
    
    return tmp

def set_to_frame(to, tmp):
    global global_frame
    global local_frame
    global temp_frame
    
    if (to.text[0:2] == "GF"):
        global_frame.set_var(tmp)
    elif (to.text[0:2] == "LF"):
        if (local_frame == None):
            exit(nonexframe)
        local_frame.set_var(tmp)
    elif (to.text[0:2] == "TF"):
        if (temp_frame == None):
            exit(nonexframe)
        temp_frame.set_var(tmp)

def make_symb(arg):
    tmp = Variable()
    if (arg.attrib["type"] == "var"):
        tmp = get_from_frame(arg)
    else:
        tmp.type = arg.attrib["type"]
        if (tmp.type == "string"):
            if (arg.text == None):
                tmp.value = ""
            else:
                tmp.value = str(arg.text)
        elif (tmp.type == "int"):
            tmp.value = int(str(arg.text))
        elif (tmp.type == "bool"):
            if (arg.text.upper() == "TRUE"):
                tmp.value = True
            elif (arg.text.upper() == "FALSE"):
                tmp.value = False
        elif (tmp.type == "nil"):
            tmp.value = None
    
    return tmp

def MOVE(to, _from):
    tmp = make_symb(_from)
    tmp.name = to.text[3:]
    
    if (tmp == None or get_from_frame(to) == None):
        exit(nonexvar)
    
    set_to_frame(to, tmp)

def DEFVAR(this):
    var = Variable()
    var.name = this.text[3:]
    var.type = "nil"
    
    if (get_from_frame(this) != None):
        exit(undefl_redefvar)
    
    set_to_frame(this, var)

def PUSHS(this):
    tmp = make_symb(this)
    
    if (tmp == None):
        exit(nonexvar)
    
    data_stack.append(str(tmp.type) + "@" + str(tmp.value))

def POPS(to):
    try:
        const_str = data_stack.pop(-1)
    except:
        exit(novalue)
    
    if (get_from_frame(to) == None):
        exit(nonexvar)
    
    var = Variable()
    var.name = to.text[3:]
    var.type = const_str[ : const_str.find("@")]
    
    if (var.type == "string"):
        var.value = const_str[const_str.find("@") + 1 : ]
    elif (var.type == "int"):
        var.value = int(const_str[const_str.find("@") + 1 : ])
    elif (var.type == "bool"):
        if (const_str[const_str.find("@") + 1 : ].upper() == "TRUE"):
            var.value = True
        elif (const_str[const_str.find("@") + 1 : ].upper() == "FALSE"):
            var.value = False
    elif (var.type == "nil"):
        var.value = None
    
    set_to_frame(to, var)

def ARITHMETIC(result, argA, argB, op):
    tmpA = make_symb(argA)
    tmpB = make_symb(argB)
    
    if (get_from_frame(result) == None or tmpA == None or tmpB == None):
        exit(nonexvar)
    
    if (tmpA.type != "int" or tmpB.type != "int"):
        exit(badtype)
    
    tmp_res = Variable()
    tmp_res.name = result.text[3:]
    tmp_res.type = "int"
    
    if (op == "ADD"):
        tmp_res.value = tmpA.value + tmpB.value
    elif (op == "SUB"):
        tmp_res.value = tmpA.value - tmpB.value
    elif (op == "MUL"):
        tmp_res.value = tmpA.value * tmpB.value
    elif (op == "IDIV"):
        if (tmpB.value == 0):
            exit(badvalue)
        tmp_res.value = tmpA.value // tmpB.value
    
    set_to_frame(result, tmp_res)

def RELATION(result, argA, argB, op):
    tmpA = make_symb(argA)
    tmpB = make_symb(argB)
    
    if (get_from_frame(result) == None or tmpA == None or tmpB == None):
        exit(nonexvar)
    
    tmp_res = Variable()
    tmp_res.name = result.text[3:]
    tmp_res.type = "bool"
    
    if (tmpA.type == "nil" or tmpB.type == "nil"):
        if (op == "EQ"):
            tmp_res.value = tmpA.value == tmpB.value
        else:
            exit(badtype)
    else:
        if (op == "LT"):
            tmp_res.value = tmpA.value < tmpB.value
        elif (op == "GT"):
            tmp_res.value = tmpA.value > tmpB.value
    
    set_to_frame(result, tmp_res)

def LOGIC(result, argA, argB, op):
    tmpA = make_symb(argA)
    tmpB = make_symb(argB)
    
    if (get_from_frame(result) == None or tmpA == None or tmpB == None):
        exit(nonexvar)
    
    tmp_res = Variable()
    tmp_res.name = result.text[3:]
    tmp_res.type = "bool"
    
    if (tmpA.type != "bool" or tmpB.type != "bool"):
        exit(badtype)
    
    if (op == "AND"):
        tmp_res.value = tmpA.value and tmpB.value
    elif (op == "OR"):
        tmp_res.value = tmpA.value or tmpB.value
    
    set_to_frame(result, tmp_res)

def NOT(result, argA):
    tmpA = make_symb(argA)
    
    if (get_from_frame(result) == None or tmpA == None):
        exit(nonexvar)
    
    if (tmpA.type != "bool"):
        exit(badtype)
    
    tmp_res = Variable()
    tmp_res.name = result.text[3:]
    tmp_res.type = "bool"
    
    tmp_res.value = not tmpA.value
    
    set_to_frame(result, tmp_res)

def INT2CHAR(result, argA):
    tmpA = make_symb(argA)
    
    if (get_from_frame(result) == None or tmpA == None):
        exit(nonexvar)
    
    tmp_res = Variable()
    tmp_res.name = result.text[3:]
    tmp_res.type = "string"
    
    if (tmpA.type != "int"):
        exit(badtype)
    
    try:
        tmp_res.value = chr(tmpA.value)
    except:
        exit(strerr)
    
    set_to_frame(result, tmp_res)

def STRI2INT(result, argA, argB):
    tmpA = make_symb(argA)
    tmpB = make_symb(argB)
    
    if (get_from_frame(result) == None or tmpA == None or tmpB == None):
        exit(nonexvar)
    
    if (tmpA.type != "string" or tmpB.type != "int"):
        exit(badtype)
    
    if (tmpB.value < 0):
        exit(badvalue)
    
    tmp_res = Variable()
    tmp_res.name = result.text[3:]
    tmp_res.type = "int"
    
    try:
        tmp_res.value = ord(tmpA.value[tmpB.value])
    except:
        exit(strerr)
    
    set_to_frame(result, tmp_res)

def READ(result, type):
    global input_args
    
    tmp_res = Variable()
    tmp_res.name = result.text[3:]
    tmp_res.type = type.text
    
    if (get_from_frame(result) == None):
        exit(nonexvar)
    
    if (type.text == "int"):
        try:
            if (args.input_path != sys.stdin):
                tmp_res.value = int(input_args.pop())
            else:
                tmp_res.value = int(input())
        except:
            exit(novalue)
    elif (type.text == "string"):
        try:
            if (args.input_path != sys.stdin):
                tmp_res.value = str(input_args.pop())
            else:
                tmp_res.value = str(input())
        except:
            exit(novalue)
    elif (type.text == "bool"):
        try:
            if (args.input_path != sys.stdin):
                tmp_res.value = str(input_args.pop())
            else:
                tmp_res.value = str(input())
        except:
            exit(novalue)
        if (tmp_res.value.upper() == "TRUE"):
            tmp_res.value = True
        else:
            tmp_res.value = False
    else:
        exit(badtype)
    
    set_to_frame(result, tmp_res)

def WRITE(this):
    tmp = make_symb(this)
    
    if (tmp == None):
        exit(nonexvar)
    
    if (tmp.type == "bool"):
        if (tmp.value):
            print("TRUE", end='')
        else:
            print("FALSE", end='')
    elif (tmp.type != "string"):
        print(tmp.value, end='')
    else:
        reg = re.findall("\\\\[0-9].{2}", tmp.value)
        char_esc_sqc = [chr(int(elem[2:])) for elem in reg]
        escaped_str = tmp.value
        for char in char_esc_sqc:
            escaped_str = re.sub("\\\\[0-9].{2}", char, escaped_str, 1)
        print(escaped_str, end='')

def CONCAT(result, argA, argB):
    tmpA = make_symb(argA)
    tmpB = make_symb(argB)
    
    if (get_from_frame(result) == None or tmpA == None or tmpB == None):
        exit(nonexvar)
    
    if (tmpA.type != "string" or tmpB.type != "string"):
        exit(badtype)
    
    tmp_res = Variable()
    tmp_res.name = result.text[3:]
    tmp_res.type = "string"
    tmp_res.value = tmpA.value + tmpB.value
    
    set_to_frame(result, tmp_res)

def STRLEN(result, argA):
    tmpA = make_symb(argA)
    
    if (get_from_frame(result) == None or tmpA == None):
        exit(nonexvar)
    
    if (tmpA.type != "string"):
        exit(badtype)
    
    tmp_res = Variable()
    tmp_res.name = result.text[3:]
    tmp_res.type = "int"
    tmp_res.value = int(len(tmpA.value))
    
    set_to_frame(result, tmp_res)

def GETCHAR(result, argA, argB):
    tmpA = make_symb(argA)
    tmpB = make_symb(argB)
    
    if (get_from_frame(result) == None or tmpA == None or tmpB == None):
        exit(nonexvar)
    
    if (tmpA.type != "string" or tmpB.type != "int"):
        exit(badtype)
    
    if (tmpB.value < 0):
        exit(badvalue)
    
    tmp_res = Variable()
    tmp_res.name = result.text[3:]
    tmp_res.type = "string"
    try:
        tmp_res.value = tmpA.value[tmpB.value]
    except:
        exit(strerr)
    
    set_to_frame(result, tmp_res)

def SETCHAR(result, argA, argB):
    tmpA = make_symb(argA)
    tmpB = make_symb(argB)
    tmp_res = make_symb(result)
    
    if (tmp_res == None or tmpA == None or tmpB == None):
        exit(nonexvar)
    
    if (tmpA.type != "int" or tmp_res.type != "string" or tmpB.type != "string"):
        exit(badtype)
    
    if (tmpA.value < 0):
        exit(badvalue)
    
    if (tmpB.value == ""):
        exit(strerr)
    
    if (tmpA.value < 0 or tmpA.value + 1 > len(tmp_res.value)):
        exit(strerr)
    else:
        tmp_res.value = tmp_res.value[:tmpA.value] + tmpB.value[0] + tmp_res.value[tmpA.value + 1:]
    
    set_to_frame(result, tmp_res)

def TYPE(result, argA):
    tmpA = make_symb(argA)
    
    if (get_from_frame(result) == None or tmpA == None):
        exit(nonexvar)
    
    tmp_res = Variable()
    tmp_res.name = result.text[3:]
    tmp_res.type = "string"
    
    if (tmpA.type == None):
        tmp_res.value = ""
    else:
        tmp_res.value = tmpA.type
    
    set_to_frame(result, tmp_res)

def LABEL(arg):
    global iteration
    
    label = arg.text
    if (label in labels_dict.keys()):
        exit(undefl_redefvar)
    
    pair = { label : iteration }
    labels_dict.update(pair)

def JUMP(arg):
    label = arg.text
    if (label not in labels_dict.keys()):
        exit(undefl_redefvar)
    
    global iteration
    iteration = labels_dict[label]

def JUMPIF(to, argA, argB, ifn):
    label = to.text
    if (label not in labels_dict.keys()):
        exit(undefl_redefvar)
    
    tmpA = make_symb(argA)
    tmpB = make_symb(argB)
    
    if (tmpA == None or tmpB == None):
        exit(nonexvar)
    
    if (tmpA.type != tmpB.type and not (tmpA.type == "nil" or tmpB.type == "nil")):
        exit(badtype)
    
    if ((tmpA.value == tmpB.value and ifn == 0) or (tmpA.value != tmpB.value and ifn)):
        global iteration
        iteration = labels_dict[label]

def EXIT(arg):
    symb = make_symb(arg)
    
    if (symb.type != "int"):
        exit(badtype)
    
    if (symb.value < 0 and symb.value > 49):
        exit(badvalue)
    
    exit(symb.value)

def DPRINT(arg):
    symb = make_symb(arg)
    
    if (symb == None):
        exit(nonexvar)
    
    print(symb.value, file=sys.stderr)

def BREAK():
    global iteration
    
    line = root[iteration].attrib["order"]
    
    print("\n======================================================", file=sys.stderr)
    print("Executing now instruction on " + str(line) + " line", file=sys.stderr)
    print("Total instructions executed: " + str(iteration + 1), file=sys.stderr)
    print("Global frame content: ", file=sys.stderr)
    for var in global_frame.get_all():
        print(var, file=sys.stderr)
    
    print("Local frame content: ", file=sys.stderr)
    if (local_frame != None):
        for var in local_frame.get_all():
            print(var, file=sys.stderr)
    else:
        print("empty", file=sys.stderr)
    
    print("Temporary frame content: ", file=sys.stderr)
    if (temp_frame != None):
        for var in temp_frame.get_all():
            print(var, file=sys.stderr)
    else:
        print("empty", file=sys.stderr)
    print("======================================================", file=sys.stderr)

def CREATEFRAME():
    global temp_frame
    temp_frame = Frame()

def PUSHFRAME():
    global frame_stack
    global temp_frame
    global local_frame
    
    if (temp_frame == None):
        exit(nonexframe)
    
    frame_stack.append(temp_frame)
    temp_frame = None
    local_frame = frame_stack[-1]

def POPFRAME():
    global frame_stack
    global temp_frame
    global local_frame
    
    if (local_frame == None):
        exit(nonexframe)
    
    temp_frame = frame_stack.pop(-1)
    try:
        local_frame = frame_stack[-1]
    except:
        local_frame = None

def CALL(arg):
    global iteration
    global call_stack
    
    call_stack.append(iteration)
    JUMP(arg)

def RETURN():
    global iteration
    global call_stack
    
    try:
        iteration = call_stack.pop(-1)
    except:
        exit(novalue)

#Stacks
data_stack = []
frame_stack = []
call_stack = []

#Frames
global_frame = Frame()
local_frame = None
temp_frame = None

#Labels
# { name : position }
labels_dict = {}

#Parse arguments
parser = argparse.ArgumentParser(add_help=False)
parser.add_argument('--help', action='store_true', dest='help', default=False, help='Show this info')
parser.add_argument('--source', dest='source_path', default=sys.stdin, help='input file with XML representation of the IPPcode22')
parser.add_argument('--input', dest='input_path', default=sys.stdin, help='file with input')

args = parser.parse_args()

if (args.help):
    if (args.source_path != sys.stdin or args.input_path != sys.stdin):
        exit(10)
    print("\nusage: Interpret.py [--help] [--source SOURCE_PATH] [--input INPUT_PATH]\n\
\n\
optional arguments:\n\
  --help                show this info\n\
  --source SOURCE_PATH  input file with XML representation of the IPPcode22\n\
  --input INPUT_PATH    file with input\n")
    exit(0)

if (args.source_path == sys.stdin and args.input_path == sys.stdin):
    exit(10)

input_args = []
if (args.input_path != sys.stdin):
    try:
        with open(args.input_path, 'r') as f:
            input_args = f.readline().split()
            input_args.reverse()
    except:
        exit(11)

#XML Tree
try:
    tree = ET.parse(args.source_path)
except:
    exit(31)
root = tree.getroot()

#Sort instructions in order
s_instr = []
while len(s_instr) != len(root):
    min = MAXINT
    for child in root:
        if child in s_instr:
            continue
        if int(child.attrib["order"]) < min:
            min = int(child.attrib["order"])
            tmp = child
    s_instr.append(tmp)
root = s_instr

#Labels processing
iteration = 0
order_list = []
while iteration < len(root):
    child = root[iteration]
    
    if (child.tag == "instruction" and child.attrib["opcode"].upper() == "LABEL"):
        LABEL(child[0])
    
    if (child.tag != "instruction"):
        exit(32)
    
    if (child.attrib["order"] in order_list):
        exit(32)
    
    try:
        if (int(child.attrib["order"]) <= order_list[-1]):
            exit(32)
    except:
        pass
    
    order_list.append(child.attrib["order"])
    iteration += 1

#Regular OP code processing
iteration = 0
while iteration < len(root):
    child = root[iteration]
    
    if (child.tag == "instruction" and child.attrib["opcode"].upper() == "DEFVAR"):
        DEFVAR(child[0])
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "MOVE"):
        MOVE(child[0], child[1])
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "PUSHS"):
        PUSHS(child[0])
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "POPS"):
        POPS(child[0])
    #Arithmetic functions
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "ADD"):
        ARITHMETIC(child[0], child[1], child[2], "ADD")
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "SUB"):
        ARITHMETIC(child[0], child[1], child[2], "SUB")
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "MUL"):
        ARITHMETIC(child[0], child[1], child[2], "MUL")
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "IDIV"):
        ARITHMETIC(child[0], child[1], child[2], "IDIV")
    #Relations
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "LT"):
        RELATION(child[0], child[1], child[2], "LT")
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "GT"):
        RELATION(child[0], child[1], child[2], "GT")
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "EQ"):
        RELATION(child[0], child[1], child[2], "EQ")
    #Logic
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "AND"):
        LOGIC(child[0], child[1], child[2], "AND")
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "OR"):
        LOGIC(child[0], child[1], child[2], "OR")
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "NOT"):
        NOT(child[0], child[1])
    #Converters
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "INT2CHAR"):
        INT2CHAR(child[0], child[1])
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "STRI2INT"):
        STRI2INT(child[0], child[1], child[2])
    #IO
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "READ"):
        READ(child[0], child[1])
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "WRITE"):
        WRITE(child[0])
    #String processing
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "CONCAT"):
        CONCAT(child[0], child[1], child[2])
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "STRLEN"):
        STRLEN(child[0], child[1])
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "GETCHAR"):
        GETCHAR(child[0], child[1], child[2])
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "SETCHAR"):
        SETCHAR(child[0], child[1], child[2])
    #Type processing
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "TYPE"):
        TYPE(child[0], child[1])
    #Jumps
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "JUMP"):
        JUMP(child[0])
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "JUMPIFEQ"):
        JUMPIF(child[0], child[1], child[2], 0)
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "JUMPIFNEQ"):
        JUMPIF(child[0], child[1], child[2], 1)
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "EXIT"):
        EXIT(child[0])
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "DPRINT"):
        DPRINT(child[0])
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "BREAK"):
        BREAK()
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "CREATEFRAME"):
        CREATEFRAME()
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "PUSHFRAME"):
        PUSHFRAME()
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "POPFRAME"):
        POPFRAME()
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "CALL"):
        CALL(child[0])
    elif (child.tag == "instruction" and child.attrib["opcode"].upper() == "RETURN"):
        RETURN()
    
    iteration += 1
