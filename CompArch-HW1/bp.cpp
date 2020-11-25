//
// Created by khour on 24/11/2020.
//
#include <iostream>
#include <vector>
#include <math.h>
#include <bitset>
#include "bp.h"
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
public:
    //constructor:
    History(unsigned size1=0){
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
        /*f(taken){
            history++;
        }*/
        return history+(uint8_t)taken;
    }
};

class BranchLine{
private:
    uint32_t tag;
    uint32_t target;
    bool valid;
    History* historyP;
    StateMachinesVector* stateMachinesVectorP;

public:
    //Constructors:

    BranchLine(uint32_t tag1=0x0,uint32_t target1=0x0,bool valid1=false){
        tag=tag1;
        target=target1;
        valid=valid1;
    }
    void initHistoryAndStateMachineVect(History* history1,StateMachinesVector* stateMachinesVector1){
        historyP=history1;
        stateMachinesVectorP=stateMachinesVector1;
    }
    uint32_t getTag(){
        return tag;
    }
    uint32_t getTarget(){
        return target;
    }
    History getBranchHistory(){
        return *historyP;
    }
    StateMachinesVector getStateMachinesVec(){
        return *stateMachinesVectorP;
    }
    bool isValid(){
        return valid;
    }

    History updateBranchHistory(bool taken){
        return historyP->updateHistory(taken);
    }
    uint32_t updateBranchTarget(uint32_t target1){
        target=target1;
        return target;
    }
    bool updateValid(bool valid1){
        valid=valid1;
        return valid;
    }
    BranchLine updateBranch(bool taken, uint32_t target1){
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

    std::vector<BranchLine> branchesVec;
    std::vector<History> localHistoryVec;
    std::vector<StateMachinesVector>  localStateMachinesVec;
    History globalHistory;
    StateMachinesVector globalStateMachineVec;

    unsigned branchesCounter;
    unsigned missPredictedCounter;
public:
    //constructors:
    BTB()=default;
    BTB(unsigned btbSize1,unsigned historySize1,unsigned tagSize1,unsigned fsmState1
        ,bool isGlobalHist1,bool isGlobalTable1,int isShare1){
        btbSize=btbSize1;
        historySize=historySize1;
        tagSize=tagSize1;
        fsmState=fsmState1;
        isGlobalHist=isGlobalHist1;
        isGlobalTable=isGlobalTable1;
        isShare=isShare1;
        branchesCounter=0;
        missPredictedCounter=0;

        if(isGlobalHist){
            globalHistory=History(historySize);
        }else{
            localHistoryVec=std::vector<History>(btbSize,History(historySize));
        }
        if(isGlobalTable){
            globalStateMachineVec=StateMachinesVector(std::pow(2,historySize),(State)fsmState);
        }else{
            localStateMachinesVec=std::vector<StateMachinesVector>(btbSize
                    ,StateMachinesVector(std::pow(2,historySize),(State)fsmState));
        }
        branchesVec=std::vector<BranchLine>(btbSize);

        for(int i=0 ; i<btbSize ;i++){
            History* hist;
            if(isGlobalTable){
                hist=&globalHistory;
            }else{
                hist=&localHistoryVec[i];
            }
            StateMachinesVector* statesTable;
            if(isGlobalTable){
                statesTable=&globalStateMachineVec;
            }else{
                statesTable=&localStateMachinesVec[i];
            }
            branchesVec[i].initHistoryAndStateMachineVect(hist,statesTable);
        }
    }

    unsigned getIndexFromPc(uint32_t pc){
        return ((pc)%(4*btbSize))/4;
    }
    uint32_t getTagFromPc(uint32_t pc){
        return (uint32_t)((pc)/(4*btbSize))%(uint32_t)(pow(2,tagSize));
    }
    bool predict(uint32_t pc, uint32_t *dst){
        unsigned index= getIndexFromPc(pc);
        uint32_t tag=getTagFromPc(pc);

        BranchLine branchLine=branchesVec[index];
        if(branchLine.getTag()==tag && branchLine.isValid()){
            uint32_t target=branchLine.getTarget();
            uint8_t history= branchLine.getBranchHistory().getHistory();
            State branchState=branchLine.getStateMachinesVec().getStateAtIndex(history);
            if (branchState==ST || branchState==WT){
                *dst=target;
                return true;
            }
        }
        *dst=pc+4;
        return false;
    }
};

//branch main
int main() {
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
    BTB btb=BTB(btbSize, historySize, tagSize, fsmState, isGlobalHist, isGlobalTable, Shared);
    return 0;
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

