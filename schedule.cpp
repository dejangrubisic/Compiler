//
// Created by Dejan Grubisic on 11/6/2019.
//

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
#include <sstream>
#include <set>
#include <algorithm>


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
static const char *RegTypeStr[] = {"nu", "pr", "vr", "sr"};

enum RegNum {
    R1, R2, R3, Rnone
};

struct InstructionIR {
    Operation opcode;
    int line_num;
    // R1, R2 => R3
    // 0 - NU, 1 - PR, 2 - VR, 3 - SR
    int registers[3][4];
    int constant;

    InstructionIR() {
        opcode = err;
        for (int i = 0; i < 12; ++i) {
            registers[i / 4][i % 4] = INT_MAX;
        }
        constant = -1;
    }

    InstructionIR(int line, Operation s):line_num(line), opcode(s){}

    InstructionIR(Operation s, int r1, int r2, int r3, int c, RegType rType = SR) {
        opcode = s;
        registers[R1][rType] = r1;
        registers[R2][rType] = r2;
        registers[R3][rType] = r3;
        constant = c;
    }

    string showReg(RegType RT) const {
        string rt = RegTypeStr[RT];//"r";//
        string s1 = (opcode == loadI || opcode == output) ? to_string(constant) : "";
        stringstream res;

        res << OperationStr[opcode] << " ";
        if(opcode != nop){
            res << ((registers[R1][RT] != INT_MAX) ? rt + to_string(registers[R1][RT]) : s1)
                << ((registers[R2][RT] != INT_MAX) ? ", " + rt + to_string(registers[R2][RT]) : "")
                << ((registers[R3][RT] != INT_MAX) ? " => " + rt + to_string(registers[R3][RT]) : "");// << endl;
        }

        return res.str();
    }

