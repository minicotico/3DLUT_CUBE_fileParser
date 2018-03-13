#include <stdio.h>
#include <stdlib.h>
#include "cubeheader.h"
#include <string.h>




#ifdef DEBUG
#define print_deb   printf
#else
#define print_deb(...)   (void)0
#endif

#ifndef CUBE_LIBRARY

#ifdef GET_SIZE_PROGRAM
int main(int argc, char* const argv[])
{
    printf("Hello Program : %s\n", argv[0]);
    
    s_cubeFile cube;
    s_line line_word;
    s_lutStat lutStat;
    int ret=0;

    ret = CUBE_initStruct(&cube, &line_word, &lutStat);
    if(ret != 0)
        goto err;

    /* Copy FileName from Arg[1]*/
    memcpy(cube.filename, argv[1], SIZE_FILENAME);


    ret = CUBE_openFile(&cube);

    if(ret != 0)
        goto err;


    while(cube.status == OK){

        ret = CUBE_readLine(&cube, &line_word);
        if(ret < 0){
            if(cube.status == EndOfFile)
                goto err;
            else
                goto err;
        }
        
        if(line_word.status == parseKeyWord){
            /* Parse Keyword to fill the stat struct */
            cube.status = CUBE_parseKeyWords(&line_word, &lutStat);
        }
        if(line_word.status == parseData){
            return lutStat.size;
        }
    }

err:
    CUBE_showStatus(&cube, &line_word, &lutStat);
    CUBE_closeFile(&cube);
    return 0;

}

#else

/**
 * @brief main
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char* const argv[])
{
    printf("Hello Program : %s\n", argv[0]);
    
    s_cubeFile cube;
    s_line line_word;
    s_lutStat lutStat;
    int ret=0;

    ret = CUBE_initStruct(&cube, &line_word, &lutStat);
    if(ret != 0)
        goto errorInit;

    /* Copy FileName from Arg[1]*/
    memcpy(cube.filename, argv[1], SIZE_FILENAME);


    ret = CUBE_openFile(&cube);

    if(ret != 0)
        goto errorOpen;


    while(cube.status == OK){

        ret = CUBE_readLine(&cube, &line_word);
        if(ret < 0){
            if(cube.status == EndOfFile)
                goto eof;
            else
                goto errorRead;
        }
        
        if(line_word.status == parseKeyWord){
            /* Parse Keyword to fill the stat struct */
            cube.status = CUBE_parseKeyWords(&line_word, &lutStat);
        }

        if(line_word.status == parseData){
            /* Parse Data */
            cube.status = CUBE_parseData(&line_word);

            /* Use Datas R/G/B from s_line struct, Send them ... */
        }
    }

eof:
    CUBE_showStatus(&cube, &line_word, &lutStat);
    CUBE_closeFile(&cube);
    return 0;


errorRead:
    printf("Error Read \n");
    CUBE_showStatus(&cube, &line_word, &lutStat);
    return 0;

errorOpen:
    printf("Error Open \n");
    CUBE_closeFile(&cube);
    return 0;

errorInit:
    printf("Error Init \n");
    CUBE_showStatus(&cube, &line_word, &lutStat);
    return 0;
}


#endif//GET_SIZE_PROGRAM
#endif//LIBRARY


/**
 * @brief CUBE_initStruct
 * @param p_cubeFile
 * @param p_line
 * @param p_lutStat
 * @return
 */
int CUBE_initStruct(s_cubeFile* p_cubeFile,
                    s_line* p_line,
                    s_lutStat* p_lutStat)
{
    int loop;

    memset(p_cubeFile->filename, 0, SIZE_FILENAME);
    p_cubeFile->fp = 0;
    p_cubeFile->status = OK;

    p_line->status = parseKeyWord;
    p_line->lineLen = 0;
    p_line->wordLen = 0;
    memset(p_line->line, 0, SIZE_LINE_MAX + 1);
    memset(p_line->word, 0, SIZE_WORD);

    p_lutStat->size = 0;
    p_lutStat->type = 0;
    memset(p_lutStat->title, 0, SIZE_FILENAME);
    for(loop=0; loop < 3 ; loop++){
        p_lutStat->domainMin[loop]=0;
        p_lutStat->domainMax[loop]=1;
    }

    return 0;
}

//Open A File

