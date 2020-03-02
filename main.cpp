#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <thread>
#include "Utilita.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>


using namespace std;

const basic_string<char> RICHIESTA_CHIUSURA_CONNESSIONE = "CHIUDI CONNESSIONE";
const basic_string<char> RICHIESTA_SPEGNIMENTO = "SPEGNI";


#define SHELLSCRIPT_START "#/bin/bash \necho \"Executing raspivid\"\n\
raspivid -o - -t 0 -h 300 -w 300 -n | cvlc -v stream:///dev/stdin --sout '#rtp{sdp=rtsp://:8554}' :demux=h264"


void avviaRaspivid ();

int chiudiVLCeRaspivid (Utilita utilita, FILE* fp, char dataStamp[50]);

int main ()
{
    Utilita utilita; //classe di utilità
    FILE *fp = Utilita::apriNuovoFile();
    char dataStamp[50]; //buffer che contiene la data attuale

    std::thread t1 (avviaRaspivid);

    // crea un socket
    int listeningSocket = socket (AF_INET,
                                  SOCK_STREAM,
                                  IPPROTO_TCP);
    if (listeningSocket == -1) {
        char errore[] = "Errore nel creare un socket\n";
        utilita.stampaRigaSuFile(fp,dataStamp,errore);
        cerr << errore;
        return -1;
    }

    // fai bind tra il socket e l'ip/porta correnti
    sockaddr_in hint; //definizione del tipo di indirizzo del socket
    hint.sin_family = AF_INET; //ipv4
    hint.sin_port = htons (9999); //definizione della porta del socket, convertito in big endian
    //The htons() function converts the unsigned short integer hostshort from
    //host byte order to network byte order.
    inet_pton (AF_INET,
               "0.0.0.0",
               &hint.sin_addr); //converte un numero in un array di integers
    //uso il formato IPV4, non specifico nessun indirizzo, e specifico il buffer

    if (bind (listeningSocket,
              (sockaddr *) &hint,
              sizeof (hint)) == -1) {
        char errore[] = "Non riesco a fare un bind con l'IP e/o la porta\n";
        utilita.stampaRigaSuFile(fp,dataStamp,errore);
        cerr << errore;
        return -2;
    }

    // segna il socket per farlo ascoltare
    if (listen (listeningSocket,
                3) == -1) //3 = numero massimo di connessioni accettate
    {   char errore[] = "Non riesco ad ascoltare\n";
        utilita.stampaRigaSuFile(fp,dataStamp,errore);
        cerr << errore;
        return -3;
    }

    // accettare una chiamata
    sockaddr_in clientsSocketAddress;
    socklen_t clientsSocketAddressSize = sizeof (clientsSocketAddress);
    char hostNameBuffer[NI_MAXHOST]; // NI_MAXHOST = massima dimensione buffer
    char serverNameBuffer[NI_MAXSERV];
    int clientSocket = accept (listeningSocket,
                               (sockaddr *) &clientsSocketAddress,
                               &clientsSocketAddressSize);
    if (clientSocket == -1) {
        char errore[] = "C'è un problema con la connessione del client\n";
        utilita.stampaRigaSuFile(fp,dataStamp,errore);
        cerr << errore;
        return -4;
    }

    // chiudi il socket che ascolta per una connessione
    close (listeningSocket);

    memset (hostNameBuffer,
            0,
            NI_MAXHOST); //pulisco con 0 il buffer
    memset (serverNameBuffer,
            0,
            NI_MAXSERV); //pulisco con 0 il buffer

    int result = getnameinfo ((sockaddr *) &clientsSocketAddress,
                              sizeof (clientsSocketAddress),
                              hostNameBuffer,
                              NI_MAXHOST,
                              serverNameBuffer,
                              NI_MAXSERV,
                              0); //cerco di recuperare dati dal socket collegato
    if (result == 0) {


        int x = sizeof (hostNameBuffer) + sizeof (" connesso al ") + sizeof (serverNameBuffer) +
                sizeof ("\n");
        char nomeBuffer[x];
        memset (nomeBuffer,
                0,
                x);
        strcpy (nomeBuffer,
                hostNameBuffer);
        strcat (nomeBuffer,
                " connesso al ");
        strcat (nomeBuffer,
                serverNameBuffer);
        strcat (nomeBuffer, "\n");

        cout << nomeBuffer << endl; //hostnamebuffer contiene il nome dell'host connesso
        send (clientSocket,
              nomeBuffer,
              x,
              0);
        utilita.stampaRigaSuFile(fp,dataStamp,nomeBuffer);


    }
    else {
        char errore_retrieve[] = "Errore nel retrieve automatico dei dati di connessione\n";
        cerr << errore_retrieve;
        utilita.stampaRigaSuFile(fp,dataStamp,errore_retrieve);
    }


    // ricevi e processa il messaggio
    char buffer[4096];
    char bufferUscita[4096];

    basic_string<char> inArrivo = "";
        while (true) {
            //pulisci il buffer da cose che c'erano prima
            memset (buffer,
                    0,
                    4096);
            memset (bufferUscita,
                    0,
                    4096);
            //Aspetta un messaggio
            int bytesReceived = recv (clientSocket,
                                      buffer,
                                      4096,
                                      0);
            if (bytesReceived == -1) {
                char errore[] = "C'è stato un problema di connessione... \n";
                utilita.stampaRigaSuFile (fp,
                                          dataStamp,
                                          errore);
                cerr << errore;
                break;
            }

            if (bytesReceived == 0) {
                char errore[] = "sembra che il client si sia disconnesso...\n";
                utilita.stampaRigaSuFile (fp,
                                          dataStamp,
                                          errore);
                cerr << errore;
                break;
            }

            char comando_riconosciuto[] = "Comando riconosciuto\n";

            inArrivo = string (buffer,
                               0,
                               bytesReceived);

            ////////////////////////////////////////////////
            //mostra il messaggio ricevuto
            cout << "Messaggio ricevuto dal client: " << string (buffer,
                                                                 0,
                                                                 bytesReceived) << endl;


            //manda un messaggio per far sapere che ha ricevuto
            char server_riceve_msg[] = "Il server ha ricevuto un messaggio;\n";
            strcpy (bufferUscita,
                    server_riceve_msg);
            send (clientSocket,
                  bufferUscita,
                  sizeof (server_riceve_msg) - 1,
                  0);
            utilita.stampaRigaSuFile (fp,
                                      dataStamp,
                                      server_riceve_msg);
            //////////////////////////////////////////////////////////////

            if (inArrivo.find (RICHIESTA_CHIUSURA_CONNESSIONE) != std::string::npos) {
                cout << "Trovata corrispondenza" << endl;

                strcpy (bufferUscita,
                        "CHIUSURA IN CORSO\n");
                send (clientSocket,
                      bufferUscita,
                      sizeof ("CHIUSURA IN CORSO\n") - 1,
                      0);

                char uscita[] = "Chiusura in corso \n";
                utilita.stampaRigaSuFile (fp,
                                          dataStamp,
                                          uscita);

                break;
            }
            else if (inArrivo.find (RICHIESTA_SPEGNIMENTO) != std::string::npos) {


                cout << comando_riconosciuto << endl;

                strcpy (bufferUscita,
                        "SPEGNIMENTO IN CORSO\n");
                send (clientSocket,
                      bufferUscita,
                      sizeof ("SPEGNIMENTO IN CORSO\n") - 1,
                      0);

                utilita.stampaRigaSuFile (fp,
                                          dataStamp,
                                          comando_riconosciuto);

                char spegnimento[] = "Spegnimento in corso \n";
                utilita.stampaRigaSuFile (fp,
                                          dataStamp,
                                          spegnimento);

                //chiudi TCP e spegni:
                chiudiVLCeRaspivid (utilita,
                                    fp,
                                    dataStamp);
                cout << system ("shutdown +1") << endl;
                close (clientSocket);// chiudi il socket
                return 0;
            }
            else {
                char comando_non_riconosciuto[] = "Comando non riconosciuto\n";
                utilita.stampaRigaSuFile (fp,
                                          dataStamp,
                                          comando_non_riconosciuto);
                cerr << comando_non_riconosciuto;
            }

        }

    close(listeningSocket);
        close(clientSocket);
    fflush (fp);
    fclose(fp);


        while(true)
        { //to avoid raspivid closing
            ;
        }

}

int chiudiVLCeRaspivid (Utilita utilita, FILE* fp, char dataStamp[50])
{
    if(fclose(fp) != 0)
    {
        char errore[] = "Errore nel chiudere il file di log.\n";
        utilita.stampaRigaSuFile(fp,dataStamp,errore);
        cerr << errore;
    }

    if (system ("pkill vlc") > 0 || system ("pkill raspivid") > 0) {
        char errore[] = "Errore nel chiudere vlc e/o raspivid\n";
        utilita.stampaRigaSuFile(fp,dataStamp,errore);
        cerr << errore;
        return -7;
    }
    char successo[] = "Processi vlc e raspivid chiusi senza errori\n";
    cout << successo;
    utilita.stampaRigaSuFile(fp,dataStamp,successo);
}

void avviaRaspivid ()
{
    //TODO: run script!
    pid_t pid;
    if ((pid = fork ()) > 0) { system (SHELLSCRIPT_START); }

}
