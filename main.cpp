#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cerrno>
#include <string.h>
#include <list>
#include <vector>
#include <array>

using namespace std;

enum Operation{ load, store, loadI, add, sub, mult, lshift, rshift, output, nop, err};
static const char *OperationStr[] = {"load", "store", "loadI", "add", "sub", "mult", "lshift", "rshift", "output", "nop"};


enum State {
    s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14,
    s15, s16, s17, s18, s19, s20, s21, s22, s23, s24, s25, s26, s27,
    s28, s29, s30, s31, s32, s33, s34, s35, s36, s37, s38, s39,
    sErr, COMMENT, MEMOP, LOADI, ARITHOP, OUTPUT, NOP, INTO, COMMA, CONST,
    REG, CONSTINTO, REGINTO, REGCOMMA};

static const char *CategoryStr[] = {"MEMOP", "LOADI", "ARITHOP", "OUTPUT", "NOP", "INTO", "COMMA", "CONST", "REG"};

struct InctructionIR{
    Operation opcode;
    int line;
    // 0 - NU, 1 - PR, 2 - VR, 3 - SR
    int registers[3][4];
    int constant;

    InctructionIR(){
        opcode = err;
    }
    InctructionIR(Operation s, int r1, int r2, int r3, int c){
        opcode = s;
        registers[0][3] = r1;
        registers[1][3] = r2;
        registers[2][3] = r3;
        constant = c;
    }


};
struct InstructionBlock{
    list< InctructionIR > instructions;

};

InstructionBlock scan(char *filename, bool check_semantics, bool display);
vector<pair<State, string>> processLine(string line, int current_line, bool display);
State nextState(State s, char new_char, string &lexeme);
InctructionIR parse(const vector < pair<State, string> > &words , int line);
InctructionIR checkSemantics(const vector < pair<State, string> > &words, const vector<State> grammar, int line);

// FUNCTIONS

InstructionBlock scan(char *filename, bool check_semantics = false, bool display = false) {

    ifstream input_stream(filename, ifstream::in);
    if (!input_stream) cerr << "Cannot open input file";

    string line;
    int current_line = 0;
    int inst_count = 0;
    vector < pair<State, string> > result;

    InctructionIR inst;
    InstructionBlock block;
    int error_count = 0;


    while (getline(input_stream, line)) {
//        cout << "LINE: " << line << endl;
        result = processLine(line, ++current_line, display);

        if(result.empty())
            continue;

        // Parse Part
        if(check_semantics){
            inst = parse(result, current_line);

            if( inst.opcode != err ){
                inst_count++;

                if(error_count == 0)
                    block.instructions.push_back(inst);

            }else{
                error_count++;
            }
        }

    }

    if(display){
        cout << --current_line << ": < ENDFILE  , \"\" >" << endl;
    }

    if(error_count == 0){
        cout << endl << "Parse succeeded, finding "<< inst_count <<" ILOC operations." << endl;
    }else{
        cout << endl << "Parser found "<< error_count <<" Syntax errors in " << current_line <<" lines of input." << endl;
    }

    if (current_line == 1){
        cout << "WARNING: ILOC file contained no operations." << endl;
    }


    return (error_count == 0) ? block : InstructionBlock();
}


int strToInt(const string &str) {
    int n = 0;
    int i = 0;

    if(str[0] == 'r') {
        i = 1;
    }
    for (; i < int(str.length()) ; ++i) {
        n = 10*n + (str[i] - '0');
    }
    return n;
}