/**
 * @brief CUBE_openFile
 * @param p_cubeFile
 * @return 0 if file is opened; -1 otherwise
 */
int CUBE_openFile(s_cubeFile* p_cubeFile)
{
    const char* modes="r";


    print_deb("Open File: %s  \n", p_cubeFile->filename);

    p_cubeFile->fp = fopen(p_cubeFile->filename, modes);
    p_cubeFile->status = OK;
    /* Sanitary check */
    if(p_cubeFile->fp > 0 )
        return 0;
    else
        return -1;

}


//Close A File
/**
 * @brief CUBE_closeFile
 * @param p_cubeFile
 * @return 0 on success, EOF otherwise
 */
int CUBE_closeFile(s_cubeFile* p_cubeFile)
{
    print_deb("Close File: %s \n", p_cubeFile->filename);

    return fclose(p_cubeFile->fp);
}





//Read a line
/**
 * @brief CUBE_readLine
 * @param p_cubeFile
 * @param p_line
 * @return 0 on success, -1 on error, check status
 *
 * Check comment lines
 */
int CUBE_readLine(s_cubeFile* p_cubeFile, s_line* p_line)
{
    const char comment = '#';
    const char invalid = '\r';
    const char jump = '\n';
    int ret;
    char* ret_fgets;

    do{
        p_cubeFile->status = OK;

        /* Reset the line */
        memset(p_line->line, 0, SIZE_LINE_MAX + 1);
        p_line->lineLen = 0;


        /* Read until '\n' or EOF characters */
        ret_fgets = fgets( p_line->line, SIZE_LINE_MAX, p_cubeFile->fp );
        if((ret_fgets == NULL) || feof(p_cubeFile->fp))
        {
            p_cubeFile->status = EndOfFile;
            return -1;
        }
        if(ferror(p_cubeFile->fp)){
            p_cubeFile->status = ReadError;
            return -1;
        }

        /* Get Line Size */
        p_line->lineLen = strlen(p_line->line);

        //print_deb("line found : %s",p_line->line);
        //print_deb("line lenght : %i\n",p_line->lineLen);

        /* Control if not a comment */
        if(p_line->lineLen > 0 )
            if(p_line->line[0] == comment){
                print_deb("comment found : %s",p_line->line);
                p_line->lineLen=0;
            }
            if(p_line->line[0] == invalid){
                print_deb("invalid found : %s",p_line->line);
                p_line->lineLen=0;
            }
            if(p_line->line[0] == jump){
                print_deb("jump found\n");
                p_line->lineLen=0;
            }

    }
    while(p_line->lineLen == 0);

    return 0;
}


/**
 * @brief CUBE_parseKeyWords
 * @param p_line
 * @param p_lutStat
 * @return
 *
 * Get the first word from a line
 */
int CUBE_parseKeyWords(s_line* p_line, s_lutStat* p_lutStat)
{
    int ret = 0;
    char *treatedLine = malloc(SIZE_LINE_MAX + 1 );
    const char delimiter[2]=" ";
    char *pch;
    
    memset(&p_line->word,0, sizeof(p_line->word));

    /* Copy line, stryok will modify it */
    memcpy(treatedLine, p_line->line, p_line->lineLen);

    pch = strtok(treatedLine, &delimiter);

    memcpy(&p_line->word, pch, strlen(pch));
    p_line->wordLen = strlen(p_line->word);

    print_deb("word found : %s \n", p_line->word);

    /* Treat the current keyword */
    ret = CUBE_treatKeyWord(p_line, p_lutStat);

    free(treatedLine);

    return ret;
}


/**
 * @brief CUBE_treatKeyWord
 * @param p_line
 * @param p_lutStat
 * @return
 *
 * Compare a word and splits other word from the line depending on the first word
 */
