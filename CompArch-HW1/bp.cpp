//***************************************************************
//* File name    : bp.cpp
//* Description  : implementation of the predictor simulator
//***************************************************************

#include <iostream>
#include <vector>
#include <math.h>
#include <bitset>
#include "bp_api.h"
#include <cmath>
/* 046267 Computer Architecture - Winter 20/21 - HW #1                  */
/* This file should hold your implementation of the predictor simulator */

typedef enum {
    SNT,
    WNT,
    WT,
    ST
}State;

typedef enum {
    not_using_share,
    using_share_lsb,
    using_share_mid
}ShareMode;

//***************************************************************
//* Class name   : StateMachine
//* Description  : 
//***************************************************************
class StateMachine{
private:
    State state;
    State defaultState;
    // privateMethods:
    void updateStateTaken(){
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
    }
    void updateStateNotTaken(){
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
    }

public:
    // constructors:
    StateMachine(State state1){
        state=state1;
        defaultState = state1;
    }
    // methods
    // Getters:
    State getState(){
        return state;
    }
    // Setters:
    void updateState(bool taken){
        if(taken)  updateStateTaken();
        else updateStateNotTaken();
    }
    void reset(){
        state = defaultState;
    };
};

//***************************************************************
//* Class name   : StateMachinesVector
//* Description  : 
//***************************************************************
class StateMachinesVector{
private:
    std::vector<StateMachine> stateMachines;
    unsigned size;

public:
    // constructors:
    StateMachinesVector(unsigned size1,State state){
        stateMachines= std::vector<StateMachine>(size1,state);
        size=size1;
    }
    // methods:
    // Getters:
    State getStateAtIndex(unsigned index){
        return stateMachines.at(index).getState();
    }
    unsigned getSize(){
        return size;
    }
    // Setters:
    void updateStateAtIndex(unsigned index, bool taken){
        stateMachines.at(index).updateState(taken);
    }
    void reset(){
        for (auto machine : stateMachines) {
            machine.reset();
        }
    };
};

//***************************************************************
//* Class name   : History
//* Description  : 
//***************************************************************
class History{
private:
    uint8_t history;
    unsigned size;

public:
    // constructor:
    History(unsigned size1=0){
        history=0x0;
        size=size1;
    }
    // methods:
    // Getters:
    uint8_t getHistory(){
        return history;
    }
    unsigned getSize(){
        return size;
    }
    // Setters:
    void updateHistory(bool taken){
        uint32_t pow=std::pow(2,size-1);
        history%=pow;
        history= (history*2) ;
        history += (uint8_t)taken;
    }
    void reset(){
        history = 0x0;
    };
};

//***************************************************************
//* Class name   : BranchLine
//* Description  : 
//***************************************************************
class BranchLine{
private:
    uint32_t tag;
    uint32_t target;
    bool valid;
    History* historyP;
    StateMachinesVector* stateMachinesVectorP;

public:
    // Constructors:
    BranchLine(uint32_t tag1=0x0,uint32_t target1=0x0,bool valid1=false){
        tag=tag1;
        target=target1;
        valid=valid1;
        historyP = NULL;
        stateMachinesVectorP = NULL;
    }
    // init methods:
    void initHistory(History* history1){
        historyP=history1;
    }
    void initStateMachineVect(StateMachinesVector* stateMachinesVector1){
        stateMachinesVectorP=stateMachinesVector1;
    }
    // Getters:
    uint32_t getTag(){
        return tag;
    }
    uint32_t getTarget(){
        return target;
    }
    History& getBranchHistory(){
        return *historyP;
    }
    StateMachinesVector& getStateMachinesVec(){
        return *stateMachinesVectorP;
    }
    bool isValid(){
        return valid;
    }
    // Setters:
    void updateBranchStateMachine(unsigned index, bool taken) {
        stateMachinesVectorP->updateStateAtIndex(index, taken);
    }
    void updateBranchHistory(bool taken){
        historyP->updateHistory(taken);
    }
    void updateBranchTarget(uint32_t target1){
        target=target1;
    }
    void updateBranchTag(uint32_t tag1) {
        tag = tag1;
    }
    void updateValid(bool valid1){
        valid=valid1;
    }
};

