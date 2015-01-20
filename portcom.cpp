#include "portcom.h"
#include <iostream>
#include <QDebug>
#include <QString>
#include <qextserialport.h>

using namespace std;

/*
 *Constructeur
 */
PortCom::PortCom()
{

}
/*
 *Destructeur
 */
PortCom::~PortCom()
{
    PortClose();
    delete PortClose;
}
/*
 *Permet d'ouvrir le port COM spécifié
 */
bool PortCom::PortOpen(char* portnumber)
{
    bool res;
    try
    {
        //ouverture du port
        QextSerialPort* PortSerie = new QextSerialPort(portnumber,portSettings);
        //uverture en mode lecture écriture
        PortSerie->open(QextSerialPort::ReadWrite);
        //ont vérifie si le port est bien ouvert
        res = PortSerie->isOpen();
    }
    catch(exception& e)
    {
        qDebug()<<"Exception lors de l'ouverture du port "<< e.what();
        res=false;
    }
    return res;
}

//-----------------------------------------
//bool PortCom::SetTimeouts(DWORD dwRxTimeout /*=5000*/,DWORD dwTxTimeout /*=5000*/)
//{
//    return 0;
//}
/*
 *Permet de configurer le port COM
 */
bool PortCom::PortSet(BaudRateType baudrate,ParityType parity,DataBitsType wordlength,StopBitsType stopbits, FlowType FlowControl, long Timeout_Millisec)
{
    portSettings.BaudRate = baudrate;
    portSettings.DataBits = wordlength;
    portSettings.Parity = parity;
    portSettings.StopBits = stopbits;
    portSettings.FlowControl = FlowControl;
    portSettings.Timeout_Millisec = Timeout_Millisec;
    return false;
}

/*
 *Permet de fermer le port COM est cours d'utilisation
 */
bool PortCom::PortClose()
{
    bool res;
    try
    {
        PortSerie->close();
        res = PortSerie->isOpen();
    }
    catch (exception& e)
    {
        qDebug()<<"Exception lors de la fermeture du port "<< e.what();
        res=false;
    }
    return !res;
}
//-----------------------------------------
bool PortCom::WritePortCom(const char * buffer,qint64 ucount)
{
    bool res=false;
    qint64 NbEcriture;
    try
    {
        //si le port est ouvert ont envoi la trame
        if (PortSerie->isOpen()==true)
        {
            NbEcriture=PortSerie->write(buffer,ucount);
            if(NbEcriture==ucount)
                res = true;
            else
                res = false;
        }
        else
           {
            res = false;
        }
    }
    catch(exception& e)
    {
        res = false;
    }

    return res;
}
//-----------------------------------------
QString PortCom::ReadPortCom()
{
    QByteArray MessageLu="";
    try
    {
        //si le port est ouvert ont envoi la trame
        if (PortSerie->isOpen()==true)
        {
            //ont lit tout ce qui à été retourné par le port com
            MessageLu = PortSerie->readAll();
        }
        else
        {
            qDebug()<<"Impossible de lire le Port Com, il est fermé";
        }
    }
    catch(exception& e)
    {
        qDebug()<<"Exception lors de la lecture du port "<< e.what();
    }
    return QString(MessageLu);
}

//-----------------------------------------
//bool PortCom::ReadChar(char &rchar )
//{
//    return false;
//}
//-----------------------------------------
//long PortCom::SizeUsedInRXBuf()
//{

//    return comstat.cbInQue;
//}
//-----------------------------------------
//bool PortCom::UseXonXoff(bool bEnable)
//{
//    return false;
//}
////-----------------------------------------
//bool PortCom::UseRtsCts(bool bEnable)
//{
//    return false;
//}
////-----------------------------------------
//bool PortCom::UseDtrDsr(bool bEnable)
//{
//    return false;
//}
//-----------------------------------------
//bool PortCom::WaitCommEvent(DWORD &rEvtMask)
//{
//    return true;
//}
//-----------------------------------------
//DWORD PortCom::GetCommMask()
//{
//    return dwMask;
//}
//-----------------------------------------
//bool PortCom::SetCommMask(DWORD EvtMask)
//{
//    return (::SetCommMask(m_hCom,EvtMask)>0);
//}
//-----------------------------------------
bool PortCom::PurgeRx()
{
    return true;
}
//-----------------------------------------
bool PortCom::PurgeTx()
{
    return true;
}
//-----------------------------------------
bool PortCom::PurgeCom()
{
    return true;
}
//-----------------------------------------
//bool PortCom::StartThread(CWnd *pParent)
//{
//   return true;
//}
//-----------------------------------------
bool PortCom::IsRXEmpty()
{
    return(false);
}
//-----------------------------------------
bool PortCom::ResumeThread()
{
    return true;
}
//-----------------------------------------
bool PortCom::StopThread()
{
    return true;
}
//-----------------------------------------
// void PortCom::OnError(DWORD dwError)
//{

//}
//-----------------------------------------
// UINT PortCom::Thread(LPVOID pParam)
//{
//    return 0;
//}
