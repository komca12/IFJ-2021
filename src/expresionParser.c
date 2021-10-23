/**
 * @brief   Expresion parsing
 *
 * @authors Jakub Komárek (xkomar33)
 */
#include "expresionParser.h"

const char precTable[10][10] =
{
//	  #    *,/,//,   +-      ..       b       (       )        i      $
    {'<'	,'>'	,'>'	,'>'	,'>'	,'<'	,'>'	,'<'	,'>'},// #
    {'<'	,'>'	,'>'	,'>'	,'>'	,'<'	,'>'	,'<'	,'>'},// *,/
	{'<'	,'<'	,'>'	,'>'	,'>'	,'<'	,'>'	,'<'	,'>'},// +,-
	{'<'	,'<'	,'<'	,'<'	,'>'	,'<'	,'>'	,'<'	,'>'},// ..
	{'<'	,'<'	,'<'	,'<'	,'>'	,'<'	,'>'	,'<'	,'>'},// >, <, <=, >=, ==, !=
	{'<'	,'<'	,'<'	,'<'	,'<'	,'<'	,'='	,'<'	,' '},// (	
	{'>'	,'>'	,'>'	,'>'	,'>'	,' '	,'>'    ,' '	,'>'},// )
	{' '	,'>'	,'>'	,'>'	,'>'	,' '	,'>'	,' '	,'>'},// i
	{'<'	,'<'	,'<'	,'<'	,'<'	,'<'	,' '	,'<'	,' '},// $
};

tokenType expresionParse(systemData *sData,bool ignor)
{
    debugS("\x1B[33m******************* analysys swich to expresion mode*************************\x1B[0m\n"); 
    stack * stack=&sData->epData.stack;
    bool separatorF=false;
    token actual =nextTokenExpParser(&separatorF,sData,true);
    stackClear(stack);
    stackPush(stack,(token){O_DOLAR});
    while (stackTop(stack).type!=O_DOLAR||actual.type!=T_EOF)
    {
        debug("input:%-10s stack:%-10s\n",tokenStr(actual),tokenStr(stackTop(stack))); 
        stackPrint(stack)  ; 
        switch (getSymFromPrecTable(actual.type,stackTop(stack).type))
        {
            case '=':
                stackPush(stack,actual);
                actual=nextTokenExpParser(&separatorF,sData,false);
                break;
            case '<':
                stackInsertHanle(stack);
                stackPush(stack,actual);
                actual=nextTokenExpParser(&separatorF,sData,false);
                break; 
            case '>':
                reduction(stack,ignor);
                break;
            case ' ':
                if(stackTop(stack).type==O_DOLAR&&actual.type==T_RBR)//right acket can end the fuction call-no lexical error
                    return stackHead(stack).typeOfValue;
                else
                    errorD(2,"syntax error in expresion");
                break;
            default:
                errorD(99,"precedence table error");
                break;
        }
    }
    if(stackHead(stack).type!=NE_EXP)
        errorD(2,"výraz nesmí být prázdný");
    return stackHead(stack).typeOfValue;
    debugS("\x1B[33m******************* analysys swich to normal mode*************************\x1B[0m\n"); 
} 

token nextTokenExpParser(bool * separatorF,systemData * sData,bool firstT)
{
    if(!firstT)
        sData->pData.actualToken=getNextUsefullToken(&sData->sData);

    tokenType new= sData->pData.actualToken.type;

    if(*separatorF==true && (new==T_ID))
        return (token){T_EOF};
    else if(isConstant(new))
        *separatorF=true;
    else if(new!=T_LBR&& new!=T_RBR)
        *separatorF=false;


    if(new==T_ID)
    {
        STSymbolPtr * ptr= frameStackSearchVar(&sData->pData.dataModel,sData->sData.fullToken.str);
        STData *varData=ptr?&(*ptr)->data:NULL;
        if(!varData)
            errorD(3,"Proměnná ve výrazu není definována");
        else if(varData->type!=ST_VAR)
            errorD(3,"Proměnná ve výrazu je typu funkce");
        else if(!varData->varData->defined)
            errorD(3,"Proměnná ve výrazu je typu funkce");
        printf("PUSHS ");genVar(varData->dekorator,(*ptr)->id);
        return (token){new,varData->varData->type};
    }
    else if(isOperator(new)||isConstant(new)||new==T_RBR||new==T_LBR)
    {
        switch (new)
        {
        case T_DOUBLE:
            printf("PUSHS float@%a\n",strtod(sData->sData.fullToken.str,NULL));
            return (token){new,K_NUMBER};
        break;
        case T_INT:
            printf("PUSHS int@%s\n",sData->sData.fullToken.str);
            return (token){new,K_INTEGER};
        break;
        case T_STR:
            printf("PUSHS string@");genStringConstant(sData->sData.fullToken.str);printf("\n");
            return (token){new,K_STRING};     
        break;
        case K_NIL:
            printf("PUSHS nil@nil\n");
            return (token){new,K_NIL};
        break;
        default:
            return (token){new,new};
        break;
        }
    }
    else
        return (token){T_EOF};
}

