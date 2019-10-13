#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cerrno>
#include <string.h>
#include <list>
#include<tuple>
#include <vector>
#include <array>
#include <stack>
#include <climits>
#include <unordered_map>

using namespace std;

enum State {
    s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14,
    s15, s16, s17, s18, s19, s20, s21, s22, s23, s24, s25, s26, s27,
    s28, s29, s30, s31, s32, s33, s34, s35, s36, s37, sComma, sErr
};

enum Category {
    EMPTY, MEMOP, LOADI, ARITHOP, OUTPUT, NOP, INTO, COMMA, CONST, REG, COMMENT
};
static const char *CategoryStr[] = {"EMPTY", "MEMOP", "LOADI", "ARITHOP", "OUTPUT", "NOP", "INTO", "COMMA", "CONST",
                                    "REG"};

enum Operation {
    load, store, loadI, add, sub, mult, lshift, rshift, output, nop, err
};
static const char *OperationStr[] = {"load", "store", "loadI", "add", "sub", "mult", "lshift", "rshift", "output",
                                     "nop"};

// 0 - NU, 1 - PR, 2 - VR, 3 - SR
enum RegType {
    NU, PR, VR, SR
};
//static const char *RegTypeStr[] = {"nu", "pr", "vr", "sr"}; //TODO: this is part of InstructionIR->showReg()

enum RegNum {
    R1, R2, R3, Rnone
};

struct InctructionIR {
    Operation opcode;
    int line_num;
    // R1, R2 => R3
    // 0 - NU, 1 - PR, 2 - VR, 3 - SR
    int registers[3][4];
    int constant;

    InctructionIR() {
        opcode = err;
        for (int i = 0; i < 12; ++i) {
            registers[i / 4][i % 4] = INT_MAX;
        }
        constant = -1;
    }

    InctructionIR(Operation s, int r1, int r2, int r3, int c, RegType rType = SR) {
        opcode = s;
        registers[R1][rType] = r1;
        registers[R2][rType] = r2;
        registers[R3][rType] = r3;
        constant = c;
    }

    void showReg(RegType RT) const {
        //todo: fix this back
        string rt = "r";//RegTypeStr[RT];
        string s1 = (opcode == loadI || opcode == output) ? to_string(constant) : "";

        cout << OperationStr[opcode] << " "
             << ((registers[R1][RT] != INT_MAX) ? rt + to_string(registers[R1][RT]) : s1)
             << ((registers[R2][RT] != INT_MAX) ? ", " + rt + to_string(registers[R2][RT]) : "")
             << ((registers[R3][RT] != INT_MAX) ? " => " + rt + to_string(registers[R3][RT]) : "") << endl;
    }

    void showAllRegs() const {
        int tmpReg;
        cout << "opcode " << "\t Const " << "\tR1 (SR,VR,PR,LU) " << "    R2   " << "\t  R3 " << endl;
        cout << OperationStr[opcode] << "\t" << constant << "\t";
        for (int i = 0; i < 3; ++i) {
            for (int j = 3; j >= 0; --j) {
                tmpReg = registers[i][j];

                cout << ((j == 3) ? " | " : " ") << ((tmpReg != INT_MAX) ? to_string(tmpReg) : "-") << " ";
            }
        }
        cout << endl;
    }

};

struct InstructionBlock {
    list <InctructionIR> instructions;
// showIR - print IR in human readable form

    void showInstructions() const {

        if (instructions.empty()) {
            cerr << "ERROR: Not valid instructions, run terminates." << endl;
            return;
        }

        for (const InctructionIR &inst : instructions) {

            switch (inst.opcode) {

                case load:
                    cout << "load\t [ sr" << inst.registers[R1][SR] << " ], [ ], [ sr" << inst.registers[R3][SR] << " ]"
                         << endl;
                    break;

                case store:
                    cout << "store\t [ sr" << inst.registers[R1][SR] << " ], [ ], [ sr" << inst.registers[R3][SR]
                         << " ]"
                         << endl;
                    break;

                case loadI:
                    cout << "loadI\t [ val " << inst.constant << " ], [ ], [ sr" << inst.registers[R3][SR] << " ]"
                         << endl;
                    break;

                case output:
                    cout << OperationStr[inst.opcode] << "\t [ val " << inst.constant << " ], [ ], [ ]" << endl;
                    break;

                case nop:
                    cout << "nop\t [ ], [ ], [ ]" << endl;
                    break;

                default: //
                    cout << OperationStr[inst.opcode] << "\t [ sr" << inst.registers[R1][SR] << " ], [ sr"
                         << inst.registers[R2][SR] << " ], [ sr" << inst.registers[R3][SR] << " ]" << endl;
                    break;
            }
        }
    }

    void showIR(RegType rt = SR) const {
        if (instructions.empty()) {
            cerr << "ERROR: Not valid instructions, run terminates." << endl;
            return;
        }

        for (const InctructionIR &inst : this->instructions) {
            inst.showReg(rt);
        }
    }

