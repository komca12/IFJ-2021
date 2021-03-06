/**
 * Implementace překladače imperativního jazyka IFJ21
 * @brief   scaner
 *
 * @authors Jakub Komárek (xkomar33)
 */
#pragma once
#include "baseDeclarations.h"
#include "dstring.h"
#include "symtable.h"
#include "tokens.h"

typedef struct {
    char curentSymbol;
    int line;
    int colum;
    bool write;
    string fullToken;   //whole text form of actual token
    string fullLine;    //actual line-for debuging purperses
}scanerData;

/**states of state machine**/
typedef enum{       
    S_START,
    S_ID,
    S_IDS,
    S_SUB,
    S_LINE_COM,
    S_LINE_COM_2,
    S_LINE_COM_PER,
    S_BLOCK_COM,
    S_BLOCK_COM2,
    S_BLOCK_COM3,
    S_ADD,
    S_DIV,
    S_DIV2,
    S_MUL,
    S_STR_LEN,
    S_ASSIGEN,
    S_EQ,
    S_NOT_EQ1,
    S_NOT_EQ2,
    S_LT,
    S_LTE,
    S_GT,
    S_GTE,
    S_RBR,
    S_LBR,
    S_COLON,
    S_COMMA,
    S_EOF,
    S_SPACE,
    S_STR1,
    S_STR2,
    S_ESC1,
    S_ESC2,
    S_ESC3,
    S_ESC4,
    S_ESC5,
    S_INT,
    S_DOUBLE1,
    S_DOUBLE2,
    S_EXP1,
    S_EXP2,
    S_EXP3,
    S_DOT1,
    S_DOT2,
    S_FUNC_CALL,
    S_ERROR,
    S_UNDEFINATED,
}state;

/**
 * @brief checking if ID is a keyword
 * @param *data scanner data
 * @return if ID-T_ID else keyword token
*/
tokenType checkKeywords(scanerData *data,tokenType type);

/**
 * @brief geting next usefull token-ignoring commets, ...
 * @param *data scanner data
 * @return next usefull token
 */
token getNextUsefullToken(scanerData * data);

/**
 * @brief geting next token
 * @param *data scanner data
 * @return next token
 */
tokenType getNextToken(scanerData * data);

/**
 * @brief geting token from state of state machine
 * @param state actual state of state machine
 * @return token from state
 */
tokenType getTokenFromState(state state);

/**
 * @brief geting next state of state machine from actual charakter
 * @param *data scanner data
 * @param curentState actual state of state machine 
 * @return next state
*/
state nextState(scanerData*data, state curentState);

/**
 * @brief checking if charakter is letter
 * @param toCompare char to compering
 * @return true-is letter, false-isnt letter
*/
bool isLetter(char toCompare);

/**
 * @brief checking if charakter is decimal 
 * @param toCompare char to compering
 * @return true-is decimal, false-isnt decimal
*/
bool isDecimal(char toCompare);

/**
 * @brief inicialization function for scanner
 * @param *data data to incializate
*/ 
void initScanerData(scanerData * data);

/**
 * @brief destriction function for scanner
 * @param *data data to destruct
*/ 
void destructScanerData(scanerData * data);

/**
 * @brief loading next char from input, menwhile harvesting actual pozition in file for error masseges
 * @param *data scanner data
*/
void loadChar(scanerData * data);

/**
 * @brief visualization, where exactly is error
 * @param *data scanner data
*/
void errorVisualization(scanerData * data);