void reduction(stack *s,bool ignor)
{
    token id1=(token){O_NONE};
    token op=(token){O_NONE};
    token id2=(token){O_NONE};

    token aux=stackPop(s);
    switch (aux.type)
    {
        case T_ID:
        case T_INT:
        case T_DOUBLE:
        case T_STR:
        case K_NIL:
            stackRemoveHande(s);
            stackPush(s,(token){NE_EXP,aux.typeOfValue});
            return;
        break;
        case NE_EXP:
            id2=aux;
        break;
        case T_RBR:
            aux=stackPop(s);
            if(aux.type!=NE_EXP)
                errorD(2,"expresion in bracked err");
            if(stackPop(s).type!=T_LBR)
                errorD(2,"expresion in bracked err");
            stackRemoveHande(s);
            stackPush(s,(token){NE_EXP,aux.typeOfValue});    
            return;
        break;
        default:
            errorD(2,"sa reduction err");
    }

    aux=stackPop(s);
    switch (aux.type)
    {
        case T_STR_LEN:
            stackRemoveHande(s);
            op=aux;
            stackPush(s,(token){NE_EXP,generateExpresion(id1,op,id2)});
            return;
        break;
        default:
            if(isOperator(aux.type))
                op=aux;
            else
                errorD(2,"sa reduction err");
    }

    if(stackHead(s).type==O_HANDLE)
    {
        if(op.type==T_SUB)
        {
            genInst("CALL neg");
            stackPush(s,(token){NE_EXP,id2.typeOfValue});
        }
        else if(op.type==T_ADD)
        {
            stackPush(s,(token){NE_EXP,id2.typeOfValue});
        }
        else
            errorD(2,"sa reduction err");
        return;
    }
    
    aux=stackPop(s);
    switch (aux.type)
    {
        case NE_EXP:
            id1=aux;
            stackRemoveHande(s);
            stackPush(s,(token){NE_EXP,generateExpresion(id1,op,id2)});
            return;
        break;
        default:
            errorD(2,"sa reduction err");
    }
}

tokenType aritmeticComCheck(token id1,token id2,bool forcedNumber)
{
    if(id1.typeOfValue==K_NIL||id2.typeOfValue==K_NIL)
        errorD(8,"nepovolená operace s nil");
    if(id1.typeOfValue!=K_INTEGER&&id1.typeOfValue!=K_NUMBER)
        error(6);
    if(id2.typeOfValue!=K_INTEGER&&id2.typeOfValue!=K_NUMBER)
        error(6);
    if(id1.typeOfValue==K_NUMBER||id2.typeOfValue==K_NUMBER||forcedNumber)
    {
        if(id1.typeOfValue!=K_NUMBER)
        {
            printf("POPS gf@&regA\nINT2FLOATS\nPUSHS gf@&regA\n");
            id1.typeOfValue=K_NUMBER;
        }
        if(id2.typeOfValue!=K_NUMBER)
        {
            genInst("INT2FLOATS");
            id2.typeOfValue=K_NUMBER;
        }
        return K_NUMBER;
    }
    return K_INTEGER;
}

tokenType comperzionComCheck(token id1,token id2,bool nillEnable)
{
    if(id1.typeOfValue==K_NIL||id2.typeOfValue==K_NIL)
    {
        if(!nillEnable)
            errorD(8,"nepovolená operace s nil");
    }
    else if(id1.typeOfValue!=id2.typeOfValue)
    {
        if(id1.typeOfValue==K_NUMBER||id1.typeOfValue==K_NUMBER)
        {
            if(id1.typeOfValue!=K_NUMBER)
            {
                printf("POPS gf@&regA\nINT2FLOATS\nPUSHS gf@&regA\n");
                id1.typeOfValue=K_NUMBER;
            }
            if(id2.typeOfValue!=K_NUMBER)
            {
                genInst("INT2FLOATS");
                id2.typeOfValue=K_NUMBER;
            }
        }
        else
            error(6);
    }
    return K_BOOL;
}


