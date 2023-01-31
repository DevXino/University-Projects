#include "gpUtilities.h"
#include "gpLib.h"

int main(int argc, char** argv){

    // Controllo degli argomenti passati da linea di comando
    if(argc != 2){
        fprintf(stderr, "usage: %s <Port>\n", argv[0]);
        exit(-1);
    }

    // Definizione dell'Indirizzo
    unsigned short port = strtous(argv[1]);
    char* IPaddress = "127.0.0.1";
    struct sockaddr_in serverGAddr = getInitAddr(AF_INET, port, IPaddress);

    // Creazione della connessione e connessione al ServerG
    int serverGSockfd = Socket(PF_INET, SOCK_STREAM, 0);
    Connect(serverGSockfd, serverGAddr, sizeof(serverGAddr));

    //Inserimento da parte dell'utente del codice fiscale
    requestPacket rp;
    fprintf(stdout, "Inserire Tessera Sanitaria: ");
    scanf(" %s", rp.SSN);
    rp.SSN[SSNSIZE-1] = '\0';
    rp.code = CHECK_GREEN_PASS_VALIDITY;

    // Invio Codice della tessera sanitaria e codice operazione al ServerG  (dunque il pacchetto rp)
    if(fullWrite(serverGSockfd, (void*)&rp, sizeof(rp)) != 0){
        fprintf(stderr, "fullWrite error\n");
        exit(-1);
    }

    // Attesa dell'esito da parte del ServerG
    gpInfoPacket gpInfo;
    if(fullRead(serverGSockfd, (void*)&gpInfo, sizeof(gpInfo)) != 0){
        fprintf(stderr, "fullRead error\n");
        exit(-1);
    }

    // Analisi dell'esito
    switch(gpInfo.opResult){
        case GP_VALID:
            printf("\nControllo effettuato con successo per: %s.\n", gpInfo.SSN);
            printf("Il Green Pass risulta valido, e scade il: %s\n", ctime(&gpInfo.expDate));
            break;
        case GP_NOT_VALID:
            printf("\nControllo effettuato con successo per: %s.\n", gpInfo.SSN);
            printf("Il Green Pass non risulta valido\n");
            break;
        case GP_NOT_FOUND:
            printf("\nIl Green Pass associato a %s non risulta essere presente nel sistema.\n", gpInfo.SSN);
            printf("Riprovare inserendo un nuovo numero di Tessera Sanitaria\n");
            break;
        case SSN_ERROR:
            printf("\nIl numero di caratteri inserito e' insufficiente.\n");
            printf("Il codice fiscale italiano ha 16 caratteri.\n\n");
            break;
        default:
            break;
    }

    Close(serverGSockfd);
    return 0;
}