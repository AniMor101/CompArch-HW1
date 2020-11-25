//
// Created by khour on 24/11/2020.
//
#include <iostream>
#include <vector>
#include <math.h>
#include <bitset>
#include "bp_api.h"
/* 046267 Computer Architecture - Winter 20/21 - HW #1                  */
/* This file should hold your implementation of the predictor simulator */

typedef enum {
    SNT,
    WNT,
    WT,
    ST
}State;

class StateMachine{
private:
    State state;
    //privateMethods:
    State updateStateTaken(){
        switch (state) {
            case SNT:
                state=WNT;
                break;
            case WNT:
                state=WT;
                break;
            case WT:
                state=ST;
                break;
            default:
                break;
        }
        return state;
    }
    State updateStateNotTaken(){
        switch (state) {
            case ST:
                state=WT;
                break;
            case WT:
                state=WNT;
                break;
            case WNT:
                state=SNT;
                break;
            default:
                break;
        }
        return state;
    }
    StateMachine(){
        state=SNT;
    }
public:
    //constructors:
    StateMachine(State state1){
        state=state1;
    }
    //methods
    State getState(){
       return state;
    }
    State updateState(bool taken){
        if(taken){
         return updateStateTaken();
        }
        return updateStateNotTaken();
    }
};

class StateMachinesVector{
private:
    std::vector<StateMachine> stateMachines;
    unsigned size;
public:
    //constructors:
    StateMachinesVector(){}
    StateMachinesVector(unsigned size){
        stateMachines= std::vector<StateMachine>(size,SNT);
    }
    StateMachinesVector(unsigned size,State state){
        stateMachines= std::vector<StateMachine>(size,state);
    }
    //methods:
    State getStateAtIndex(unsigned index){
        return stateMachines.at(index).getState();
    }
    unsigned getSize(){
        return size;
    }
    State updateStateAtIndex(unsigned  index,bool taken){
        return stateMachines.at(index).updateState(taken);
    }

};

class History{
private:
    uint8_t history;
    unsigned size;
    History(){ //we dont want it to be called because 1<=size<=8
        history=0x0;
        size=0;
    }
public:
    //constructor:
    History(unsigned size1){
        history=0x0;
        size=size1;
    }
    //methods:
    uint8_t getHistory(){
        return history;
    }
    unsigned getSize(){
        return size;
    }
    uint8_t updateHistory(bool taken){
        history%=(uint8_t)pow(2,size-1);
        history= (history*2) ;
        if(taken){
            history++;
        }
        return history;
    }
};

class BranchCmd{
private:
    uint32_t tag;
    uint32_t target;
    bool valid;
    History history;
    StateMachinesVector stateMachinesVector;
    BranchCmd():history(0) {
        tag=0x0;
        target=0x0;
        }
public:
    //Constructors:
    BranchCmd(uint32_t tag1,uint32_t target1,unsigned historySize):history(historySize){
        tag=tag1;
        target=target1;
        valid=false;
        stateMachinesVector=StateMachinesVector(pow(2,historySize));
    }
    uint32_t getTag(){
        return tag;
    }
    uint32_t getTarget(){
        return target;
    }
    History getHistory(){
        return history;
    }
    bool getValid(){
        return valid;
    }

    History updateBranchHistory(bool taken){
        return history.updateHistory(taken);
    }
    uint32_t updateBranchTarget(uint32_t target1){
        target=target1;
    }
    bool updateValid(bool valid1){
        valid=valid1;
    }
    BranchCmd updateBranch(bool taken, uint32_t target1){
        updateBranchHistory(taken);
        updateBranchTarget(target1);
        return *this;
    }
};

class BTB{
private:
    unsigned btbSize;
    unsigned historySize;
    unsigned tagSize;
    unsigned fsmState;
    bool isGlobalHist;
    bool isGlobalTable;
    int isShare;
    std::vector<BranchCmd> branchesVect;
    History globalHistory;
    StateMachine globalStateMachine;
    StateMachinesVector stateMachineVec;
    unsigned branchesCounter;
    unsigned missPredictedCounter;

