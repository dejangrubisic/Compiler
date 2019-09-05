#include <iostream>
#include <fstream>
#include <cstdio>
#include <cerrno>
#include <string.h>
#include <list>
#include <array>

using namespace std;

enum Operations {
    load, store, loadI, add, sub, mult, lshift, rshift, output, nop
};
enum State {
    s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14,
    s15, s16, s17, s18, s19, s20, s21, s22, s23, s24, s25, s26, s27,
    s28, s29, s30, s31, s32, s33, s34, s35, s36, s37, s38, s39,
    sErr, COMMENT, MEMOP, LOADI, ARITHOP, OUTPUT, NOP, INTO, COMMA, CONST, REG,
    CONSTINTO, REGINTO, REGCOMMA};

static const char *CategoryStr[] = {"MEMOP", "LOADI", "ARITHOP", "OUTPUT", "NOP", "INTO", "COMMA", "CONST", "REG"};



class instruction {
    int fields[13];

    instruction() {
        for (int i = 0; i < 12; ++i) {
            fields[i] = 0;
        }
    }
};

State nextState(State s, char new_char, string &lexeme);
list<pair<State, string>> processLine(string line, int current_line);

// FUNCTIONS

bool scan(char *filename) {

    ifstream input_stream(filename, ifstream::in);
    string line;
    int current_line = 1;
    list< list < pair<State, string> > > result;

    if (!input_stream) cerr << "Cannot open input file";

    while (getline(input_stream, line)) {

//        cout << "LINE: " << line << endl;
        result.push_back( processLine(line, current_line) );


        current_line++;
    }
    cout << --current_line << ": < ENDFILE  , \"\" >" << endl;
    if (current_line == 1){
        cout << "WARNING: ILOC file contained no operations." << endl;
    }

    return true;
}

list<pair<State, string>> processLine(string line, int current_line){
    string lexeme;
    State s = s0;
    list< pair<State, string> > tokens;

    line.push_back('\n');

    for (int char_pos = 0; char_pos < int(line.length()); ++char_pos) {
//        cout << s <<"->";
        s = nextState(s, line[char_pos], lexeme);
//        cout << s << endl;

        if (s >= MEMOP) {
            if ( s <= REG){
                cout << current_line << ": < " << CategoryStr[s - MEMOP] << "\t, \"" << lexeme << "\" >"<< endl;
                tokens.push_back(make_pair(s, lexeme));

            }else if(s == CONSTINTO || s == REGINTO) {
                cout << current_line << ": < " << ((s == CONSTINTO)?"CONST":"REG") << "\t, \"" << lexeme << "\" >" << endl;
                cout << current_line << ": < INTO\t, \"=>\" >" << endl;
                tokens.push_back(make_pair(s, lexeme));
                tokens.push_back(make_pair(INTO, "=>"));

            }else if (s == REGCOMMA) {
                cout << current_line << ": < REG\t, \"" << lexeme <<"\" >"<< endl;
                cout << current_line << ": < COMMA\t, \",\" >" << endl;
                tokens.push_back(make_pair(s, lexeme));
                tokens.push_back(make_pair(COMMA, ","));
            }
            s = s0;

        } else if (s == sErr) {
            cout << "Lexical error: " << current_line << ": \"" << lexeme <<
                 "\" is not a valid word." << endl;
            tokens.push_front(make_pair(sErr,""));
            break;
        } else if (s == COMMENT) {
            break;
        }

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
                scan(argv[2]);

            } else if (strcmp(argv[1], "-p") == 0) {
                cout << "Make IR from File Specified" << endl;

            } else if (strcmp(argv[1], "-r") == 0) {
                cout << "Make IR and write info from File Specified" << endl;

            }
            break;

        default: //TODO: IF SPECIFIED MORE -H -R -P -S IMPLEMENT IN THAT PRIORITY
            cout << "Error: Input Format is bad. Type ./412fe -h for more information" << endl;
    }


    return 0;
}