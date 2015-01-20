#ifndef PORTCOM_H
#define PORTCOM_H
#include <qextserialport.h>



class PortCom
{
public:
    //constructeur
    PortCom();

    //destructeur
    ~PortCom();

    // ouverture du port série
    bool PortOpen(char* portnumber);
    // réglage vitesse ,parité ,longueur de la data et du bit de stop.
    // ex : 1 : Com1 , 'N' :none , 8 : data 8 bits ,   1 ou 2 : bit de stop
    bool PortSet(BaudRateType baudrate,ParityType parity,DataBitsType wordlength,StopBitsType stopbits, FlowType FlowControl, long Timeout_Millisec);
    //  Fermeture du port.
    bool PortClose();
    // Envoie d'une chaîne de caractères la longueur est facultative si elle se termine par ‘\0'
    bool WritePortCom(const char * buffer,qint64 ucount);

    // lecture d'une chaîne de caractères d'une taille donnée.
    // GetCountRead() contiendra la taille reellement lue .
    QString ReadPortCom();

    // Renvoie le nombre d'octets dans le buffer de reception avant lecture
    long SizeUsedInRXBuf();

    // renvoie true si le buffer de reception est vide.
    bool IsRXEmpty();

    // Attente d'un événement fixé par SetCommMask()
    //bool  WaitCommEvent(DWORD &rEvtMask);

    // fixe la gestion des evenements sur le port serie. Voir doc MSDN :   EV_RXCHAR
    //bool  SetCommMask(DWORD EvtMask);

    // recupere l'événement en cours.
    //DWORD GetCommMask();

    // Renvoie sous forme litterale la derniere erreur rencontrée avec GetLastError()
    //CString GetStringError(){return m_StrError;}

    // Renvoie la derniere valeur du nombre d'octets lus.
    //int   GetCountRead(){return m_count;}

    // fixe la fenetre parent pour les messages en provenance de la voie serie.
    //void SetParentNotify(CWnd *pParent){m_pParent=pParent;}

    // purge le port serie.
    bool PurgeCom();

    // purge la réception
    bool PurgeRx();

    // purge l'émission
    bool PurgeTx();

    // lancer le thread
    //bool StartThread(CWnd *pParent);

    // redemarre le thread
    bool ResumeThread();

    // stop le thread.
    bool StopThread();

    // réglage du timeout sur la reception et l'émission
    // Note par défaut le reglage de la voie est a 1s.
    //bool SetTimeouts(DWORD dwRxTimeout=5000,DWORD dwTxTimeout=5000);

    // fonction appelée en cas d'erreur detectée dans le thread.
    // envoie un message à la fenetre parent.
    //virtual void OnError(DWORD dwError);
protected:
    PortSettings        portSettings;
    QextSerialPort*     PortSerie;


};

#endif // PORTCOM_H
