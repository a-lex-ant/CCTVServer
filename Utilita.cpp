//
// Created by lex on 02/03/20.
//

#include "Utilita.h"
#include <iostream>
#include <ctime>
#include <string.h>

FILE * Utilita::apriNuovoFile ()
{
    std::string dataInString = getDataDiOraString();
    std::string nomefile = "log" + dataInString
                           + ".txt";
    const char * nomefileChar = nomefile.c_str();
    FILE *fp = fopen(nomefileChar, "a+");
    if (fp == NULL)
    {
        std::cerr << "File non apribile." << std::endl;
        exit(EXIT_FAILURE);
    }
    return fp;
}

std::string Utilita::getDataDiOraString ()
{
    time_t ora = time(NULL);
    struct tm _structOra = *localtime(&ora);
    Utilita util;
    std::string dataInString = util.costruisciDataInString (_structOra);
    return dataInString;
}


void Utilita::stampaRigaSuFile (FILE *pFile, char dataStampChar[50], char messaggio_da_stampare[50])
{
    valorizzaDatadiOra (dataStampChar);
    char tab[] = "\t";
    fprintf (pFile, dataStampChar);
    fprintf (pFile, tab);
    fprintf (pFile, messaggio_da_stampare);
}

std::string Utilita::costruisciDataInString (tm _struct)
{

    int anno = (int)(_struct.tm_year + 1900);
    int giorno = (int)_struct.tm_mday;
    int mese = (int)_struct.tm_mon + 1;
    int ora = (int)_struct.tm_hour;
    int minuti = (int)_struct.tm_min;
    int secondi = (int)_struct.tm_sec;

    char buffer [100];
    snprintf ( buffer, 100, "%04d-%02d-%02d@%02d:%02d:%02d", anno,mese, giorno, ora, minuti,
               secondi);
    puts (buffer);

    std::string dataFinale(buffer);

    return dataFinale;
}

void Utilita::valorizzaDatadiOra (char *stringPassata)
{
    std::string dataInString = getDataDiOraString();
    const char * dataInChar = dataInString.c_str();
    strncpy(stringPassata, dataInChar, 20);
}