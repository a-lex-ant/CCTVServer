//
// Created by lex on 02/03/20.
//

#ifndef PROVALOGGERCONCLASSI_UTILITA_H
#define PROVALOGGERCONCLASSI_UTILITA_H

#include "Utilita.h"
#include <iostream>
#include <ctime>
#include <string.h>
#include <unistd.h>

class Utilita
    {

        static std::__cxx11::basic_string<char> getDataDiOraString ();

        std::string costruisciDataInString (tm _struct);

        void valorizzaDatadiOra (char *stringPassata);

    public:
        static _IO_FILE *apriNuovoFile ();

        void stampaRigaSuFile (FILE *pFile, char *dataStampChar, char *messaggio_da_stampare);
    };


#endif //PROVALOGGERCONCLASSI_UTILITA_H