    BTB()=default;

public:
    //constructors:
    BTB(unsigned btbSize1,unsigned historySize1,unsigned tagSize1,unsigned fsmState1
        ,bool isGlobalHist1,bool isGlobalTable1,int isShare1):globalHistory(historySize1)
        ,globalStateMachine((State)fsmState1){
        btbSize=btbSize1;
        historySize=historySize1;
        tagSize=tagSize1;
        fsmState=fsmState1;
        isGlobalHist=isGlobalHist1;
        isGlobalTable=isGlobalTable1;
        isShare=isShare1;
        //branchesVect=std::vector<BranchCmd>(btbSize);


    }
};

//branch main
int main4() {
    std::cout << "Hello, World! 1 " << std::endl;
    StateMachine machine =StateMachine(SNT);
    std::cout << machine.getState() << std::endl;
    std::cout << "Taken:" << std::endl;
    for (int i=0 ;i<=5 ; i++){
        std::cout << machine.updateState(true) << std::endl;
    }
    std::cout << "Not taken:" << std::endl;
    for (int i=0 ;i<=5 ; i++){
        std::cout << machine.updateState(false) << std::endl;
    }

    return 0;
}



//history main
int main3() {
    std::cout << "Hello, World! 3 " << std::endl;
    History history1=History(4);
    std::cout << unsigned(history1.getHistory())<< std::endl;
    std::cout << unsigned(history1.updateHistory(true))<< std::endl;
    std::cout << unsigned(history1.updateHistory(false)) << std::endl;
    std::cout << unsigned(history1.updateHistory(true)) << std::endl;
    std::cout << unsigned(history1.updateHistory(false)) << std::endl;
    std::cout << unsigned(history1.updateHistory(true)) << std::endl;
    std::cout << unsigned(history1.updateHistory(true)) << std::endl;
    std::cout << unsigned(history1.updateHistory(true)) << std::endl;
    std::cout << unsigned(history1.updateHistory(false)) << std::endl;
    std::cout << unsigned(history1.updateHistory(false)) << std::endl;
    std::cout << unsigned(history1.updateHistory(false)) << std::endl;
    std::string binary = std::bitset<8>(history1.getHistory()).to_string();
    std::cout << binary << std::endl;


    return 0;
}
//StateMachinesVector main
int main2() {
    std::cout << "Hello, World! 2 " << std::endl;
    StateMachinesVector vec=StateMachinesVector(5);

    for (int i=0 ;i<=4 ; i++){
        std::cout << vec.getStateAtIndex(i)<<" " ;
    }
    std::cout << std::endl;


    std::cout<<vec.updateStateAtIndex(3,true)<<" ";
    std::cout << std::endl;
    std::cout<<vec.updateStateAtIndex(3,true)<<" ";
    std::cout << std::endl;
    std::cout<<vec.updateStateAtIndex(2,true)<<" ";
    std::cout << std::endl;
    std::cout<<vec.updateStateAtIndex(3,true)<<" ";
    std::cout << std::endl;
    std::cout<<vec.updateStateAtIndex(3,false)<<" ";
    std::cout << std::endl;
    std::cout<<vec.updateStateAtIndex(3,false)<<" ";
    std::cout << std::endl;
    std::cout<<vec.updateStateAtIndex(3,false)<<" ";
    std::cout << std::endl;
    std::cout<<vec.updateStateAtIndex(2,false)<<" ";
    std::cout << std::endl;

    for (int i=0 ;i<=4 ; i++){
        std::cout << vec.getStateAtIndex(i)<<" " ;
    }
    std::cout << std::endl;


    return 0;
}
//StateMachineCheck main
int main1() {
    std::cout << "Hello, World! 1 " << std::endl;
    StateMachine machine =StateMachine(SNT);
    std::cout << machine.getState() << std::endl;
    std::cout << "Taken:" << std::endl;
    for (int i=0 ;i<=5 ; i++){
        std::cout << machine.updateState(true) << std::endl;
    }
    std::cout << "Not taken:" << std::endl;
    for (int i=0 ;i<=5 ; i++){
        std::cout << machine.updateState(false) << std::endl;
    }

    return 0;
}




int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
            bool isGlobalHist, bool isGlobalTable, int Shared){
    return -1;
}

bool BP_predict(uint32_t pc, uint32_t *dst){
    return false;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
    return;
}

void BP_GetStats(SIM_stats *curStats){
    return;
}