//***************************************************************
//* Class name   : BTB
//* Description  : 
//***************************************************************
class BTB{
private:
    unsigned btbSize;
    unsigned historySize;
    unsigned tagSize;
    unsigned fsmState;
    bool isGlobalHist;
    bool isGlobalTable;
    ShareMode shareMode;

    std::vector<BranchLine>* branchesVec;
    std::vector<History>* localHistoryVec;
    std::vector<StateMachinesVector>*  localStateMachinesVec;
    History* globalHistory;
    StateMachinesVector* globalStateMachineVec;

    unsigned branchesCounter;
    unsigned missPredictedCounter;
public:
    // constructors:
    BTB()=default;
    BTB(unsigned btbSize1,unsigned historySize1,unsigned tagSize1,unsigned fsmState1
            ,bool isGlobalHist1,bool isGlobalTable1,int isShare1){
        btbSize=btbSize1;
        historySize=historySize1;
        tagSize=tagSize1;
        fsmState=fsmState1;
        isGlobalHist=isGlobalHist1;
        isGlobalTable=isGlobalTable1;
        shareMode = (ShareMode)isShare1;
        branchesCounter=0;
        missPredictedCounter=0;
        globalHistory = NULL;
        globalStateMachineVec = NULL;
        localHistoryVec = NULL;
        localStateMachinesVec = NULL;

        if(isGlobalHist){
            globalHistory=new History(historySize);
            //globalHistory=History(historySize);
        }else{
            localHistoryVec=new std::vector<History>(btbSize,History(historySize));
            //localHistoryVec=std::vector<History>(btbSize,History(historySize));
        }
        if(isGlobalTable){
            unsigned pow1=std::pow(2,historySize);
            globalStateMachineVec= new StateMachinesVector(pow1,(State)fsmState);
            //globalStateMachineVec=StateMachinesVector(std::pow(2,historySize),(State)fsmState);
        }else{
            unsigned pow=std::pow(2,historySize);
            localStateMachinesVec=new std::vector<StateMachinesVector>(btbSize
                    ,StateMachinesVector(pow,(State)fsmState));
            //localStateMachinesVec=std::vector<StateMachinesVector>(btbSize
             //       ,StateMachinesVector(std::pow(2,historySize),(State)fsmState));
        }
        branchesVec=new std::vector<BranchLine>(btbSize);
        
        for(unsigned i=0 ; i<btbSize ;i++){
            if(isGlobalHist){
                branchesVec->operator[](i).initHistory(globalHistory);
            }else{
                branchesVec->operator[](i).initHistory(&(localHistoryVec->at(i)));
            }
            if(isGlobalTable){
                branchesVec->operator[](i).initStateMachineVect(globalStateMachineVec);
            }else{
                branchesVec->operator[](i).initStateMachineVect(&localStateMachinesVec->at(i));
            }
            //branchesVec->operator[](i).initHistoryAndStateMachineVect(hist,statesTable);
        }
    }
    // Getters:
    unsigned getIndexFromPc(uint32_t pc){
        return ((pc)%(4*btbSize))/4;
    }
    uint32_t getTagFromPc(uint32_t pc){
        return (uint32_t)((pc)/(4*btbSize))%(uint32_t)(pow(2,tagSize));
    }

    uint8_t getHistoryXor(uint32_t pc, History history) {
        uint8_t tmpIndex = history.getHistory();
        uint32_t pow=(uint32_t)std::pow(2,history.getSize());
        switch (shareMode){
            case not_using_share:
                return tmpIndex;
            case using_share_lsb:
                pc >>= 2;
                tmpIndex ^= pc;
                tmpIndex %= (uint32_t)(pow);
                return tmpIndex;
            case using_share_mid:
                pc >>= 16;
                tmpIndex ^= pc;
                tmpIndex %= (uint32_t)(pow);
                return tmpIndex;
            default:
                return tmpIndex;
        }
    }