    void showAllRegs() const {
        if (instructions.empty()) {
            cerr << "ERROR: Not valid instructions, run terminates." << endl;
            return;
        }

        for (const InctructionIR &inst : this->instructions) {
            inst.showAllRegs();
        }
    }
};

enum InputMode {
    s, p, r, k, x, h
};
// Parsing funcitons
tuple<InputMode, int, char *> manageInput(int argc, char *argv[]);

int strToInt(const string &str);

void takeErrWord(int line_num, const string &line, int &char_pos, string &err_word);

InstructionBlock scan(char *filename, bool check_semantics, bool display_tokens);

vector <pair<Category, string>> processLine(const string &line, int line_num, bool display_tokens);

pair <State, Category> nextState(State state, char new_char, string &lexeme);

InctructionIR parse(const vector <pair<Category, string>> &words, int line_num);

InctructionIR setupInstruction(InctructionIR &inst, const vector <pair<Category, string>> &words, int line_num);

InctructionIR
checkSemantics(const vector <pair<Category, string>> &words, const vector <Category> &grammar, int line_num);


//***********************************************************************************************************
//TODO: maybe put vector for srToVr and lastUse
struct TableSrVr {
    int *srToVr;
    int *lastUse;
    int n;

    TableSrVr(int size) : n(size) {
        try {
            srToVr = new int[size];
            lastUse = new int[size];
        } catch (bad_alloc &ba) {
            cerr << "ERROR ALLOCATING MEMORY: bad_alloc caugth: " << ba.what() << endl;
            exit(1);
        }

        for (int i = 0; i < size; ++i) {
            srToVr[i] = INT_MAX;
            lastUse[i] = INT_MAX;
        }

    }

    ~TableSrVr() {
        delete[] srToVr;
        delete[] lastUse;
    }

    void show() {
        cout << endl << "srToVr: ";
        for (int i = 0; i < n; ++i)
            cout << srToVr[i] << ", ";

        cout << endl << "LU: ";
        for (int i = 0; i < n; ++i)
            cout << lastUse[i] << ", ";
        cout << endl;
    }
};

//Registar Renaming
int registerRenaming(const InstructionBlock &block);

int regNormalize(InstructionBlock &block) {

    int cnt = 0;
    unordered_map<int, int> regMap;

//    cout<<"Reg Normalize: "<<endl;

    for (auto &ir: block.instructions) {
        for (int i = 0; i < 3; ++i) {

            if (ir.registers[i][SR] == INT_MAX)
                continue;

            if (regMap.find(ir.registers[i][SR]) != regMap.end()) {
                ir.registers[i][SR] = regMap[ir.registers[i][SR]];
            } else {
//                cout<<cnt<<" SR = "<< ir.registers[i][SR] <<" => SR_norm = "<<cnt<<endl;
                regMap[ir.registers[i][SR]] = cnt;
                ir.registers[i][SR] = cnt++;
            }
        }
    }

    return regMap.size();
}

void opDef(int regs[][4], TableSrVr &table, int &vrName, RegNum Ri = R3) {
    //OP defines
    if (table.srToVr[regs[Ri][SR]] == INT_MAX) {   //unused srToVr
        table.srToVr[regs[Ri][SR]] = vrName++;
    }
    regs[R3][VR] = table.srToVr[regs[Ri][SR]];
    regs[R3][NU] = table.lastUse[regs[Ri][SR]];

    table.srToVr[regs[Ri][SR]] = INT_MAX;
    table.lastUse[regs[Ri][SR]] = INT_MAX;

}

void opUse(int regs[][4], TableSrVr &table, int &vrName, RegNum Ri, const int &index) {
    //OP uses
//    cout << "opUse"<<endl;
//    cout << regs[Ri][SR]<<endl;//<<" "<<table.srToVr[regs[Ri][SR]]<<endl;

    if (table.srToVr[regs[Ri][SR]] == INT_MAX) {   //last Use
        table.srToVr[regs[Ri][SR]] = vrName++;
    }
    regs[Ri][VR] = table.srToVr[regs[Ri][SR]];
    regs[Ri][NU] = table.lastUse[regs[Ri][SR]];
    table.lastUse[regs[Ri][SR]] = index;
}


void setVR(list<InctructionIR>::iterator ins, TableSrVr &table, int &vrName, const int &index) {

//    cout << "SET VR"<<endl;
    switch (ins->opcode) {
        case load:
            opDef(ins->registers, table, vrName, R3);
            opUse(ins->registers, table, vrName, R1, index);
            break;

        case loadI:
            opDef(ins->registers, table, vrName, R3);
            break;

        case store:
            opUse(ins->registers, table, vrName, R1, index);
            opUse(ins->registers, table, vrName, R3, index);
            break;

        case output:
            break;

        default: //ARITH
            opDef(ins->registers, table, vrName, R3);
            opUse(ins->registers, table, vrName, R1, index);
            opUse(ins->registers, table, vrName, R2, index);
            break;
    }
//    cout << endl << index << " " << OperationStr[ins->opcode] << ": vrName = " << vrName;
//    table.show();

}

