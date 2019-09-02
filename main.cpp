#include <iostream>
#include <fstream>
#include <cstdio>
#include <cerrno>
#include <string.h>
#include <list>
#include <array>

using namespace std;

#define MAX_LENGTH_WORD 10

enum Operations {
    load, store, loadI, add, sub, mult, lshift, rshift, output, nop
};
enum State {
    s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12,
    s13, s14, s15, s16, s17, s18, s19, s20, s21, s22, s23, s24,
    s25, s26, s27, s28, s29, s30, s31, s32, s33, s34, s35, s36, s37, s38,
    sErr, COMMENT, MEMOP, LOADI, ARITHOP, OUTPUT, NOP, INTO, COMMA, CONST, REG, CONSTCOMMA, REGCOMMA };

static const char *CategoryStr[] = {"MEMOP", "LOADI", "ARITHOP", "OUTPUT", "NOP", "INTO", "COMMA", "CONST", "REG"};

class classWord {
    int category;
    std::string lexeme; //TODO: maybe try this with char[Max_length_word]
    classWord(int cat, std::string &lex) : category(cat), lexeme(lex) {}

};


class instruction {
    int fields[13];

    instruction() {
        for (int i = 0; i < 12; ++i) {
            fields[i] = 0;
        }
    }
};

State processCharacter(State s, char new_char, string &lexeme);



// FUNCTIONS

bool scan(char *filename, int total_lines = 10) {
    cout << "SCAN" << endl;

    ifstream input_stream(filename, ifstream::in);
    string line;
    string lexeme;
    int current_line = 1;

    list <array<int, 3>> error_lines;
    State s = s0, s_prev;

    if (!input_stream) cerr << "Cannot open input file";

    while (getline(input_stream, line) && current_line < total_lines) {
        cout << current_line << ". " << line << endl;

        lexeme.clear();
        s = s0;

        for (int char_pos = 0; char_pos < int(line.length()); ++char_pos) {
            s_prev = s;
            s = processCharacter(s, line[char_pos], lexeme);

            if (s >= MEMOP) {
                if (s == CONSTCOMMA || s == REGCOMMA) {
                    lexeme.pop_back();
                    cout << current_line << " " << CategoryStr[s + (REG - REGCOMMA) - MEMOP] << " " << lexeme << endl;
                    cout << current_line << " " << CategoryStr[COMMA - MEMOP] << " " << ',' << endl;
                }else{
                    cout << current_line << " " << CategoryStr[s - MEMOP] << " " << lexeme << endl;
                }

                s = s0;
                lexeme.clear();
            } else if (s == sErr) {
                cout << "ERROR: line " << current_line << ", " << (char_pos + 1) <<
                     " ---> state: " << s_prev << endl;
                error_lines.push_back(array < int, 3 > {current_line, char_pos + 1, s_prev});
                break;
            } else if (s == COMMENT) {
                break;
            }

        }


        current_line++;
    }


    return true;
}