vector<pair<State, string>> processLine(string line, int current_line, bool display){
    string lexeme;
    State s = s0;
    vector< pair<State, string> > tokens;
    ostringstream stream;
    line.push_back('\n');

//    cout << "PROCSS LINE: " << line;

    for (int char_pos = 0; char_pos < int(line.length()); ++char_pos) {
//        cout << s <<"->";
        s = nextState(s, line[char_pos], lexeme);
//        cout << s << endl;

        if (s >= MEMOP) {
            if ( s <= REG){
                stream << current_line << ": < " << CategoryStr[s - MEMOP] << "\t, \"" << lexeme << "\" >"<< endl;
                tokens.push_back(make_pair(s, lexeme));

            }else if(s == CONSTINTO) {
                stream << current_line << ": < CONST\t, \"" << lexeme << "\" >" << endl;
                stream << current_line << ": < INTO\t, \"=>\" >" << endl;
                tokens.push_back(make_pair(CONST, lexeme));
                tokens.push_back(make_pair(INTO, "=>"));

            }else if(s == REGINTO) {
                stream << current_line << ": < REG\t, \"" << lexeme << "\" >" << endl;
                stream << current_line << ": < INTO\t, \"=>\" >" << endl;
                tokens.push_back(make_pair(REG, lexeme));
                tokens.push_back(make_pair(INTO, "=>"));

            }else if (s == REGCOMMA) {
                stream << current_line << ": < REG\t, \"" << lexeme <<"\" >"<< endl;
                stream << current_line << ": < COMMA\t, \",\" >" << endl;
                tokens.push_back(make_pair(REG, lexeme));
                tokens.push_back(make_pair(COMMA, ","));
            }
            s = s0;

        } else if (s == sErr) {

            while(line[char_pos] != ' ' && line[char_pos] != '\t' && line[char_pos] != '\n'){
                lexeme.push_back(line[++char_pos]);
            }
            if(line[char_pos] == '\n')
                lexeme.pop_back();

            cout << "ERROR: " << current_line << ": Lexical: \"" << lexeme <<
                 "\" is not a valid word." << endl;

            tokens.push_back(make_pair(sErr, lexeme));
            s = s0;

//            break;

        } else if (s == COMMENT ) {
            break;
        }

    }
    if(display){
        cout << stream.str();
    }
    return tokens;
}