LUTState CUBE_treatKeyWord(s_line* p_line, s_lutStat* p_lutStat)
{
    int ret;

    if( strcmp(p_line->word, "TITLE") == 0){

        ret = sscanf(p_line->line, "%*s %s", p_lutStat->title);

        if(ret != 1)
            return CouldNotParseKeyWord;
    }
    else if( strcmp(p_line->word, "DOMAIN_MIN") == 0){

        ret = sscanf(p_line->line, "%*s %f %f %f",  &p_lutStat->domainMin[0],
                                                    &p_lutStat->domainMin[1],
                                                    &p_lutStat->domainMin[2]);

        if(ret != 3)
            return CouldNotParseKeyWord;


    }
    else if( strcmp(p_line->word, "DOMAIN_MAX") == 0){

        ret = sscanf(p_line->line, "%*s %f %f %f",  &p_lutStat->domainMax[0],
                                                    &p_lutStat->domainMax[1],
                                                    &p_lutStat->domainMax[2]);

        if(ret != 3)
            return CouldNotParseKeyWord;


    }
    else if( strcmp(p_line->word, "LUT_1D_SIZE") == 0){

        if(p_lutStat->type != 0)
            return UnknownOrRepeatedKeyword;


        sscanf(p_line->line, "%*s %i", &p_lutStat->size);
        p_lutStat->type = TYPE_1D;

        if((p_lutStat->size < 2) || (p_lutStat->size > 65536))
            return LUTSizeOutOfRange;
    }
    else if( strcmp(p_line->word, "LUT_3D_SIZE") == 0){


        //printf ("p_lutStat->type = %i \n", p_lutStat->type);
        if(p_lutStat->type != 0)
            return UnknownOrRepeatedKeyword;

        sscanf(p_line->line, "%*s %i", &p_lutStat->size);
        p_lutStat->type = TYPE_3D;

        if((p_lutStat->size < 2) || (p_lutStat->size > 256))
            return LUTSizeOutOfRange;
    }
    else{
        print_deb("Unknown KeyWord \n");

        if((p_line->word[0] >= '+') && (p_line->word[0] <= '9'))
        {
            if( (p_lutStat->domainMin[0] > p_lutStat->domainMax[0]) ||
                (p_lutStat->domainMin[1] > p_lutStat->domainMax[1]) ||
                (p_lutStat->domainMin[2] > p_lutStat->domainMax[2]))
                return DomainBoundsReversed;

            if(p_lutStat->type == 0)
                return LUTSizeOutOfRange;
            printf("Start Parsing Data \n");
            p_line->status = parseData;
        }
        
        CUBE_showInfo(p_lutStat);

    }

    return OK;
}


/**
 * @brief CUBE_parseData
 * @param p_line
 * @return
 */
LUTState CUBE_parseData(s_line* p_line)
{
    int nbVal = 0;

    nbVal = sscanf(p_line->line, "%f %f %f",    &p_line->r,
                                                &p_line->g,
                                                &p_line->b);
    if(nbVal < 3)
        return CouldNotParseTableData;

    return OK;
}



/****** INFO / STATUS / TEXT ******/






int CUBE_showInfo(s_lutStat* p_lutStat)
{
#ifndef CUBE_LIBRARY
    int i;
    printf("*************** Show LUT Info ***************\n");
    printf("title = %s \n", p_lutStat->title);
    printf("type = %i \n", p_lutStat->type);
    printf("size = %i \n", p_lutStat->size);
    printf("domainMin = ");
    for(i=0; i<p_lutStat->type; i++)
        printf("%f\t", p_lutStat->domainMin[i]);
    printf("\n");
    printf("domainMax = ");
    for(i=0; i<p_lutStat->type; i++)
        printf("%f\t", p_lutStat->domainMax[i]);
    printf("\n");


    printf("*************** End Stat ***************\n\n");
#endif
}




int CUBE_showStatus(s_cubeFile* p_cubeFile, s_line* p_line, s_lutStat* p_lutStat)
{
    printf("*******Status*******\n");
    printf("p_cubeFile.status = %i : %s\n",p_cubeFile->status, CUBE_textStatusCube(p_cubeFile));
    printf("p_line.status = %i : %s\n",p_line->status, CUBE_textStatusLine(p_line));
    printf("********************\n");
}

char* CUBE_textStatusCube(s_cubeFile* p_cubeFile)
{
    switch(p_cubeFile->status)
    {
    case EndOfFile : return str_EndOfFile;
    case CouldNotParseTableData : return str_CouldNotParseTableData;
    case CouldNotParseKeyWord : return str_CouldNotParseKeyWord;
    case LUTSizeOutOfRange : return str_LUTSizeOutOfRange;
    }
}
char* CUBE_textStatusLine(s_line* p_line)
{
    switch(p_line->status)
    {
    case parseKeyWord : return str_parseKeyWord;
    case parseData : return str_parseData;
    }
}