State processCharacter(State s, char new_char, string &lexeme) {

//    cout << "s=" << s << " c=" << new_char <<endl;
    lexeme.push_back(new_char);

    switch (s) {

        case s0:
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
            } else if (new_char == ' ') {
                s = s0;
            } else if (new_char >= '0' && new_char <= '9') {
                s = s36;
            } else if (new_char == '/') {
                s = s38;
            } else {
                s = sErr;
            }
            break;

        case s1:
            if (new_char == 't') {
                s = s2;
            } else if (new_char == 'u') {
                s = s6;
            } else {
                s = sErr;
            }
            break;

        case s2:
            if (new_char == 'o') {
                s = s3;
            } else {
                s = sErr;
            }
            break;

        case s3:
            if (new_char == 'r') {
                s = s4;
            } else {
                s = sErr;
            }
            break;

        case s4:
            if (new_char == 'e') {
                s = s5;
            } else {
                s = sErr;
            }
            break;

        case s5:
            if (new_char == ' ') {
                s = MEMOP;
            } else {
                s = sErr;
            }
            break;

        case s6:
            if (new_char == 'b') {
                s = s7;
            } else {
                s = sErr;
            }
            break;

        case s7:
            if (new_char == ' ') {
                s = ARITHOP;
            } else {
                s = sErr;
            }
            break;

        case s8:
            if (new_char == 'o') {
                s = s9;
            } else if (new_char == 's') {
                s = s14;
            } else {
                s = sErr;
            }
            break;

        case s9:
            if (new_char == 'a') {
                s = s10;
            } else {
                s = sErr;
            }
            break;

        case s10:
            if (new_char == 'd') {
                s = s11;
            } else {
                s = sErr;
            }
            break;

        case s11:
            if (new_char == 'I') {
                s = s12;
            } else if (new_char == ' ') {
                s = MEMOP;
            } else {
                s = sErr;
            }
            break;

        case s12:
            if (new_char == ' ') {
                s = LOADI;
            } else {
                s = sErr;
            }
            break;

        case s13:
            if (new_char == 's') {
                s = s14;
            } else if (new_char >= '0' && new_char <= '9') {
                s = s37;
            } else {
                s = sErr;
            }
            break;

        case s14:
            if (new_char == 'h') {
                s = s15;
            } else {
                s = sErr;
            }
            break;

        case s15:
            if (new_char == 'i') {
                s = s16;
            } else {
                s = sErr;
            }
            break;

        case s16:
            if (new_char == 'f') {
                s = s17;
            } else {
                s = sErr;
            }
            break;

        case s17:
            if (new_char == 't') {
                s = s18;
            } else {
                s = sErr;
            }
            break;

        case s18:
            if (new_char == ' ') {
                s = ARITHOP;
            } else {
                s = sErr;
            }
            break;

        case s19:
            if (new_char == 'u') {
                s = s20;
            } else {
                s = sErr;
            }
            break;

        case s20:
            if (new_char == 'l') {
                s = s21;
            } else {
                s = sErr;
            }
            break;

        case s21:
            if (new_char == 't') {
                s = s18;
            } else {
                s = sErr;
            }
            break;

        case s22:
            if (new_char == 'd') {
                s = s23;
            } else {
                s = sErr;
            }
            break;

        case s23:
            if (new_char == 'd') {
                s = s24;
            } else {
                s = sErr;
            }
            break;

        case s24:
            if (new_char == ' ') {
                s = ARITHOP;
            } else {
                s = sErr;
            }
            break;

        case s25:
            if (new_char == 'o') {
                s = s26;
            } else {
                s = sErr;
            }
            break;

        case s26:
            if (new_char == 'p') {
                s = s27;
            } else {
                s = sErr;
            }
            break;

        case s27:
            if (new_char == ' ' || new_char == '\n') {
                s = NOP;
            } else {
                s = sErr;
            }
            break;

        case s28:
            if (new_char == 'u') {
                s = s29;
            } else {
                s = sErr;
            }
            break;

        case s29:
            if (new_char == 't') {
                s = s30;
            } else {
                s = sErr;
            }
            break;

        case s30:
            if (new_char == 'p') {
                s = s31;
            } else {
                s = sErr;
            }
            break;

        case s31:
            if (new_char == 'u') {
                s = s32;
            } else {
                s = sErr;
            }
            break;

        case s32:
            if (new_char == 't') {
                s = s33;
            } else {
                s = sErr;
            }
            break;

        case s33:
            if (new_char == ' ') {
                s = OUTPUT;
            } else {
                s = sErr;
            }
            break;

        case s34:
            if (new_char == '>') {
                s = s35;
            } else {
                s = sErr;
            }
            break;

        case s35:   //TODO: Check if there has to be ' ' after =>, if not merge s35 to s34
            if (new_char == ' ') {
                s = INTO;
            } else {
                s = sErr;
            }
            break;

        case s36: // Handle Constant
            if (new_char >= '0' && new_char <= '9') {
                s = s36;
            } else if (new_char == ' ') {
                s = CONST;
            } else if (new_char == ',') {
                s = CONSTCOMMA;
            } else {
                s = sErr;
            }
            break;

        case s37: // Handle Register
            if (new_char >= '0' && new_char <= '9') {
                s = s37;
            } else if (new_char == ' ') {
                s = REG;
            } else if (new_char == ',') {
                s = REGCOMMA;
            } else {
                s = sErr;
            }
            break;
        case s38: // Handle Comments
            if (new_char == '/') {
                s = COMMENT;
            } else {
                s = sErr;
            }
            break;
        default:
            s = sErr;
    }

//    cout<< "new state: "<< s <<endl;
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
                cout << "Read File Specified" << endl;
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