State nextState(State s, char new_char, string &lexeme) {


    switch (s) {

        case s0:
            lexeme.clear();
            lexeme.push_back(new_char);
            if (new_char == 's') {
                s = s1;
            } else if (new_char == 'l') {
                s = s8;
            } else if (new_char == 'r') {
                s = s13;
            } else if (new_char == 'm') {
                s = s19;
            } else if (new_char == 'a') {
                s = s22;
            } else if (new_char == 'n') {
                s = s25;
            } else if (new_char == 'o') {
                s = s28;
            } else if (new_char == '=') {
                s = s34;
            } else if (new_char == ',') {
                s = COMMA;
            } else if (new_char == ' ' || new_char == '\t' || new_char == '\n') {
                s = s0;
            } else if (new_char >= '0' && new_char <= '9') {
                s = s35;
            } else if (new_char == '/') {
                s = s37;
            } else {
                s = sErr;
            }
            break;

        case s1:
            lexeme.push_back(new_char);
            if (new_char == 't') {
                s = s2;
            } else if (new_char == 'u') {
                s = s6;
            } else {
                s = sErr;
            }
            break;

        case s2:
            lexeme.push_back(new_char);
            if (new_char == 'o') {
                s = s3;
            } else {
                s = sErr;
            }
            break;

        case s3:
            lexeme.push_back(new_char);
            if (new_char == 'r') {
                s = s4;
            } else {
                s = sErr;
            }
            break;

        case s4:
            lexeme.push_back(new_char);
            if (new_char == 'e') {
                s = s5;
            } else {
                s = sErr;
            }
            break;

        case s5:
            if (new_char == ' ' || new_char == '\t') {
                s = MEMOP;
            } else {
                s = sErr;
                lexeme.push_back(new_char);
            }
            break;

        case s6:
            lexeme.push_back(new_char);
            if (new_char == 'b') {
                s = s7;
            } else {
                s = sErr;
            }
            break;

        case s7:
            if (new_char == ' ' || new_char == '\t') {
                s = ARITHOP;
            } else {
                s = sErr;
                lexeme.push_back(new_char);
            }
            break;

        case s8:
            lexeme.push_back(new_char);
            if (new_char == 'o') {
                s = s9;
            } else if (new_char == 's') {
                s = s14;
            } else {
                s = sErr;
            }
            break;

        case s9:
            lexeme.push_back(new_char);
            if (new_char == 'a') {
                s = s10;
            } else {
                s = sErr;
            }
            break;

        case s10:
            lexeme.push_back(new_char);
            if (new_char == 'd') {
                s = s11;
            } else {
                s = sErr;
            }
            break;

        case s11:
            if (new_char == 'I') {
                s = s12;
                lexeme.push_back(new_char);
            } else if (new_char == ' ' || new_char == '\t') {
                s = MEMOP;
            } else {
                s = sErr;
                lexeme.push_back(new_char);
            }
            break;

        case s12:
            if (new_char == ' ' || new_char == '\t') {
                s = LOADI;
            } else {
                s = sErr;
                lexeme.push_back(new_char);
            }
            break;

        case s13:
            lexeme.push_back(new_char);
            if (new_char == 's') {
                s = s14;
            } else if (new_char >= '0' && new_char <= '9') {
                s = s36;
            } else {
                s = sErr;
                lexeme.push_back(new_char);
            }
            break;

        case s14:
            lexeme.push_back(new_char);
            if (new_char == 'h') {
                s = s15;
            } else {
                s = sErr;
            }
            break;

        case s15:
            lexeme.push_back(new_char);
            if (new_char == 'i') {
                s = s16;
            } else {
                s = sErr;
            }
            break;

        case s16:
            lexeme.push_back(new_char);
            if (new_char == 'f') {
                s = s17;
            } else {
                s = sErr;
            }
            break;

        case s17:
            lexeme.push_back(new_char);
            if (new_char == 't') {
                s = s18;
            } else {
                s = sErr;
            }
            break;

        case s18:
            if (new_char == ' ' || new_char == '\t') {
                s = ARITHOP;
            } else {
                s = sErr;
                lexeme.push_back(new_char);
            }
            break;

        case s19:
            lexeme.push_back(new_char);
            if (new_char == 'u') {
                s = s20;
            } else {
                s = sErr;
            }
            break;

        case s20:
            lexeme.push_back(new_char);
            if (new_char == 'l') {
                s = s21;
            } else {
                s = sErr;
            }
            break;

        case s21:
            lexeme.push_back(new_char);
            if (new_char == 't') {
                s = s18;
            } else {
                s = sErr;
            }
            break;

        case s22:
            lexeme.push_back(new_char);
            if (new_char == 'd') {
                s = s23;
            } else {
                s = sErr;
            }
            break;

        case s23:
            lexeme.push_back(new_char);
            if (new_char == 'd') {
                s = s24;
            } else {
                s = sErr;
            }
            break;

        case s24:
            if (new_char == ' ' || new_char == '\t') {
                s = ARITHOP;
            } else {
                s = sErr;
                lexeme.push_back(new_char);
            }
            break;

        case s25:
            lexeme.push_back(new_char);
            if (new_char == 'o') {
                s = s26;
            } else {
                s = sErr;
            }
            break;

        case s26:
            lexeme.push_back(new_char);
            if (new_char == 'p') {
                s = s27;
            } else {
                s = sErr;
            }
            break;

        case s27:
            if (new_char == ' ' || new_char == '\t' || new_char == '\n') {
                s = NOP;
            } else {
                s = sErr;
                lexeme.push_back(new_char);
            }
            break;

        case s28:
            lexeme.push_back(new_char);
            if (new_char == 'u') {
                s = s29;
            } else {
                s = sErr;
            }
            break;

        case s29:
            lexeme.push_back(new_char);
            if (new_char == 't') {
                s = s30;
            } else {
                s = sErr;
            }
            break;

        case s30:
            lexeme.push_back(new_char);
            if (new_char == 'p') {
                s = s31;
            } else {
                s = sErr;
            }
            break;

        case s31:
            lexeme.push_back(new_char);
            if (new_char == 'u') {
                s = s32;
            } else {
                s = sErr;
            }
            break;

        case s32:
            lexeme.push_back(new_char);
            if (new_char == 't') {
                s = s33;
            } else {
                s = sErr;
            }
            break;

        case s33:
            if (new_char == ' ' || new_char == '\t') {
                s = OUTPUT;
            } else {
                s = sErr;
                lexeme.push_back(new_char);
            }
            break;

        case s34:
            lexeme.push_back(new_char);
            if (new_char == '>') {
                s = INTO;
            } else {
                s = sErr;
            }
            break;

        case s35: // Handle Constant
            if (new_char >= '0' && new_char <= '9') {
                s = s35;
                lexeme.push_back(new_char);
            } else if (new_char == ' ' || new_char == '\t' || new_char == '\n') {
                s = CONST;
            } else if (new_char == '='){
                s = s38;
            } else {
                s = sErr;
                lexeme.push_back(new_char);
            }
            break;

        case s36: // Handle Register
            if (new_char >= '0' && new_char <= '9') {
                s = s36;
                lexeme.push_back(new_char);
            } else if (new_char == ' ' || new_char == '\t' || new_char == '\n') {
                s = REG;
            } else if (new_char == ',') {
                s = REGCOMMA;
            } else if (new_char == '='){
                s = s39;
            } else {
                s = sErr;
                lexeme.push_back(new_char);
            }
            break;
        case s37: // Handle Comments
            if (new_char == '/') {
                s = COMMENT;
            } else {
                s = sErr;
                lexeme.push_back(new_char);
            }
            break;
        case s38: // Handle number=>
            if ( new_char == '>'){
                s = CONSTINTO;
            }else{
                s = sErr;
                lexeme.push_back(new_char);
            }
            break;
        case s39: // Handle Reg=>
            if ( new_char == '>'){
                s = REGINTO;
            }else{
                s = sErr;
                lexeme.push_back(new_char);
            }
            break;
        default:
            s = sErr;
    }

    return s;
}