int registerRenaming(InstructionBlock &block) {

    int vrName = 0;
    int index = block.instructions.size() - 1; //just to come to 0
    int maxReg = regNormalize(block);
    TableSrVr table(maxReg);
//    int maxLive = 0;//TODO: maybe this info will be useful

//    cout << "Num Reg = " << maxReg << endl;
//    block.showIR(SR);

    //Go from end of list to begin
    for (auto it = prev(block.instructions.end()); it != prev(block.instructions.begin()); --it) {

        if (it->opcode == nop) {
            it = block.instructions.erase(it);
            continue;
        }
        setVR(it, table, vrName, index);

        index--;
    }


    return vrName;
}
//---------------------------------------------------------------------
// for each physical register p, VR TO PR [PR TO VR[p]] = p
// for each virtual register v, if VR TO PR [v] is defined, PR TO VR[VR TO PR [v]] = v
// if you have same value on reg try to allocate same reg for same virtual reg (avoid reg to reg copy)

pair <list<RegNum>, RegNum> opUseDef(Operation op) {
//    cout<<"OP: "<<OperationStr[op]<<endl;
    switch (op) {
        case load:
            return make_pair(list < RegNum > {R1}, R3);
        case loadI:
            return make_pair(list < RegNum > {}, R3);
        case store:
            return make_pair(list < RegNum > {R1, R3}, Rnone);
        case output:
            return make_pair(list < RegNum > {}, Rnone);
        default: //ARITH
            return make_pair(list < RegNum > {R1, R2}, R3);
    }
}

struct TableVrPr {
    vector<int> vrToPr;
    vector <pair<bool, int>> vrRemat;    //if bool = 0, int = location in memory
    //if bool = 1, int = const for loadI,
    int memLoc;
    vector<int> prToVr; //prNum-1 places for regular operations and 1 for spill
    vector<int> prNU;   // TODO: heap_max
//    vector<bool> prMarked;
    stack<int> prFree;
    int markedReg[2];

    TableVrPr(int vrNum, int prNum, int mem) : vrToPr(vrNum, INT_MAX), vrRemat(vrNum, make_pair(false, INT_MAX)),
                                               memLoc(mem), prToVr(prNum-1, INT_MAX), prNU(prNum-1, INT_MAX){
        for (int i = prNum-2; i >= 0 ; --i) {
            prFree.push(i);
        }
        markedReg[0] = INT_MAX;
        markedReg[1] = INT_MAX;
    }
//                                               prMarked(prNum-1, INT_MAX) {}
    void printFreePR(){
        list<int> tmp;
        cout<< "Free PR:"<<endl;
        while(!prFree.empty()){
            tmp.push_front(prFree.top());
            cout<<prFree.top()<<" | ";
            prFree.pop();
        }
        cout<<endl;
        for(auto &x: tmp)
            prFree.push(x);
    }
    void print(){
        cout<<"----------------------------------------------"<<endl;
        cout<<"Table:"<<endl<<"vrToPr | vrRemat: "<<endl;
        for (int i = 0; i < int(vrToPr.size()); ++i) {
            cout <<i<<". "<< vrToPr[i] << " [ " << vrRemat[i].first << " , " << vrRemat[i].second <<" ]"<<endl;
        }
        cout<<endl;

        cout<<"prToVr | NU: "<<endl;
        for(int i = 0; i < int(prToVr.size()); ++i){
            cout<<i<<". "<< prToVr[i]<< " | "<<prNU[i]<<endl;
        }
        cout<<endl;
        printFreePR();
        cout<<"----------------------------------------------"<<endl;
        cout<<endl;
    }
    void clearMark() {
        markedReg[0] = INT_MAX;
        markedReg[1] = INT_MAX;
    }
};

int posMax(TableVrPr &table) {
    int pos=-1, m = 0;
    for (int i = 0; i < int(table.prNU.size()); ++i) {
        if (i == table.markedReg[0] || i == table.markedReg[1])
            continue;

        if(table.vrRemat[table.prToVr[i]].first == true)
            return i;
        if (table.prNU[i] > m) {
            m = table.prNU[i];
            pos = i;
        }
    }
    if(pos == -1)
        cerr<<"Free PR not found"<<endl<<endl;

    return pos;
}

int spill(list<InctructionIR> &block, list<InctructionIR>::iterator ins, TableVrPr &table) {
    int prSpill = posMax(table);
    int &vrSpill = table.prToVr[prSpill];
    int prMem = table.prToVr.size();

    if (table.vrRemat[vrSpill].first == false && table.prNU[prSpill] != INT_MAX) {
        //Spill LRU Physical REG
        block.insert(ins, InctructionIR(loadI, INT_MAX, INT_MAX, prMem, table.memLoc, PR));
        block.insert(ins, InctructionIR(store, prSpill, INT_MAX, prMem, -1, PR));
        table.vrRemat[vrSpill].second = table.memLoc++;
    }
    table.vrToPr[vrSpill] = INT_MAX;

//    cout <<"SPILL::MARKED = "<< table.markedReg[0]<<", "<<table.markedReg[1]<<"| newPR-> "<< prSpill<<endl;
    return prSpill;
}