    void showAllRegs() const {
        int tmpReg;
        cout << line_num << " opcode " << "\t Const " << "\tR1 (SR,VR,PR,LU) " << "    R2   " << "\t  R3 " << endl;
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
    list <InstructionIR> instructions;
// showIR - print IR in human readable form

    void showInstructions() const {

        if (instructions.empty()) {
            cerr << "ERROR: Not valid instructions, run terminates." << endl;
            return;
        }

        for (const InstructionIR &inst : instructions) {

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

        for (const InstructionIR &inst : this->instructions) {
            cout << inst.showReg(rt) << endl;
        }
    }

    void showAllRegs() const {
        if (instructions.empty()) {
            cerr << "ERROR: Not valid instructions, run terminates." << endl;
            return;
        }

        for (const InstructionIR &inst : this->instructions) {
            inst.showAllRegs();
        }
    }
};

enum InputMode {
    s, h
};

// Parsing funcitons
tuple<InputMode, int, char *> manageInput(int argc, char *argv[]);


//***********************************************************************************************************
int strToInt(const string &str);

void takeErrWord(int line_num, const string &line, int &char_pos, string &err_word);

InstructionBlock scan(char *filename, bool check_semantics, bool display_tokens);

vector <pair<Category, string>> processLine(const string &line, int line_num, bool display_tokens);

pair <State, Category> nextState(State state, char new_char, string &lexeme);

InstructionIR parse(const vector <pair<Category, string>> &words, int line_num);

InstructionIR setupInstruction(InstructionIR &inst, const vector <pair<Category, string>> &words, int line_num);

InstructionIR
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

    void show() const {
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

void setVR(list<InstructionIR>::iterator ins, TableSrVr &table, int &vrName, const int &index) {

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

//***********************************************************************************************************
//TODO: Scheduling

struct PriorityQueue{
    set<pair<int, int>> q;
    
    PriorityQueue(){}

    bool empty(){
        return q.empty();
    }

    int size(){
        return q.size();
    }

    bool contains(int node_id){
        auto it = find_if(q.begin(), q.end(), [node_id](const pair<int,int>& p ){ return p.second == node_id; });
        if( it != q.end())
            return true;
        else
            return false;
    }

    pair<int, int> get(set< pair<int, int> >::iterator it){
        return *it;
    }

    void decKey(pair<int, int> member, int newKey){

        auto it = q.find(member);

        if (it != q.end()){
            q.erase(it);
            q.insert(make_pair(newKey, member.second));
        }
        else
            insertKey(newKey, member.second);

    }

    void insertKey(int key, int value){
        q.insert(make_pair(key, value));
    }

    set< pair<int, int> >::iterator erase(set< pair<int, int> >::iterator it){
        if(it == q.end())
            return it;
        return q.erase(it);
    }
    void erase(int node_id){
        auto it = find_if(q.begin(), q.end(), [node_id](const pair<int,int>& p ){ return p.second == node_id; });
        if(it != q.end())
            q.erase(it);
    }

    pair<int, int> getMin(){
        auto it = q.begin();
        pair<int, int> min = *it;
        q.erase(it);
        return min;
    }

    pair<int, int> getMax(){
        auto rit = q.rbegin();
        pair<int, int> max = *rit;
        q.erase(next(rit).base());
        return max;
    }

};

pair<list<int>, int> opUseDef(const InstructionIR &ins, RegType reg) {
//    cout<<"OP: "<<OperationStr[op]<<endl;
    switch (ins.opcode) {
        case load:
            return make_pair(list < int > {ins.registers[R1][reg]}, ins.registers[R3][reg]);
        case loadI:
            return make_pair(list < int > {}, ins.registers[R3][reg]);
        case store:
            if (ins.registers[R1][reg] != ins.registers[R3][reg])
                return make_pair(list < int > {ins.registers[R1][reg], ins.registers[R3][reg]}, INT_MAX);
            else
                return make_pair(list < int > {ins.registers[R1][reg]}, INT_MAX);
        case output:
            return make_pair(list < int > {}, INT_MAX);
        default: //ARITH
            if (ins.registers[R1][reg] != ins.registers[R2][reg])
                return make_pair(list < int > {ins.registers[R1][reg], ins.registers[R2][reg]}, ins.registers[R3][reg]);
            else
                return make_pair(list < int > {ins.registers[R1][reg]}, ins.registers[R3][reg]);
    }
}


struct Edge {
    int from;
    int to;
    int weight;
    bool typeIO;

    Edge(int from, int to, int weight, bool IO = false) : from(from), to(to), weight(weight), typeIO(IO) {}

    string show() const {
        stringstream ss;

        ss << from << " -> " << to << " [ label = \"" << weight << (typeIO ? " (IO)" : "") << "\" ];";
        return ss.str();
    }
};

struct Node {
    list <Edge> edgeOut;
    list <Edge> edgeIn;

    int id;
    const InstructionIR *ins;
    int priority;   // - priority of scheduling
    int memLoc;
    int latency;

    Node() {
        priority = 0;
        ins = 0;
        memLoc = INT_MAX;
    }

    void insertEdgeIn(Edge edge) {
        for (auto &x: edgeIn) {
            if (x.from == edge.from && x.to == edge.to) {
                if (x.weight < edge.weight) {
                    x.weight = edge.weight;
                    x.typeIO = edge.typeIO;
                }
                return;
            }
        }
        edgeIn.push_back(edge);
    }

    void insertEdgeOut(Edge edge) {
        for (auto &x: edgeOut) {
            if (x.from == edge.from && x.to == edge.to) {
                if (x.weight < edge.weight) {
                    x.weight = edge.weight;
                    x.typeIO = edge.typeIO;
                }
                return;
            }
        }
        edgeOut.push_back(edge);
    }
};

struct Graph {
    vector <Node> nodes;
    //    load, store, loadI, add, sub, mult, lshift, rshift, output, nop, err
    int latencyTable[10] = {5, 5, 1, 1, 1, 3, 1, 1, 1, 1};
    vector<int> regToNode;
    vector <array<int, 3>> listIO; //line_num, Opcode, mem_loc
    vector<int> vrConst; //key - vr, val = memLoc
    vector<int> vrLatency; //key - vr, val = latency of instr that defines vr
    
    Graph(int insNum, int regNum) : nodes(insNum), regToNode(regNum), vrConst(regNum, INT_MAX), vrLatency(regNum) {}

    void insertEdge(Edge edge) {
        nodes[edge.from].insertEdgeOut(edge);
        nodes[edge.to].insertEdgeIn(edge);
    }

    void addToListIO(const InstructionIR &ins, list<int> usedReg) {
        int tmp;
        switch (ins.opcode) {
            case load:
                tmp = vrConst[usedReg.front()];
                break;
            case store:
                tmp = vrConst[usedReg.back()];
                break;
            case output:
                tmp = ins.constant;
                break;
            default:;
        }
        listIO.push_back({ins.line_num, ins.opcode, tmp});

    }

    void addEdgesIO(const InstructionIR &ins, list<int> usedReg) {

        array<int, 3> lastInst;

        addToListIO(ins, usedReg);
        lastInst = listIO.back();
        nodes[lastInst[0]].memLoc = lastInst[2];    // write location into node in graph

        if (lastInst[1] == store) {
            for (auto rit = listIO.rbegin() + 1; rit != listIO.rend(); ++rit) {
                if (lastInst[2] == INT_MAX || (*rit)[2] == INT_MAX || lastInst[2] == (*rit)[2]) {
                    //TODO: this weights try to change on - output,store = -4
                    insertEdge(Edge((*rit)[0], lastInst[0], 1, true));
                    break;
                }
            }
        } else { //if( load || output)
            for (auto rit = listIO.rbegin() + 1; rit != listIO.rend(); ++rit) {
                if ((*rit)[1] == store && (lastInst[2] == INT_MAX || (*rit)[2] == INT_MAX || lastInst[2] == (*rit)[2])) {
                    insertEdge(Edge((*rit)[0], lastInst[0], 5, true));
                    break;
                }
            }
        }

    }

    void constantPropagation(const InstructionIR &ins, const pair<list<int>, int> &useDef) {

        if (ins.opcode == loadI) {
            vrConst[useDef.second] = ins.constant;
            nodes[ins.line_num].memLoc = ins.constant;
            return;
        }
        //add, sub, mult, lshift, rshift

        if (useDef.first.size() == 1 && vrConst[useDef.first.front()] != INT_MAX) {

            switch (ins.opcode) {
                case add:
                    vrConst[useDef.second] = vrConst[useDef.first.front()] << 1;
                    break;
                case sub:
                    vrConst[useDef.second] = 0;
                    break;
                case mult:
                    vrConst[useDef.second] = vrConst[useDef.first.front()] * vrConst[useDef.first.front()];
                    break;
                case lshift:
                    vrConst[useDef.second] = vrConst[useDef.first.front()] << vrConst[useDef.first.front()];
                    break;
                case rshift:
                    vrConst[useDef.second] = vrConst[useDef.first.front()] >> vrConst[useDef.first.front()];
                    break;
                default:;
            }

        } else if (useDef.first.size() == 2 &&
                   vrConst[useDef.first.front()] != INT_MAX && vrConst[useDef.first.back()] != INT_MAX) {

            switch (ins.opcode) {
                case add:
                    vrConst[useDef.second] = vrConst[useDef.first.front()] + vrConst[useDef.first.back()];
                    break;
                case sub:
                    vrConst[useDef.second] = vrConst[useDef.first.front()] - vrConst[useDef.first.back()];
                    break;
                case mult:
                    vrConst[useDef.second] = vrConst[useDef.first.front()] * vrConst[useDef.first.back()];
                    break;
                case lshift:
                    vrConst[useDef.second] = vrConst[useDef.first.front()] << vrConst[useDef.first.back()];
                    break;
                case rshift:
                    vrConst[useDef.second] = vrConst[useDef.first.front()] >> vrConst[useDef.first.back()];
                    break;
                default:;
            }
        }
        nodes[ins.line_num].memLoc = vrConst[useDef.second];
        return;
    }

    void insertNode(const InstructionIR &ins) {
        pair<list<int>, int> useDef = opUseDef(ins, VR);
        int id = ins.line_num;

        if (useDef.second != INT_MAX) {
            regToNode[useDef.second] = id;
            vrLatency[useDef.second] = latencyTable[ins.opcode];
        }
        nodes[id].id = id;
        nodes[id].ins = &ins;
        nodes[id].latency = latencyTable[ins.opcode];


        for (const auto &x: useDef.first) {
            insertEdge(Edge(regToNode[x], id, vrLatency[x] ));
        }

        if (ins.opcode == load || ins.opcode == store || ins.opcode == output)
            addEdgesIO(ins, useDef.first);
        else
            constantPropagation(ins, useDef);


    }

    void relax(const Edge edge, PriorityQueue &q){
        int oldDist = nodes[edge.from].priority;    // distTo[edge.from];
        int newDist = nodes[edge.to].priority + edge.weight; // distTo[edge.to]

        if( oldDist < newDist ){
            nodes[edge.from].priority = newDist;
            q.decKey(make_pair(oldDist, edge.from), newDist);
        }
    }

    void computePriority(){
        PriorityQueue qmin;
        pair<int, int> nodeMin; // dist, line_num

        for( auto rit = nodes.rbegin(); rit != nodes.rend(); rit++){
            //all nodes from back to start
            if(rit->priority != 0)
                continue;

            qmin.insertKey(0, rit->id);

            while(!qmin.empty()){
                nodeMin = qmin.getMin();

                for(const auto &edge: nodes[nodeMin.second].edgeIn ){
                    relax(edge, qmin);
                }
            }
        }

        return;
    }

    pair<int, int> findReadyIns(set<pair<int,int>> &q){

        int funcUnit[3] = {INT_MAX, INT_MAX, INT_MAX};
        int cnt = 0;
        int insId;

        for(auto it=q.begin(); it != q.end() || cnt == 2;it++){
            insId = it->second;

            if( nodes[insId].ins->opcode == load || nodes[insId].ins->opcode == store ){
                if(funcUnit[0] == INT_MAX){
                    funcUnit[0] = insId;
                    cnt++;
                }
            }else if(nodes[insId].ins->opcode == mult){
                if(funcUnit[1] == INT_MAX){
                    funcUnit[1] = insId;
                    cnt++;
                }
            }else{ // not load, store or mult
                if(funcUnit[2] == INT_MAX){
                    funcUnit[2] = insId;
                }else{
                    return make_pair(funcUnit[2], insId);
                }
                cnt++;
            }
        }

        if(funcUnit[0] == INT_MAX){
            return make_pair(funcUnit[2], funcUnit[1]);
        }
        else if(funcUnit[1] == INT_MAX){
            return make_pair(funcUnit[0], funcUnit[2]);
        }
        else{
            return make_pair(funcUnit[0], funcUnit[1]);
        }

    }

    void updateQReady(PriorityQueue &qActive, PriorityQueue &qReady, int cycle, vector<int> depCnt){
        list<int> finishedIns;
        auto it=qActive.q.begin();

        for(; it != qActive.q.end() && it->first <= cycle; it++){
            finishedIns.push_back(it->second);
        }

        qActive.q.erase(qActive.q.begin(), it);

        for(const auto &ins: finishedIns){
            for(const auto &eOut: nodes[ins].edgeOut) {
                if( --depCnt[eOut.to] == 0){
                    qReady.insertKey(-nodes[eOut.to].priority, eOut.to);
                }
            }
        }

    }

    list< pair<InstructionIR, InstructionIR> > schedule(){

        list<pair<InstructionIR, InstructionIR>> listTwoIns;
        pair<InstructionIR, InstructionIR> currentIns;
        pair<int, int> twoInsIds;
        vector<int> dependencyCounter(nodes.size());

        int cycle = 0;
        PriorityQueue qReady;    //priority, ins_id
        PriorityQueue qActive;     //finish_time, ins_id


        for( const auto &node: nodes){
            dependencyCounter[node.id] = node.edgeIn.size();
            if( node.edgeIn.empty()){
                qReady.insertKey(-node.priority, node.id);
            }
        }

        while( !qReady.empty() || !qActive.empty() ){

            twoInsIds = findReadyIns(qReady.q);

            currentIns = make_pair(InstructionIR(cycle, nop), InstructionIR(cycle, nop));

            if(twoInsIds.first != INT_MAX){
                qActive.insertKey(cycle + nodes[twoInsIds.first].latency, twoInsIds.first);
                qReady.erase(twoInsIds.first);
                currentIns.first = *nodes[twoInsIds.first].ins;
            }

            if(twoInsIds.second != INT_MAX){
                qActive.insertKey(cycle + nodes[twoInsIds.second].latency, twoInsIds.second);
                qReady.erase(twoInsIds.second);
                currentIns.second = *nodes[twoInsIds.second].ins;
            }

            updateQReady(qActive, qReady, cycle, dependencyCounter);

            listTwoIns.push_back( currentIns );
            cycle++;
        }



        return listTwoIns;
    }

    void show(string fileName = "") const {
        stringstream node_stream;
        stringstream edge_stream;

        int vrDef;
        node_stream << "digraph G {" << endl;
        for (const auto &x: nodes) {
            if (x.ins == 0) {
                continue;
            }
            vrDef = opUseDef(*x.ins, VR).second;

            node_stream << x.id << " [ label = \"" << x.ins->line_num << ". " << x.ins->showReg(VR)
                        << "\n Mem = " << (x.memLoc != INT_MAX ? x.memLoc : -1)
                        << " | Pri = " << (x.priority)
                        << "\" ];" << endl;

            for (const auto &e: x.edgeOut)
                edge_stream << e.show() << endl;
        }
        node_stream << edge_stream.str();
        node_stream << "}" << endl;

        cout << node_stream.str();


        cout << "fileName = " << fileName << endl;

        if (fileName != "") {
            ofstream graphFile(fileName, ios::out);

            if (graphFile.is_open()) {
                graphFile << node_stream.str();
//                graphFile.flush();
                graphFile.close();
            } else {
                cerr << "Not Created Graphviz file : " << fileName << endl;
            }

        }

    }

};

Graph createDependenceGraph(InstructionBlock &block, int regNum) {

    Graph dg(block.instructions.size(), regNum);
    int i = 0;
    for (auto &ins: block.instructions) {
        ins.line_num = i++;
        dg.insertNode(ins);
//        dg.show();

    }
//    int *p;
//    p[545454444] = 5;
    return dg;
}

void schedule(InstructionBlock &block, int regNum, string gFileName) {

    Graph dg(createDependenceGraph(block, regNum));
    list<pair<InstructionIR, InstructionIR>> newSchedule;

    dg.computePriority();

    dg.show("./graphviz/g_" + gFileName.substr(7, 9) + ".txt");

    newSchedule = dg.schedule();
    for(const auto &ins: newSchedule){
        cout<< "[ "<<ins.first.showReg(VR) <<"\t\t, "<<ins.second.showReg(VR)<<" ]"<<endl;
    }
}












//***********************************************************************************************************
// HELPER FUNCTIONS

// manageInput - parse command line
tuple<InputMode, int, char *> manageInput(int argc, char *argv[]) {

    tuple<InputMode, int, char *> input(make_tuple(h, 0, argv[argc - 1]));

    if (argc == 2 && strcmp(argv[1], "-h") != 0) {
        get<0>(input) = s;
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
        if (str[i] < '0' || str[i] > '9')
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


//***********************************************************************************************************
// MAIN FUNCTIONS Implementations

// Scan - creates tokens from input file, check lexical errors, and check_semantics(make IR)
InstructionBlock scan(char *filename, bool check_semantics = false, bool display_tokens = false) {

    ifstream input_stream(filename, ifstream::in);
    if (!input_stream) cerr << "Cannot open input file: \"" << filename << "\"" << endl;

    string line;
    int line_num = 0;
    int inst_count = 0;
    vector <pair<Category, string>> result;

    InstructionIR inst;
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
InstructionIR parse(const vector <pair<Category, string>> &words, int line_num) {

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
            return InstructionIR();
    }
}

// setupInstruction - creates instruction IR from tokens
InstructionIR setupInstruction(InstructionIR &inst, const vector <pair<Category, string>> &words, int line_num) {
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
InstructionIR
checkSemantics(const vector <pair<Category, string>> &words, const vector <Category> &grammar, int line_num) {

    InstructionIR inst;
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
                 "    <file_name>  prints optimized reordered ILOC program \n";
            break;

        case s:
            irBlock = scan(get<2>(input), true, false);
//            irBlock.showAllRegs();
//            irBlock.showIR();

            int regNum = registerRenaming(irBlock);
//            cout << "reg REName pass" << endl;
//            irBlock.showAllRegs();
//            irBlock.showIR(SR);
//            irBlock.showIR(VR);
            schedule(irBlock, regNum, get<2>(input));

            break;
    }

    return 0;
}