InctructionIR parse(const vector < pair<State, string> > &words, int line ){

//    cout << "PARSE: ";

//    for (int i = 0; i < int(words.size()); ++i) {
//        cout << words[i].second<<"|";
//    }
//    cout << endl;

    switch (words[0].first){
        case MEMOP:
            return checkSemantics(words, vector<State>({MEMOP, REG, INTO, REG}), line);

        case LOADI:
            return checkSemantics(words, vector<State>({LOADI, CONST, INTO, REG}), line);

        case ARITHOP:
            return checkSemantics(words, vector<State>({ARITHOP, REG, COMMA, REG, INTO, REG}), line);

        case OUTPUT:
            return checkSemantics(words, vector<State>({OUTPUT, CONST}), line);

        case NOP:
            return checkSemantics(words, vector<State>({NOP}), line);

        default:
            cout << "ERROR: " << line << ": Syntax: Undefined operation \"" << words[0].second <<"\""<< endl;
            return InctructionIR();
    }
}


Operation strToOperation(const string &op){

    if(op == "load")
        return load;
    else if(op == "store")
        return store;
    else if(op == "loadI")
        return loadI;
    else if(op == "add")
        return add;
    else if(op == "sub")
        return sub;
    else if(op == "mult")
        return mult;
    else if(op == "lshift")
        return lshift;
    else if(op == "rshift")
        return rshift;
    else if(op == "output")
        return output;
    else
        return nop;

}