void restore(list<InctructionIR> &block, list<InctructionIR>::iterator ins, RegNum reg, TableVrPr &table, int vr) {
    int prMem = table.prToVr.size();

    if(table.vrRemat[vr].first == true){    //Rematerialize
        block.insert(ins, InctructionIR(loadI, INT_MAX, INT_MAX, table.vrToPr[vr], table.vrRemat[vr].second, PR));
    }else{  // Have to ReLoad from Mem
        block.insert(ins, InctructionIR(loadI, INT_MAX, INT_MAX, prMem, table.vrRemat[vr].second, PR));
        block.insert(ins, InctructionIR(load, prMem, INT_MAX, table.vrToPr[vr], INT_MAX, PR));
    }
}


int getPr(list<InctructionIR> &block, list<InctructionIR>::iterator ins, RegNum reg, TableVrPr &table) {
    int &vr = ins->registers[reg][VR];
    int &nu = ins->registers[reg][NU];
    int pr;

    if (table.prFree.empty()) {
//        cout<<"No Free PRs"<<endl;
        pr = spill(block, ins, table); //todo: consider dirty bit
    } else {
        pr = table.prFree.top();
        table.prFree.pop();
    }
    table.vrToPr[vr] = pr;
    table.prToVr[pr] = vr;
    table.prNU[pr] = nu;

    return pr;
}

void setPr(list<InctructionIR> &block, list<InctructionIR>::iterator ins, TableVrPr &table) {

    pair <list<RegNum>, RegNum> regUseDef = opUseDef(ins->opcode);    // first - uses, second - def
    int pr;
    int vr;
//    table.clearMark();

    for (auto &r1_2: regUseDef.first) {
//        cout << regUseDef.first.empty()<<" SetPR: R1_2 = "<< r1_2 <<endl;

        vr = ins->registers[r1_2][VR];
//        cout << "VR: "<< vr <<endl;

        pr = table.vrToPr[vr];
//        cout << "PR: "<< pr <<endl;

        if (pr == INT_MAX) {
            pr = getPr(block, ins, r1_2, table);
            restore(block, ins, r1_2, table, vr);
//            table.print();
        }
        ins->registers[r1_2][PR] = pr;
//            table.prMarked[pr] = true;

        table.markedReg[r1_2] = pr;
    }

    for (auto &r1_2: regUseDef.first) {
        pr = ins->registers[r1_2][PR];
        if (ins->registers[r1_2][NU] == INT_MAX && pr != INT_MAX) {
            table.vrToPr[table.prToVr[pr]] = INT_MAX;
            table.prToVr[pr] = INT_MAX;
            table.prNU[pr] = INT_MAX;
            table.prFree.push(pr);
        }
    }

    table.clearMark();
//    cout << "SetPR: R3 = "<< regUseDef.second<<endl;

    RegNum &r3 = regUseDef.second;
    if ( r3 != Rnone) {

//        cout << "dVR: "<< vr <<endl;

        if (ins->opcode == loadI){
            vr = ins->registers[r3][VR];
            table.vrRemat[vr] = make_pair(true, ins->constant);
        }else{
            pr = getPr(block, ins, r3, table);
//        table.print();
            ins->registers[r3][PR] = pr;
//        cout << "dPR: "<< pr <<endl;

        }
//        table.prMarked[pr] = true;
    }
}

int bottomUpAllocator(InstructionBlock &block, int vrNum, int prNum) {

    TableVrPr table(vrNum, prNum, 4000);
//    cout << "table init: VRnum = "<< vrNum<< "| PRnum = "<< prNum<<endl;

    for (auto it = block.instructions.begin(); it != block.instructions.end(); ) {

//        cout<<"----------------------------------------------"<<endl;
//        cout<<"----------------------------------------------"<<endl;
//        it->showReg(VR);
        setPr(block.instructions, it, table);
        if(it->opcode == loadI)
            it = block.instructions.erase(it);
        else
            ++it;
    }
    return 0;
}
//***********************************************************************************************************
// HELPER FUNCTIONS

