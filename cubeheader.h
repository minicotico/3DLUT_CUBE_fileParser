#ifndef CUBEHEADER_H
#define CUBEHEADER_H


//A line of text shall not be longer than 250 bytes
#define SIZE_LINE_MAX   250
#define SIZE_FILENAME   200
#define SIZE_WORD       50

#define str_EndOfFile               "End Of File"
#define str_CouldNotParseTableData  "Could Not Parse Table Data"
#define str_CouldNotParseKeyWord    "Could Not Parse KeyWord"
#define str_OK                      "OK"
#define str_LUTSizeOutOfRange       "LUT Size Out Of Range"

typedef enum {
    OK = 0,
    NotInitialized = 1,
    ReadError = 10,
    WriteError,
    //PrematuredEndOfFile,
    EndOfFile,
    LineError,
    UnknownOrRepeatedKeyword = 20,
    //TitleMissingQuote,
    DomainBoundsReversed,
    LUTSizeOutOfRange,
    CouldNotParseKeyWord,
    CouldNotParseTableData,
}
LUTState;


#define str_parseKeyWord        "Parsing Keywords"
#define str_parseData           "Parsing Datas"

typedef enum{
    parseKeyWord = 1,
    parseData,
    error,
}
ParsingState;


#define TYPE_3D 3
#define TYPE_1D 1

typedef struct
{
    int type;//1D/3D
    int size;
    float domainMin[3];
    float domainMax[3];
    char title[SIZE_FILENAME];

}
s_lutStat;


typedef struct
{
    ParsingState status;
    /* Store current line */
    char line[SIZE_LINE_MAX + 1 ];
    int lineLen;
    /* Store current (first) word */
    char word[SIZE_WORD];
    int wordLen;
    /* Store Data (of a line)*/
    float r;
    float g;
    float b;
}
s_line;



typedef struct
{
    char filename[SIZE_FILENAME];
    FILE* fp;
    LUTState status;
}
s_cubeFile;


//PROTO

int CUBE_initStruct(s_cubeFile* p_cubeFile, s_line* p_line, s_lutStat* p_lutStat);
int CUBE_openFile(s_cubeFile* p_cubeFile);
int CUBE_closeFile(s_cubeFile* p_cubeFile);
int CUBE_readLine(s_cubeFile* p_cubeFile, s_line* p_line);
int CUBE_parseKeyWords(s_line* p_line, s_lutStat* p_lutStat);
LUTState CUBE_treatKeyWord(s_line* p_line, s_lutStat* p_lutStat);
LUTState CUBE_parseData(s_line* p_line);

/****** INFO / STATUS / TEXT ******/
int CUBE_showInfo(s_lutStat* p_lutStat);
int CUBE_showStatus(s_cubeFile* p_cubeFile, s_line* p_line, s_lutStat* p_lutStat);
char* CUBE_textStatusCube(s_cubeFile* p_cubeFile);
char* CUBE_textStatusLine(s_line* p_line);
//OPTIONS
//#define CUBE_LIBRARY


#endif // CUBEHEADER_H