    void getStats(SIM_stats* currStats) {
        currStats->br_num = branchesCounter;
        currStats->flush_num = missPredictedCounter;
        unsigned targetSize = 30, validSize = 1;
        unsigned stateMachinesVecSize = std::pow(2, historySize + 1);
        unsigned size = btbSize * (tagSize + targetSize + validSize);
        size += ((isGlobalHist) ? historySize : btbSize * historySize);
        size += ((isGlobalTable) ? stateMachinesVecSize : btbSize * stateMachinesVecSize);
        currStats->size = size;
    }

    bool getPrediction(uint32_t pc, uint32_t* dst) {
        unsigned index = getIndexFromPc(pc);
        uint32_t tag = getTagFromPc(pc);
        BranchLine branchLine = branchesVec->operator[](index);

        if (branchLine.getTag() == tag && branchLine.isValid()) {
            uint32_t target = branchLine.getTarget();
            History history = branchLine.getBranchHistory();
            uint8_t stateIndex = getHistoryXor(pc, history);
            State branchState = branchLine.getStateMachinesVec().getStateAtIndex(stateIndex);
            if (branchState == ST || branchState == WT) {
                //if(dst!=NULL){
                //    *dst=target;
                //}
                *dst = target;
                return true;
            }
        }
        //if(dst!=NULL){
        //    *dst=pc+4;
        //}
        *dst = pc + 4;
        return false;
    }

    // Setters:
    void updateBtbOnExe(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst) {
        unsigned BranchIndex = getIndexFromPc(pc);
        uint32_t newTag = getTagFromPc(pc);
        BranchLine& branchLine = branchesVec->operator[](BranchIndex);
        //History oldHistory = branchLine.getBranchHistory();
        uint32_t oldTag = branchLine.getTag();
        uint32_t oldTarget = branchLine.getTarget();
        uint8_t StateIndex;

        branchesCounter++;
        if (branchLine.isValid()) { // Taken line in BTB
            if (newTag != oldTag) { // Different branch
                if (!isGlobalHist) branchLine.getBranchHistory().reset();
                if (!isGlobalTable) branchLine.getStateMachinesVec().reset();
                branchLine.updateBranchTag(newTag);
                //branchLine.updateBranchTarget(targetPc);
            }
        }
        else {  // Empty line in BTB
            branchLine.updateBranchTag(newTag);
            //branchLine.updateBranchTarget(targetPc);
            branchLine.updateValid(true);
            //StateIndex = getHistoryXor(pc, branchLine.getBranchHistory());
            //branchLine.updateBranchHistory(taken);
            //branchLine.updateBranchStateMachine(StateIndex, taken);
            //return;
        }

        //oldTag = branchLine.getTag();
        //oldTarget = branchLine.getTarget();
        //
        // Same branch made the prediction
        //if (oldTarget == pred_dst) {
        //    if ((oldTarget != targetPc) && taken) missPredictedCounter++;
        //    if (!taken) missPredictedCounter++;
        //}

        //// Different branch made the prediction
        //if ((oldTarget != pred_dst) && taken) {
        //    missPredictedCounter++;
        //}

        if (taken && (targetPc != pred_dst)) missPredictedCounter++;
        if (!taken && (pred_dst != pc + 4)) missPredictedCounter++;
        
        StateIndex = getHistoryXor(pc, branchLine.getBranchHistory());
        branchLine.updateBranchHistory(taken);
        branchLine.updateBranchStateMachine(StateIndex, taken);
        branchLine.updateBranchTarget(targetPc);
    }
};

/*
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
*/


BTB *main_BP;


int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
            bool isGlobalHist, bool isGlobalTable, int Shared){
    main_BP = new BTB(btbSize, historySize, tagSize, fsmState, isGlobalHist, isGlobalTable, Shared);
    return 0;
}

bool BP_predict(uint32_t pc, uint32_t *dst){

    return main_BP->getPrediction(pc, dst);
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
    main_BP->updateBtbOnExe(pc, targetPc, taken, pred_dst);
    return;
}

void BP_GetStats(SIM_stats *curStats){
    main_BP->getStats(curStats);
    delete main_BP;
    return;
}