// manageInput - parse command line
tuple<InputMode, int, char *> manageInput(int argc, char *argv[]) {

    tuple<InputMode, int, char *> input(make_tuple(s, 0, argv[argc - 1]));

//    cout << "INPUT MODE"<<endl;

    if( argc == 1)
        get<0>(input) = h;
    else if (argc == 2) {
        get<0>(input) = p;
        return input;
    }else if(argc == 3){
        int regNum = strToInt(argv[1]);
        if (3 <= regNum && regNum <= 64){
            get<0>(input) = k;
            get<1>(input) = regNum;
        }else{
            cerr << "Number of registers must be in interval [3, 64]" << endl;
            exit(1);
        }

        return input;
    }

    for (int i = 1; i < argc - 1; ++i) {

        if (strcmp(argv[i], "-h") == 0) {
            get<0>(input) = h;
        } else if (strcmp(argv[i], "-x") == 0 && get<0>(input) < h) {
            get<0>(input) = x;
        } else if (strcmp(argv[i], "-r") == 0 && get<0>(input) < r) {
            get<0>(input) = r;
        } else if (strcmp(argv[i], "-p") == 0 && get<0>(input) < p) {
            get<0>(input) = p;
        } else if (strcmp(argv[i], "-s") == 0 && get<0>(input) < s) {
            get<0>(input) = s;
        }
    }


    return input;
}

// strToInt - converts string(const | r+const) to int
int strToInt(const string &str) {
    int n = 0;
    int i = 0;

    if (str[0] == 'r') {
        i = 1;
    }
    for (; i < int(str.length()); ++i) {
        if(str[i] < '0' || str[i] > '9')
            return INT_MAX;
        n = 10 * n + (str[i] - '0');
    }
    return n;
}

// strToOperation - converts string to enum Operation
Operation strToOperation(const string &op) {

    if (op == "load")
        return load;
    else if (op == "store")
        return store;
    else if (op == "loadI")
        return loadI;
    else if (op == "add")
        return add;
    else if (op == "sub")
        return sub;
    else if (op == "mult")
        return mult;
    else if (op == "lshift")
        return lshift;
    else if (op == "rshift")
        return rshift;
    else if (op == "output")
        return output;
    else
        return nop;

}

// If error happen it gets whole word as error word
void takeErrWord(int line_num, const string &line, int &char_pos, string &err_word) {

    while (line[char_pos] != ' ' && line[char_pos] != '\t' && line[char_pos] != '\n') {
        err_word.push_back(line[++char_pos]);
    }
    if (line[char_pos] == ' ' || line[char_pos] == '\t' || line[char_pos] == '\n')
        err_word.pop_back();

    cerr << "ERROR: " << line_num << ": Lexical: \"" << err_word <<
         "\" is not a valid word." << endl;

    return;
}



// MAIN FUNCTIONS Implementations

// Scan - creates tokens from input file, check lexical errors, and check_semantics(make IR)
InstructionBlock scan(char *filename, bool check_semantics = false, bool display_tokens = false) {

    ifstream input_stream(filename, ifstream::in);
    if (!input_stream) cerr << "Cannot open input file: \"" << filename << "\"" << endl;

    string line;
    int line_num = 0;
    int inst_count = 0;
    vector <pair<Category, string>> result;

    InctructionIR inst;
    InstructionBlock block;
    int error_count = 0;


    while (getline(input_stream, line)) {
//        cout << "LINE: " << line << endl;
        line.push_back('\n');
        result = processLine(line, ++line_num, display_tokens);

        if (result.empty())
            continue;

        inst_count++;

        // Parse Part
        if (check_semantics) {
            inst = parse(result, line_num);

            if (inst.opcode != err) {
                if (error_count == 0)
                    block.instructions.push_back(inst);
            } else {
                error_count++;
            }
        }

    }

    if (display_tokens) {
        cout << line_num << ": < ENDFILE  , \"\" >" << endl;
    }

    if (check_semantics) {
        if (error_count == 0) {
//            cout << endl << "Parse succeeded, finding " << inst_count - error_count << " ILOC operations." << endl;
        } else {
            cerr << endl << "Parser found " << error_count << " Syntax errors in " << line_num << " lines of input."
                 << endl;
        }
    }

    if (inst_count == 0) {
        cout << "WARNING: ILOC file contained no operations." << endl;
    }


    return (error_count == 0) ? block : InstructionBlock();
}