tokenType generateExpresion(token id1, token op ,token id2)
{
    debug("generated: %s %s %s\n",tokenStr(id2),tokenStr(op),tokenStr(id1));
    tokenType type;
    switch (op.type)
    {
        case T_MUL:
            type=aritmeticComCheck(id1,id2,false);
            genInst("MULS");
        break;
        case T_DIV:
            type=aritmeticComCheck(id1,id2,true);
            genInst("CALL safediv");
        break;
        case T_DIV2:
            if(id1.typeOfValue==K_NIL||id2.typeOfValue==K_NIL)
                errorD(8,"nepovolená operace s nil");
            if(id1.typeOfValue!=K_INTEGER||id2.typeOfValue!=K_INTEGER)
                errorD(6,"celočíselné dělení lze provádět pouze s operandy typu integer");
            type=K_INTEGER;
            genInst("CALL safediv");
        break;
        case T_ADD:
            type=aritmeticComCheck(id1,id2,false);
            genInst("ADDS");
        break;
        case T_SUB:
            type=aritmeticComCheck(id1,id2,false);
            genInst("SUBS");
        break;
        case T_EQ:
            comperzionComCheck(id1,id2,true);
            type=K_BOOL;
            genInst("EQS");
        break;
        case T_NOT_EQ:
            comperzionComCheck(id1,id2,true);
            genInst("EQS\nNOTS");
            type=K_BOOL;   
        break;
        case T_GT:
            comperzionComCheck(id1,id2,false);
            type=K_BOOL;
            genInst("GTS");
        break;
        case T_GTE:
            comperzionComCheck(id1,id2,false);
            genInst("POPS gf@&regB");
            genInst("POPS gf@&regA");
            genInst("GT gf@&regC gf@&regA gf@&regB");
            genInst("PUSHS  gf@&regC");
            genInst("EQ gf@&regC gf@&regA gf@&regB");
            genInst("PUSHS  gf@&regC");
            genInst("ORS");
            type=K_BOOL;    
        break;
        case T_LT:
            comperzionComCheck(id1,id2,false);
            type=K_BOOL;
            genInst("LTS");
        break;
        case T_LTE:
            comperzionComCheck(id1,id2,false);  
            genInst("POPS gf@&regB");
            genInst("POPS gf@&regA");
            genInst("LT gf@&regC gf@&regA gf@&regB");
            genInst("PUSHS  gf@&regC");
            genInst("EQ gf@&regC gf@&regA gf@&regB");
            genInst("PUSHS  gf@&regC");
            genInst("ORS");
            type=K_BOOL;
        break;
        case T_STR_LEN:
            if(id2.typeOfValue==K_NIL)
                errorD(8,"nepovolená operace s nil");
            if(id2.typeOfValue!=K_STRING)
                errorD(6,"operace délka řetězce lze provádět pouze na stringu");
            genInst("CALL hashtag");
            type=K_INTEGER;

        break;
        case T_DOT2:
            if(id1.typeOfValue==K_NIL||id2.typeOfValue==K_NIL)
                errorD(8,"nepovolená operace s nil");
            if(id1.typeOfValue!=K_STRING||id2.typeOfValue!=K_STRING)
                errorD(6,"operace konkatenance řetězců lze provádět pouze na stringu");
            type=K_STRING;

        break;
        default:
            errorD(2,"porušeno rozkladové pravidlo");
        break;
    }
    return type;
}

char getSymFromPrecTable(tokenType input, tokenType stack)
{
    int colum=getPosInTable(input);
    int row=getPosInTable(stack);
    return precTable[row][colum];
}

int getPosInTable(tokenType toDecode)
{
    switch (toDecode)
    {
        case T_STR_LEN:
            return 0;
        case T_MUL:  
        case T_DIV:
        case T_DIV2:
            return 1;
        case T_ADD:
        case T_SUB:
            return 2;  
        case T_DOT2:
            return 3;   
        case T_GT:
        case T_LT:
        case T_LTE:
        case T_GTE:  
        case T_EQ:
        case T_NOT_EQ:
            return 4;
        case T_LBR:
            return 5;           
        case T_RBR:
            return 6;
        case T_ID:
        case T_STR:
        case T_INT:
        case K_NIL:
        case T_DOUBLE:
            return 7;  
        case O_DOLAR:
        case T_EOF:
        default:
            return 8;
    }
}

void initExpresionData(expresionParserData *data)
{   
    stackInit(&data->stack);
}

void destructExpresionData(expresionParserData *data)
{   
    stackDestruct(&data->stack);
}

bool isConstant(tokenType toCompere)   
{
    return toCompere==T_STR||toCompere==T_INT||toCompere==K_NIL||toCompere==T_DOUBLE||toCompere==T_ID;
}

bool isOperator(tokenType toCompere)    
{
    return toCompere==T_DIV2||toCompere==T_DIV||toCompere==T_MUL||toCompere==T_ADD||toCompere==T_SUB||toCompere==T_STR_LEN||toCompere==T_EQ||toCompere==T_NOT_EQ||toCompere==T_LT||toCompere==T_LTE||toCompere==T_GT||toCompere==T_GTE||toCompere==T_DOT2||toCompere==T_STR_LEN;
}

