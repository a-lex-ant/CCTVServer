#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <thread>


using namespace std;

const basic_string<char> RICHIESTA_CHIUSURA_CONNESSIONE = "CHIUDI CONNESSIONE";
const basic_string<char> RICHIESTA_SPEGNIMENTO = "SPEGNI";


#define SHELLSCRIPT_START "#/bin/bash \necho \"Executing raspivid\"\n\
raspivid -o - -t 0 -h 300 -w 300 | cvlc -v stream:///dev/stdin --sout '#rtp{sdp=rtsp://:8554}' :demux=h264"


void avviaRaspivid();

int chiudiVLCeRaspivid ();

int main ()
{
    std::thread t1(avviaRaspivid);

    // crea un socket
    int listeningSocket = socket (AF_INET,
                                  SOCK_STREAM,
                                  IPPROTO_TCP);
    if (listeningSocket == -1) {
        cerr << "Errore nel creare un socket";
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
        cerr << "Non riesco a fare un bind con l'IP e/o la porta";
        return -2;
    }

    // segna il socket per farlo ascoltare
    if (listen (listeningSocket,
                3) == -1) //3 = numero massimo di connessioni accettate
    {
        cerr << "Non riesco ad ascoltare";
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
        cerr << "C'è un problema con la connessione del client";
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
        cout << hostNameBuffer << " connesso al " << serverNameBuffer
             << endl; //hostnamebuffer contiene il nome dell'host connesso
    }
    else {
        cout << "errore nel retrieve automatico dei dati di connessione";
    }


    // mentre ricevi, mostra il messaggio ricevuto e fai echo
    char buffer[4096];
    basic_string<char> inArrivo = "";
    while (true) {
        //pulisci il buffer da cose che c'erano prima
        memset (buffer,
                0,
                4096);
        //Aspetta un messaggio
        int bytesReceived = recv (clientSocket,
                                  buffer,
                                  4096,
                                  0);
        if (bytesReceived == -1) {
            cerr << "c'è stato un problema di connessione... " << endl;
            break;
        }

        if (bytesReceived == 0) {
            cout << "sembra che il client si sia disconnesso..." << endl;
            break;
        }

        inArrivo = string (buffer,
                           0,
                           bytesReceived);


        if (inArrivo.find (RICHIESTA_CHIUSURA_CONNESSIONE) != std::string::npos) {
            cout << "Trovata corrispondenza" << endl;
            send (clientSocket,
                  "Trovata corrispondenza\n",
                  24,
                  0);
            //TODO: chiudi TCP:
            chiudiVLCeRaspivid ();
            break;

        }

        if (inArrivo.find (RICHIESTA_SPEGNIMENTO) != std::string::npos) {
            cout << "Trovata corrispondenza" << endl;
            send (clientSocket,
                  "Trovata corrispondenza\n",
                  24,
                  0);
            //TODO: chiudi TCP e spegni:
            chiudiVLCeRaspivid ();
            cout << system ("shutdown +1") << endl;
            break;
        }


        //mostra il messaggio ricevuto
        cout << "Messaggio ricevuto dal client: " << string (buffer,
                                                             0,
                                                             bytesReceived) << endl;

        //rimanda il messaggio indietro per fare echo
        send (clientSocket,
              buffer,
              bytesReceived + 1,
              0);
    }
    // chiudi il socket
    close (clientSocket);

    return 0;
}

int chiudiVLCeRaspivid ()
{
    if (system ("pkill vlc") > 0 || system ("pkill raspivid") > 0) {
        cerr << "Errore nel chiudere vlc e/o raspivid" << endl;
        return -7;
    }
    cout << "Processi vlc e raspivid chiusi senza errori" << endl;
    return 0;
}

void avviaRaspivid ()
{
    //TODO: run script!
    pid_t pid;
    if ((pid = fork ()) > 0) { system (SHELLSCRIPT_START); }

}