// processLine - creates vector of tokens(Category, string)
vector <pair<Category, string>> processLine(const string &line, int line_num, bool display_tokens) {

    string lexeme, err_str;
    pair <State, Category> s;
    vector <pair<Category, string>> tokens;
    ostringstream stream;

//    cout << "PROCSS LINE: " << line;
    //Initialize first state
    s.first = s0;
//    line.push_back('\n');


    for (int char_pos = 0; char_pos < int(line.length()); ++char_pos) {
//        cout << s <<"->";
        s = nextState(s.first, line[char_pos], lexeme);
//        cout << s << endl;


        if (s.first != sErr) {
            if (s.second != EMPTY) {
                // Category {EMPTY, MEMOP, LOADI, ARITHOP, OUTPUT, NOP, INTO, COMMA, CONST, REG, COMMENT};
                if (s.second == COMMENT) {
                    break;
                } else {   //ALL OPERATIONS, REG, CONST
                    stream << line_num << ": < " << CategoryStr[s.second] << "\t, \"" << lexeme << "\" >" << endl;
                    tokens.push_back(make_pair(s.second, lexeme));

                    if (s.second == REG && s.first == sComma) {
                        stream << line_num << ": < COMMA\t, \",\" >" << endl;
                        tokens.push_back(make_pair(COMMA, ","));
                        s.first = s0;
                    }
                }
            }

        } else { // Error States
            if (s.second != EMPTY) { //Accept lexeme[0:n-1] and make err message
                err_str = lexeme;
                lexeme.pop_back();  // word you accept
                stream << line_num << ": < " << CategoryStr[s.second] << "\t, \"" << lexeme << "\" >" << endl;
                tokens.push_back(make_pair(s.second, lexeme));

                takeErrWord(line_num, line, char_pos, err_str);
                tokens.push_back(make_pair(EMPTY, err_str));
                s.first = s0;
            } else { // make err message
                takeErrWord(line_num, line, char_pos, lexeme);
                tokens.push_back(make_pair(EMPTY, lexeme));
                s.first = s0;
            }
        }


    }
    if (display_tokens) {
        cout << stream.str();
    }
    return tokens;
}