InctructionIR setupInstruction(InctructionIR &inst, const vector < pair<State, string> > &words, int line_num){
    // SETUP INSTRUCTION_IR

    inst.opcode = strToOperation(words[0].second);
    inst.line = line_num;
    switch (words[0].first){
        case MEMOP:
            inst.registers[0][3] = strToInt(words[1].second);
            inst.registers[2][3] = strToInt(words[3].second);
            break;

        case LOADI:
            inst.constant = strToInt(words[1].second);
            inst.registers[2][3] = strToInt(words[3].second);
            break;

        case ARITHOP:
            inst.registers[0][3] = strToInt(words[1].second);
            inst.registers[1][3] = strToInt(words[3].second);
            inst.registers[2][3] = strToInt(words[5].second);
            break;

        case OUTPUT:
            inst.constant = strToInt(words[1].second);
            break;

        default:
            break;
    }
    return inst;
}
InctructionIR checkSemantics(const vector < pair<State, string> > &words, const vector<State> grammar, int line_num){

    InctructionIR inst;
    int i;

    if (words.size() < grammar.size()){
        cout << "ERROR: " << line_num << ": Syntax: too few words. Format is: ";

        for (int i = 0; i < int(grammar.size()) ; ++i) {
            cout << CategoryStr[grammar[i] - MEMOP] <<" ";
        }
        cout << endl;
        return inst;

    }

    for (i = 1; i < int(grammar.size()); ++i) {
//            cout << words[i].first << " -- gramar: "<< grammar[i] << endl;
        if(words[i].first != grammar[i]){
            cout << "ERROR: " << line_num << ": Syntax: Invalid word \"" << words[i].second <<
            "\" Should be: "<< CategoryStr[grammar[i] - MEMOP] << endl;
            return inst;
        }
    }
    if(words.size() > grammar.size()){
        cout << "ERROR: " << line_num << ": Syntax: too many words :\"" << words[i].second <<"\""<<endl;
        return inst;
    }

//    cout << "GOOD SEMANTICS ++++++++++ "<< endl;
    return  setupInstruction(inst, words, line_num);
}

void showIR(InstructionBlock ir){

    for (const InctructionIR &inst : ir.instructions ) {

        switch (inst.opcode){

            case load:
                cout << "load\t [ sr" << inst.registers[0][3] << " ], [ ] [ " << "[ sr" << inst.registers[2][3] << " ]" << endl;
                break;

            case store:
                cout << "store\t [ sr" << inst.registers[0][3] << " ], [ ] [ " << "[ sr" << inst.registers[2][3] << " ]" << endl;
                break;

            case loadI:
                cout << "loadI\t [ val " << inst.constant << " ], [ ] [ " << "[ sr" << inst.registers[2][3] << " ]" << endl;
                break;

            case output:
                cout << OperationStr[inst.opcode] <<"\t [ val" << inst.constant << " ], [ ] [ ]" << endl;
                break;

            case nop:
                cout << "nop\t [ ], [ ], [ ]" << endl;
                break;

            default: //
                cout << OperationStr[inst.opcode] <<"\t [ sr" << inst.registers[0][3] << " ], [ sr"
                << inst.registers[1][3] << " ], [ sr" << inst.registers[2][3] << " ]" << endl;
                break;
        }

    }

}


int main(int argc, char *argv[]) {

    switch (argc) {
        case 1:
            //IF NO ARGS, THEN -P
            cout << "Make IR from File Specified" << endl;

            break;

        case 2:
            if (strcmp(argv[1], "-h") == 0) {
                cout << "Command Line Arguments" << endl;
                cout << "IMPLEMENT OTHER ARGUMENTS" << endl;

            }
            break;

        case 3:
            if (strcmp(argv[1], "-s") == 0) {
                scan(argv[2], false, true);

            } else if (strcmp(argv[1], "-p") == 0) {
                scan(argv[2], true, false);

            } else if (strcmp(argv[1], "-r") == 0) {
                InstructionBlock ir;
                ir = scan(argv[2], true, false);
                showIR(ir);
            }
            break;

        default: //TODO: IF SPECIFIED MORE -H -R -P -S IMPLEMENT IN THAT PRIORITY
            cout << "Error: Input Format is bad. Type ./412fe -h for more information" << endl;
    }


    return 0;
}