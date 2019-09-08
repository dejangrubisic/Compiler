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

enum State {
    s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14,
    s15, s16, s17, s18, s19, s20, s21, s22, s23, s24, s25, s26, s27,
    s28, s29, s30, s31, s32, s33, s34, s35, s36, s37, sComma, sErr};

enum Category {EMPTY, MEMOP, LOADI, ARITHOP, OUTPUT, NOP, INTO, COMMA, CONST, REG, COMMENT};
static const char *CategoryStr[] = { "EMPTY", "MEMOP", "LOADI", "ARITHOP", "OUTPUT", "NOP", "INTO", "COMMA", "CONST", "REG"};

enum Operation{ load, store, loadI, add, sub, mult, lshift, rshift, output, nop, err};
static const char *OperationStr[] = {"load", "store", "loadI", "add", "sub", "mult", "lshift", "rshift", "output", "nop"};

struct InctructionIR{
    Operation opcode;
    int line_num;
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
enum InputMode{ s, p, r, h};

int strToInt(const string &str);
InstructionBlock scan(char * filename, bool check_semantics, bool display_tokens);
vector<pair<Category, string>> processLine(string line, int line_num, bool display_tokens);
pair< State, Category> nextState(State state, char new_char, string &lexeme);
InctructionIR parse(const vector < pair<Category, string> > &words , int line_num);
InctructionIR setupInstruction(InctructionIR &inst, const vector < pair<Category , string> > words, int line_num);
InctructionIR checkSemantics(const vector < pair<Category, string> > &words, const vector<Category> grammar, int line_num);

// HELPER FUNCTIONS

// manageInput - parse command line
pair<InputMode, char *> manageInput(int argc, char *argv[]){

    pair<InputMode, char *> input(make_pair(s, argv[argc-1] ));

    if(argc == 2){
        input.first = p;
        return input;
    }

    for (int i = 1; i < argc - 1; ++i) {

        if (strcmp(argv[i], "-h") == 0) {
            input.first = h;
        } else if (strcmp(argv[i], "-r") == 0 && input.first < r) {
            input.first = r;
        } else if (strcmp(argv[i], "-p") == 0 && input.first < p) {
            input.first = p;
        } else if (strcmp(argv[i], "-s") == 0 && input.first < s) {
            input.first = s;
        }
    }

    return input;
}

// If error happen it gets whole word as error word
void takeErrWord(int line_num, string &line, int &char_pos, string &err_word);

// strToInt - converts string(const | r+const) to int
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

// strToOperation - converts string to enum Operation
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

// MAIN FUNCTIONS

// Scan - creates tokens from input file, check lexical errors, and check_semantics(make IR)
InstructionBlock scan(char * filename, bool check_semantics = false, bool display_tokens = false) {

    ifstream input_stream(filename, ifstream::in);
    if (!input_stream) cerr << "Cannot open input file: \"" << filename << "\"" << endl;

    string line;
    int line_num = 0;
    int inst_count = 0;
    vector < pair<Category, string> > result;

    InctructionIR inst;
    InstructionBlock block;
    int error_count = 0;


    while (getline(input_stream, line)) {
//        cout << "LINE: " << line << endl;
        result = processLine(line, ++line_num, display_tokens);

        if(result.empty())
            continue;

        inst_count++;

        // Parse Part
        if(check_semantics){
            inst = parse(result, line_num);

            if( inst.opcode != err ){
                if(error_count == 0)
                    block.instructions.push_back(inst);
            }else{
                error_count++;
            }
        }

    }

    if(display_tokens){
        cout << line_num << ": < ENDFILE  , \"\" >" << endl;
    }

    if(check_semantics){
        if(error_count == 0){
            cout << endl << "Parse succeeded, finding "<< inst_count - error_count <<" ILOC operations." << endl;
        }else{
            cerr << endl << "Parser found "<< error_count <<" Syntax errors in " << line_num <<" lines of input." << endl;
        }
    }

    if (inst_count == 0){
        cout << "WARNING: ILOC file contained no operations." << endl;
    }


    return (error_count == 0) ? block : InstructionBlock();
}

// processLine - creates vector of tokens(Category, string)
vector<pair<Category, string>> processLine(string line, int line_num, bool display_tokens){

    string lexeme, err_str;
    pair< State, Category > s;
    vector< pair<Category, string> > tokens;
    ostringstream stream;

//    cout << "PROCSS LINE: " << line;
    //Initialize first state
    s.first = s0;
    line.push_back('\n');


    for (int char_pos = 0; char_pos < int(line.length()); ++char_pos) {
//        cout << s <<"->";
        s = nextState(s.first, line[char_pos], lexeme);
//        cout << s << endl;


        if ( s.first != sErr ){
            if( s.second != EMPTY ){
                // Category {EMPTY, MEMOP, LOADI, ARITHOP, OUTPUT, NOP, INTO, COMMA, CONST, REG, COMMENT};
                if (s.second == COMMENT) {
                    break;
                }
                else{   //ALL OPERATIONS, REG, CONST
                    stream << line_num << ": < " << CategoryStr[s.second] << "\t, \"" << lexeme << "\" >"<< endl;
                    tokens.push_back(make_pair(s.second, lexeme));

                    if(s.second == REG && s.first == sComma){
                        stream << line_num << ": < COMMA\t, \",\" >" << endl;
                        tokens.push_back(make_pair(COMMA, ","));
                        s.first = s0;
                    }
                }
            }

        }
        else{ // Error States
            if( s.second != EMPTY ){ //Accept lexeme[0:n-1] and make err message
                err_str = lexeme;
                lexeme.pop_back();  // word you accept
                stream << line_num << ": < " << CategoryStr[s.second] << "\t, \"" << lexeme << "\" >"<< endl;
                tokens.push_back(make_pair(s.second, lexeme));

                takeErrWord(line_num, line, char_pos, err_str);
                tokens.push_back(make_pair(EMPTY, err_str));
                s.first = s0;
            }
            else{ // make err message
                takeErrWord(line_num, line, char_pos, lexeme);
                tokens.push_back(make_pair(EMPTY, lexeme));
                s.first = s0;
            }
        }



    }
    if(display_tokens){
        cout << stream.str();
    }
    return tokens;
}

void takeErrWord(int line_num, string &line, int &char_pos, string &err_word){

    while(line[char_pos] != ' ' && line[char_pos] != '\t' && line[char_pos] != '\n'){
        err_word.push_back(line[++char_pos]);
    }
    if(line[char_pos] == '\n')
        err_word.pop_back();

    cerr << "ERROR: " << line_num << ": Lexical: \"" << err_word <<
         "\" is not a valid word." << endl;

    return;
}


// nextState - DFA of trasitions
pair< State, Category> nextState( State state, char new_char, string &lexeme) {

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
            } else if(new_char == '/'){
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
            lexeme = "=>";
            if (new_char == '>') {
                cat = INTO;
                state = s0;
            } else {
                state = sErr;
            }
            break;

        case s35: // Handle Constant
            if (new_char >= '0' && new_char <= '9') {
                state = s35;
                lexeme.push_back(new_char);
            } else if (new_char == ' ' || new_char == '\t' || new_char == '\n') {
                cat = CONST;
                state = s0;
            } else if (new_char == '/'){
                cat = CONST;
                state = s37;
            } else if (new_char == '='){
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
            }  else if (new_char == '/'){
                cat = REG;
                state = s37;
            } else if (new_char == ',') {
                cat = REG;
                state = sComma; // send both REG and COMMA
            } else if (new_char == '='){
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
                state = s0;
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
InctructionIR parse(const vector < pair<Category , string> > &words, int line_num ){

    switch (words[0].first){
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
            cerr << "ERROR: " << line_num << ": Syntax: Undefined operation \"" << words[0].second <<"\""<< endl;
            return InctructionIR();
    }
}

// setupInstruction - creates instruction IR from tokens
InctructionIR setupInstruction(InctructionIR &inst, const vector < pair<Category , string> > words, int line_num){
    // SETUP INSTRUCTION_IR

    inst.opcode = strToOperation(words[0].second);
    inst.line_num = line_num;
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

// checkSemantics - compare grammar with tokens
InctructionIR checkSemantics(const vector < pair<Category , string> > &words, const vector<Category> grammar, int line_num){

    InctructionIR inst;
    int i;

    if (words.size() < grammar.size()){
        cerr << "ERROR: " << line_num << ": Syntax: too few words. Format is: ";

        for (int i = 0; i < int(grammar.size()) ; ++i) {
            cout << CategoryStr[grammar[i]] <<" ";
        }
        cout << endl;
        return inst;

    }

    for (i = 1; i < int(grammar.size()); ++i) {
//            cout << words[i].first << " -- gramar: "<< grammar[i] << endl;
        if(words[i].first != grammar[i]){
            cerr << "ERROR: " << line_num << ": Syntax: Invalid word \"" << words[i].second <<
            "\" Should be: "<< CategoryStr[grammar[i]] << endl;
            return inst;
        }
    }
    if(words.size() > grammar.size()){
        cerr << "ERROR: " << line_num << ": Syntax: too many words :\"" << words[i].second <<"\""<<endl;
        return inst;
    }

    return  setupInstruction(inst, words, line_num);
}

// showIR - print IR in human readable form
void showIR(InstructionBlock ir){

    if(ir.instructions.empty()){
        cerr << "ERROR: Not valid instructions, run terminates." << endl;
        return;
    }

    for (const InctructionIR &inst : ir.instructions ) {

        switch (inst.opcode){

            case load:
                cout << "load\t [ sr" << inst.registers[0][3] << " ], [ ], [ sr" << inst.registers[2][3] << " ]" << endl;
                break;

            case store:
                cout << "store\t [ sr" << inst.registers[0][3] << " ], [ ], [ sr" << inst.registers[2][3] << " ]" << endl;
                break;

            case loadI:
                cout << "loadI\t [ val " << inst.constant << " ], [ ], [ sr" << inst.registers[2][3] << " ]" << endl;
                break;

            case output:
                cout << OperationStr[inst.opcode] <<"\t [ val " << inst.constant << " ], [ ], [ ]" << endl;
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

    pair< InputMode , char * > input = manageInput(argc, argv);
    InstructionBlock ir;

    switch (input.first){

        case h:
            cout << "Optional flags:\n" <<
                    "        -h       prints this message\n" <<
                    "        -s       prints tokens in token stream\n" <<
                    "        -p       invokes parser and reports on success or failure\n" <<
                    "        -r       prints human readable version of parser's IR\n";
            break;

        case r:
            ir = scan(input.second, true, false);
            showIR(ir);
            break;

        case p:
            scan(input.second, true, false);
            break;

        case s:
            scan(input.second, false, true);
            break;
    }

    return 0;
}