// nextState - DFA of trasitions
pair <State, Category> nextState(State state, char new_char, string &lexeme) {

    Category cat = EMPTY;

    switch (state) {

        case s0:
            lexeme.clear();
            lexeme.push_back(new_char);
            if (new_char == 's') {
                state = s1;
            } else if (new_char == 'l') {
                state = s8;
            } else if (new_char == 'r') {
                state = s13;
            } else if (new_char == 'm') {
                state = s19;
            } else if (new_char == 'a') {
                state = s22;
            } else if (new_char == 'n') {
                state = s25;
            } else if (new_char == 'o') {
                state = s28;
            } else if (new_char == '=') {
                state = s34;
            } else if (new_char == ',') {
                cat = COMMA;
                state = s0;
            } else if (new_char == ' ' || new_char == '\t' || new_char == '\n') {
                state = s0;
            } else if (new_char >= '0' && new_char <= '9') {
                state = s35;
            } else if (new_char == '/') {
                state = s37;
            } else {
                state = sErr;
            }
            break;

        case s1:
            lexeme.push_back(new_char);
            if (new_char == 't') {
                state = s2;
            } else if (new_char == 'u') {
                state = s6;
            } else {
                state = sErr;
            }
            break;

        case s2:
            lexeme.push_back(new_char);
            if (new_char == 'o') {
                state = s3;
            } else {
                state = sErr;
            }
            break;

        case s3:
            lexeme.push_back(new_char);
            if (new_char == 'r') {
                state = s4;
            } else {
                state = sErr;
            }
            break;

        case s4:
            lexeme.push_back(new_char);
            if (new_char == 'e') {
                state = s5;
            } else {
                state = sErr;
            }
            break;

        case s5: //store
            if (new_char == ' ' || new_char == '\t' || new_char == '\n') {
                cat = MEMOP;
                state = s0;
            } else {
                state = sErr;
                cat = MEMOP;
                lexeme.push_back(new_char);
            }
            break;

        case s6:
            lexeme.push_back(new_char);
            if (new_char == 'b') {
                state = s7;
            } else {
                state = sErr;
            }
            break;

        case s7: //sub
            if (new_char == ' ' || new_char == '\t' || new_char == '\n') {
                cat = ARITHOP;
                state = s0;
            } else {
                state = sErr;
                cat = ARITHOP;
                lexeme.push_back(new_char);
            }
            break;

        case s8:
            lexeme.push_back(new_char);
            if (new_char == 'o') {
                state = s9;
            } else if (new_char == 's') {
                state = s14;
            } else {
                state = sErr;
            }
            break;

        case s9:
            lexeme.push_back(new_char);
            if (new_char == 'a') {
                state = s10;
            } else {
                state = sErr;
            }
            break;

        case s10:
            lexeme.push_back(new_char);
            if (new_char == 'd') {
                state = s11;
            } else {
                state = sErr;
            }
            break;

        case s11: //load
            if (new_char == 'I') {
                state = s12;
                lexeme.push_back(new_char);
            } else if (new_char == ' ' || new_char == '\t' || new_char == '\n') {
                cat = MEMOP;
                state = s0;
            } else {
                state = sErr;
                cat = MEMOP;
                lexeme.push_back(new_char);
            }
            break;

        case s12:
            if (new_char == ' ' || new_char == '\t' || new_char == '\n') {
                cat = LOADI;
                state = s0;
            } else {
                state = sErr;
                cat = LOADI;
                lexeme.push_back(new_char);
            }
            break;

        case s13:
            lexeme.push_back(new_char);
            if (new_char == 's') {
                state = s14;
            } else if (new_char >= '0' && new_char <= '9') {
                state = s36;
            } else {
                state = sErr;
                lexeme.push_back(new_char);
            }
            break;

        case s14:
            lexeme.push_back(new_char);
            if (new_char == 'h') {
                state = s15;
            } else {
                state = sErr;
            }
            break;

        case s15:
            lexeme.push_back(new_char);
            if (new_char == 'i') {
                state = s16;
            } else {
                state = sErr;
            }
            break;

        case s16:
            lexeme.push_back(new_char);
            if (new_char == 'f') {
                state = s17;
            } else {
                state = sErr;
            }
            break;

        case s17:
            lexeme.push_back(new_char);
            if (new_char == 't') {
                state = s18;
            } else {
                state = sErr;
            }
            break;

        case s18: //rshift, lshift, mult
            if (new_char == ' ' || new_char == '\t' || new_char == '\n') {
                cat = ARITHOP;
                state = s0;
            } else {
                state = sErr;
                cat = ARITHOP;
                lexeme.push_back(new_char);
            }
            break;

        case s19:
            lexeme.push_back(new_char);
            if (new_char == 'u') {
                state = s20;
            } else {
                state = sErr;
            }
            break;

        case s20:
            lexeme.push_back(new_char);
            if (new_char == 'l') {
                state = s21;
            } else {
                state = sErr;
            }
            break;

        case s21:
            lexeme.push_back(new_char);
            if (new_char == 't') {
                state = s18;
            } else {
                state = sErr;
            }
            break;

        case s22:
            lexeme.push_back(new_char);
            if (new_char == 'd') {
                state = s23;
            } else {
                state = sErr;
            }
            break;

        case s23:
            lexeme.push_back(new_char);
            if (new_char == 'd') {
                state = s24;
            } else {
                state = sErr;
            }
            break;

        case s24: //add
            if (new_char == ' ' || new_char == '\t' || new_char == '\n') {
                cat = ARITHOP;
                state = s0;
            } else {
                state = sErr;
                cat = ARITHOP;
                lexeme.push_back(new_char);
            }
            break;

        case s25:
            lexeme.push_back(new_char);
            if (new_char == 'o') {
                state = s26;
            } else {
                state = sErr;
            }
            break;

        case s26:
            lexeme.push_back(new_char);
            if (new_char == 'p') {
                state = s27;
            } else {
                state = sErr;
            }
            break;

        case s27: //nop
            if (new_char == ' ' || new_char == '\t' || new_char == '\n') {
                cat = NOP;
                state = s0;
            } else if (new_char == '/') {
                cat = NOP;
                state = s37;
            } else {
                state = sErr;
                cat = NOP;
                lexeme.push_back(new_char);
            }
            break;

        case s28:
            lexeme.push_back(new_char);
            if (new_char == 'u') {
                state = s29;
            } else {
                state = sErr;
            }
            break;

        case s29:
            lexeme.push_back(new_char);
            if (new_char == 't') {
                state = s30;
            } else {
                state = sErr;
            }
            break;

        case s30:
            lexeme.push_back(new_char);
            if (new_char == 'p') {
                state = s31;
            } else {
                state = sErr;
            }
            break;

        case s31:
            lexeme.push_back(new_char);
            if (new_char == 'u') {
                state = s32;
            } else {
                state = sErr;
            }
            break;

        case s32:
            lexeme.push_back(new_char);
            if (new_char == 't') {
                state = s33;
            } else {
                state = sErr;
            }
            break;

        case s33: //output
            if (new_char == ' ' || new_char == '\t' || new_char == '\n') {
                cat = OUTPUT;
                state = s0;
            } else {
                state = sErr;
                cat = OUTPUT;
                lexeme.push_back(new_char);
            }
            break;

        case s34: // INTO =>
            if (new_char == '>') {
                cat = INTO;
                lexeme = "=>";
                state = s0;
            } else {
                state = sErr;
                lexeme.push_back(new_char);
            }
            break;

        case s35: // Handle Constant
            if (new_char >= '0' && new_char <= '9') {
                state = s35;
                lexeme.push_back(new_char);
            } else if (new_char == ' ' || new_char == '\t' || new_char == '\n') {
                cat = CONST;
                state = s0;
            } else if (new_char == '/') {
                cat = CONST;
                state = s37;
            } else if (new_char == '=') {
                state = s34;
                cat = CONST;
            } else {
                state = sErr;
                cat = CONST;
                lexeme.push_back(new_char);
            }
            break;

        case s36: // Handle Register
            if (new_char >= '0' && new_char <= '9') {
                state = s36;
                lexeme.push_back(new_char);
            } else if (new_char == ' ' || new_char == '\t' || new_char == '\n') {
                cat = REG;
                state = s0;
            } else if (new_char == '/') {
                cat = REG;
                state = s37;
            } else if (new_char == ',') {
                cat = REG;
                state = sComma; // send both REG and COMMA
            } else if (new_char == '=') {
                cat = REG;
                state = s34;
            } else {
                state = sErr;
                cat = REG;
                lexeme.push_back(new_char);
            }
            break;
        case s37: // Handle COMMENTs
            if (new_char == '/') {
                cat = COMMENT;


            } else {
                state = sErr;
                lexeme.push_back(new_char);
            }
            break;

        default:
            state = sErr;
    }

    return make_pair(state, cat);
}

// parse - check semantics for every Category
InctructionIR parse(const vector <pair<Category, string>> &words, int line_num) {

    switch (words[0].first) {
        case MEMOP:
            return checkSemantics(words, vector<Category>({MEMOP, REG, INTO, REG}), line_num);

        case LOADI:
            return checkSemantics(words, vector<Category>({LOADI, CONST, INTO, REG}), line_num);

        case ARITHOP:
            return checkSemantics(words, vector<Category>({ARITHOP, REG, COMMA, REG, INTO, REG}), line_num);

        case OUTPUT:
            return checkSemantics(words, vector<Category>({OUTPUT, CONST}), line_num);

        case NOP:
            return checkSemantics(words, vector<Category>({NOP}), line_num);

        default:
            cerr << "ERROR: " << line_num << ": Syntax: Undefined operation \"" << words[0].second << "\"" << endl;
            return InctructionIR();
    }
}

// setupInstruction - creates instruction IR from tokens
InctructionIR setupInstruction(InctructionIR &inst, const vector <pair<Category, string>> &words, int line_num) {
    // SETUP INSTRUCTION_IR

    inst.opcode = strToOperation(words[0].second);
    inst.line_num = line_num;
    switch (words[0].first) {
        case MEMOP:
            inst.registers[R1][SR] = strToInt(words[1].second);
            inst.registers[R3][SR] = strToInt(words[3].second);
            break;

        case LOADI:
            inst.constant = strToInt(words[1].second);
            inst.registers[R3][SR] = strToInt(words[3].second);
            break;

        case ARITHOP:
            inst.registers[R1][SR] = strToInt(words[1].second);
            inst.registers[R2][SR] = strToInt(words[3].second);
            inst.registers[R3][SR] = strToInt(words[5].second);
            break;

        case OUTPUT:
            inst.constant = strToInt(words[1].second);
            break;

        default:
            break;
    }
    return inst;
}

// checkSemantics - compare grammar with tokens
InctructionIR
checkSemantics(const vector <pair<Category, string>> &words, const vector <Category> &grammar, int line_num) {

    InctructionIR inst;
    int i;

    if (words.size() < grammar.size()) {
        cerr << "ERROR: " << line_num << ": Syntax: too few words. Format is: ";

        for (int i = 0; i < int(grammar.size()); ++i) {
            cout << CategoryStr[grammar[i]] << " ";
        }
        cout << endl;
        return inst;

    }

    for (i = 1; i < int(grammar.size()); ++i) {
//            cout << words[i].first << " -- gramar: "<< grammar[i] << endl;
        if (words[i].first != grammar[i]) {
            cerr << "ERROR: " << line_num << ": Syntax: Invalid word \"" << words[i].second <<
                 "\" Should be: " << CategoryStr[grammar[i]] << endl;
            return inst;
        }
    }
    if (words.size() > grammar.size()) {
        cerr << "ERROR: " << line_num << ": Syntax: too many words :\"" << words[i].second << "\"" << endl;
        return inst;
    }

    return setupInstruction(inst, words, line_num);
}


int main(int argc, char *argv[]) {

    tuple<InputMode, int, char *> input = manageInput(argc, argv);
    InstructionBlock irBlock;

    switch (get<0>(input)) {

        case h:
            cout << "Optional flags:\n" <<
                 "        -h       prints this message\n" <<
                 "        -x       renaming source registers to virtual registers\n" <<
                 "        k        k - register allocation\n" <<
                 "        -s       prints tokens in token stream\n" <<
                 "        -p       invokes parser and reports on success or failure\n" <<
                 "        -r       prints human readable version of parser's IR\n";
            break;

        case r:
            irBlock = scan(get<2>(input), true, false);
            irBlock.showIR();
            break;

        case p:
            scan(get<2>(input), true, false);
            break;

        case s:
            scan(get<2>(input), false, true);
            break;

        case x:
            irBlock = scan(get<2>(input), true, false);
//            irBlock.showAllRegs();
//            irBlock.showIR();

            registerRenaming(irBlock);
//            cout << "reg REName pass" << endl;
//            irBlock.showAllRegs();
//            irBlock.showIR(SR);
            irBlock.showIR(VR);
            break;

        case k:
//            cout <<"BOTTOM UP ALLOCATOR PR"<<endl<<endl;
            irBlock = scan(get<2>(input), true, false);
            int vrNum = registerRenaming(irBlock);
//            irBlock.showIR(VR);

            int prReg = get<1>(input);
//            cout <<"Successful Renaming"<<endl<<endl;
            bottomUpAllocator(irBlock, vrNum, prReg);
//            cout <<"Successful Allocation"<<endl<<endl;
            irBlock.showIR(PR);

//            irBlock.showAllRegs();
            break;
    }

    return 0;
}