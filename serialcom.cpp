
// SerialCom.cpp
// Farscape-dev@Tiscali.fr : Classe Mininum gestion port série.
#include "stdafx.h"
#include "SerialCom.h"

CCom::CCom()
{
    // Construct
    m_hCom =NULL;
    m_Ov.hEvent=NULL;
    m_comerr=0;
    m_count=0;
    m_pParent=NULL;
    m_pThread=NULL;
    m_hCloseCom=NULL;
    m_bThreadExist=false;
}
//-----------------------------------------
CCom::~CCom()
{
    // Destruct
    PortClose( );
}
//-----------------------------------------
DWORD CCom::GetError()
{
    LPVOID lpMsgBuf;
    DWORD dw;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        (dw=GetLastError()),
        0, // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL
        );
    m_StrError=CString((LPTSTR)lpMsgBuf);
    // Free the buffer.
    LocalFree( lpMsgBuf );
    return dw;
}
//-----------------------------------------
bool CCom::PortOpen(int portnumber,long baudrate,char parity,int wordlength,int stopbits)
{
   //
    char sz[20];
    m_nInputBufferSize=1050;
    m_nOutputBufferSize=1050;

    memset(&m_Ov,0,sizeof(m_Ov));

    wsprintf( sz, "\\\\.\\COM%d", portnumber);
    m_hCom = CreateFile( sz,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL );

    if ( m_hCom == INVALID_HANDLE_VALUE )
    {
        GetError();
        return false;
    }

    SetupComm( m_hCom, m_nInputBufferSize, m_nOutputBufferSize );
    ::GetCommMask( m_hCom, &m_EventMask );
    ::SetCommMask( m_hCom, 0 );

    m_dcb.fBinary = 1;
    m_dcb.fParity = 0;
    m_dcb.fNull = 0;
    m_dcb.XonChar = XON;
    m_dcb.XoffChar = XOFF;
    m_dcb.XonLim = (WORD)( ( m_nInputBufferSize) / 4 );
    m_dcb.XoffLim = (WORD)( ( m_nOutputBufferSize ) / 4 );
    m_dcb.EofChar = 0;
    m_dcb.EvtChar = 0;
    m_dcb.fOutxDsrFlow = 0;
    m_dcb.fOutxCtsFlow = 0;
    m_dcb.fDtrControl = DTR_CONTROL_ENABLE;
    m_dcb.fRtsControl = RTS_CONTROL_ENABLE;

    GetCommState( m_hCom, &m_dcb );

    GetCommTimeouts( m_hCom, &m_ComTimeouts );

    m_ComTimeouts.ReadIntervalTimeout = 1000;
    m_ComTimeouts.ReadTotalTimeoutMultiplier = 1000;
    m_ComTimeouts.ReadTotalTimeoutConstant = 1000;
    m_ComTimeouts.WriteTotalTimeoutMultiplier = 1000;
    m_ComTimeouts.WriteTotalTimeoutConstant = 1000;

    SetCommTimeouts( m_hCom, &m_ComTimeouts );

    return PortSet(baudrate,parity,wordlength,stopbits);
}
//-----------------------------------------
bool CCom::SetTimeouts(DWORD dwRxTimeout /*=5000*/,DWORD dwTxTimeout /*=5000*/)
{
//
    if(m_hCom== NULL)
    {
        TRACE0("CCom::SetTimeouts': NULL handle !");
        return 0;
    }
    COMMTIMEOUTS  commTimeOuts ;
    commTimeOuts.ReadIntervalTimeout = dwRxTimeout;
    commTimeOuts.ReadTotalTimeoutMultiplier = 1;
    commTimeOuts.ReadTotalTimeoutConstant = dwRxTimeout;
    commTimeOuts.WriteTotalTimeoutMultiplier = 1;
    commTimeOuts.WriteTotalTimeoutConstant = dwTxTimeout;

    SetCommTimeouts(m_hCom, &commTimeOuts ) ;
    return true;
}
//-----------------------------------------
bool CCom::PortSet(long baudrate,char parity,int wordlength,int stopbits )
{
    int result;

    if(!m_hCom) return false;

    m_baudrate=baudrate;
    m_parity=parity;
    m_wordlength=wordlength;
    m_stopbits=stopbits;

    char *szBaud = new char[50];
    sprintf(szBaud, "baud=%d parity=%c data=%d stop=%d", baudrate,parity,wordlength,stopbits);
    if (result=GetCommState(m_hCom,&m_dcb))
    {
        m_dcb.fRtsControl = RTS_CONTROL_ENABLE;
        result=-1;
        if (BuildCommDCB(szBaud, &m_dcb)) result=SetCommState(m_hCom, &m_dcb);
    }
    if ( result < 0 )  GetError();
    delete szBaud;
    PurgeCom();
    return( (result >0) );
}
//-----------------------------------------
bool CCom::PortClose()
{
    //
    if(!m_hCom) return false;

    if(m_pThread) // fermeture thread
    {
        do
        {
            SetEvent(m_hCloseCom);
        }
        while(m_bThreadExist);
        CloseHandle(m_hCloseCom);
    }
    if(m_Ov.hEvent) CloseHandle(m_Ov.hEvent);
    m_Ov.hEvent=NULL;
    m_hCloseCom=NULL;
    m_pThread=NULL;

    EscapeCommFunction( m_hCom, CLRDTR );
    EscapeCommFunction( m_hCom, CLRRTS );

    int status= CloseHandle( m_hCom);
    m_hCom=NULL;
    if ( status ) return true;
    GetError();
    return false;
}
//-----------------------------------------
bool CCom::WriteBuffer(const char *buffer,unsigned int ucount /*=0*/)
{
   //
    int result;
    DWORD comerr;
    COMSTAT comstat;
    unsigned int amounttowrite;
    DWORD amountwritten;

    if(!ucount) ucount=strlen(buffer);

    ClearCommError( m_hCom, &comerr, &comstat );
    m_comerr |= comerr;
    amounttowrite = m_nOutputBufferSize - comstat.cbOutQue;
    if ( ucount < amounttowrite ) amounttowrite = ucount;

    result = WriteFile( m_hCom,
        buffer,
        (int) amounttowrite,
        &amountwritten,
        &m_Ov );

    m_count = amountwritten;
    if ( result == 0 )
    {
        if ( ( comerr = GetLastError() ) != ERROR_IO_PENDING )
        {
            ClearCommError( m_hCom, &comerr, &comstat );
            m_comerr|= comerr;
        }
        else m_count = amounttowrite;
    }
    if ( m_count < ucount ) return(0);
    return( 1 ); // succes
}
//-----------------------------------------
int CCom::ReadBuffer(char *buffer,unsigned int ucount)
{
   //
    int result;
    COMSTAT comstat;
    DWORD comerr;
    DWORD countread;
    DWORD counttoread;

    ClearCommError( m_hCom, &comerr, &comstat );
    if ( comerr > 0 ) m_comerr |= comerr;

    if ( comstat.cbInQue > 0 )
    {
        if ( comstat.cbInQue < ucount ) counttoread = comstat.cbInQue;
        else                        counttoread = ucount;
        result = ReadFile( m_hCom,buffer,(int) counttoread,&countread,&m_Ov );
        m_count = countread;
        if ( result == 0 )
        {
            if ( ( comerr = GetLastError() ) != ERROR_IO_PENDING )
            {
                ClearCommError( m_hCom, &comerr, &comstat );
                m_comerr |= comerr;
            }
        }
        if ( m_count < ucount )
        {
            if ( GetOverlappedResult( m_hCom, &m_Ov, &countread, TRUE ) )
            {
                m_count = countread;
                return ( 1 );
            }
            return ( - 1);
        }
        return( 1 );
    }
    else
    {
        m_count = 0;
        return( -1 );
    }
    return -1;
}
//-----------------------------------------
bool CCom::ReadChar(char &rchar )
{
    return((ReadBuffer(&rchar,1)==0));
}
//-----------------------------------------
long CCom::SizeUsedInRXBuf()
{
    //
    COMSTAT comstat;
    DWORD comerr;

    ClearCommError( m_hCom, &comerr, &comstat );
    m_comerr |= comerr;
    return comstat.cbInQue;
}
//-----------------------------------------
bool CCom::UseXonXoff(bool bEnable)
{
    //
    int result;

    m_dcb.fInX = ( bEnable ) ? 1 : 0;
    m_dcb.fOutX = ( bEnable ) ? 1 : 0;
    result= SetCommState( m_hCom, &m_dcb );
    if ( result == TRUE ) return true;
    GetError();
    return false;
}
//-----------------------------------------
bool CCom::UseRtsCts(bool bEnable)
{
   //
    int result;

    m_dcb.fOutxCtsFlow = ( bEnable ) ? 1 : 0;
    m_dcb.fRtsControl = ( bEnable) ? RTS_CONTROL_HANDSHAKE : RTS_CONTROL_DISABLE;
    result= SetCommState( m_hCom, &m_dcb );
    if ( result == TRUE ) return true;
    GetError();
    return false;
}
//-----------------------------------------
bool CCom::UseDtrDsr(bool bEnable)
{
   //
    int result;

    m_dcb.fOutxDsrFlow = ( bEnable ) ? 1 : 0;
    m_dcb.fDtrControl = ( bEnable ) ? DTR_CONTROL_HANDSHAKE : DTR_CONTROL_DISABLE;
    result= ::SetCommState( m_hCom, &m_dcb );
    if ( result == TRUE ) return true;
    GetError();
    return false;
}
//-----------------------------------------
bool CCom::WaitCommEvent(DWORD &rEvtMask)
{
    //

    if(m_hCom== NULL)
    {
        TRACE0("CCom::WaitCommEvent': NULL handle !");
        ASSERT(FALSE);
        return false;
    }

    if(!::WaitCommEvent(m_hCom,&rEvtMask,&m_Ov))
    {
        TRACE1("\nCCom::WaitCommEvent:%d",GetLastError());
        GetError();
        return false;
    }
    return true;
}
//-----------------------------------------
DWORD CCom::GetCommMask()
{
    if(m_hCom== NULL)
    {
        TRACE0("CCom::GetCommMask': NULL handle !");
        return 0;
    }
    DWORD dwMask;
    ::GetCommMask( m_hCom, &dwMask);
    return dwMask;
}
//-----------------------------------------
bool CCom::SetCommMask(DWORD EvtMask)
{
   //
    if(!m_Ov.hEvent)
        m_Ov.hEvent=CreateEvent(NULL,   // no security attributes
                                FALSE,  // auto reset event
                                FALSE,  // not signaled
                                NULL    // no name
                                );

    SetEvent(m_Ov.hEvent);

    return (::SetCommMask(m_hCom,EvtMask)>0);

}
//-----------------------------------------
bool CCom::PurgeRx()
{
    if(m_hCom== NULL)
    {
        TRACE0("CCom::PurgeRx': NULL handle !");
        return false;
    }
    ::PurgeComm(m_hCom, PURGE_RXCLEAR);
    return true;
}
//-----------------------------------------
bool CCom::PurgeTx()
{
    if(m_hCom== NULL)
    {
        TRACE0("CCom::PurgeTx': NULL handle !");
        return false;
    }
    ::PurgeComm(m_hCom, PURGE_TXCLEAR);
    return true;
}
//-----------------------------------------
bool CCom::PurgeCom()
{
    //
    if(m_hCom== NULL)
    {
        TRACE0("CCom::PurgeCom': NULL handle !");
        return false;
    }
    // flush the port
    ::PurgeComm(m_hCom, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
    return true;
}
//-----------------------------------------
bool CCom::StartThread(CWnd *pParent)
{
    //
    SetParentNotify(pParent);

    if(!m_pParent || !PurgeCom()) return false;
    if(m_bThreadExist) // femeture thread .
    {
        do
        {
            SetEvent(m_hCloseCom);
        }
        while (m_bThreadExist);
        CloseHandle(m_hCloseCom);
    }
    m_hCloseCom =CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hArrayEvent[0]=m_hCloseCom;
    m_hArrayEvent[1]=m_Ov.hEvent;

    if (!(m_pThread = AfxBeginThread(Thread, this))) return false;

    TRACE("Le Thread Démarre\n");
    return true;
}
//-----------------------------------------
bool CCom::IsRXEmpty()
{
    long n;

    n = SizeUsedInRXBuf();
    if ( n < 0 )
    {
        OnError(GetError());
        return true;
    }
    return(( n==0));
}
//-----------------------------------------
bool CCom::ResumeThread()
{
    if(!m_pThread) return false;
    m_pThread->ResumeThread();
    return true;
}
//-----------------------------------------
bool CCom::StopThread()
{
    if(!m_pThread) return false;
    m_pThread->SuspendThread();
    return true;
}
//-----------------------------------------
/*virtual*/ void CCom::OnError(DWORD dwError)
{
//
    if(m_pParent)
        m_pParent->PostMessage(WM_CCOMERROR,(WPARAM)this,dwError);
}
//-----------------------------------------
/*static*/ UINT CCom::Thread(LPVOID pParam)
{
//
    CCom *pCom = (CCom*)pParam;
    int nResult;
    DWORD WaitEvent = 0;
    DWORD dwError=0;
    DWORD dwMaskEvent=0;

    pCom->m_bThreadExist=true; // amorce l'existance du thread
    while(1)
    {
        nResult=pCom->WaitCommEvent(dwMaskEvent);
        if (!nResult)
        {
            switch(dwError=GetLastError())
            {
            case 87:
            case ERROR_IO_PENDING:
                break;

            default:
                {
                    // erreur de communication on fait suivre ...
                    pCom->GetError();
                    pCom->OnError(dwError);
                    break;
                }
            }
        }
        else
        {
            if(pCom->IsRXEmpty()) continue;
        }
        // attente evenement pour sortie eventuelle du thread ou reception
        WaitEvent = WaitForMultipleObjects(2, pCom->m_hArrayEvent, FALSE, INFINITE);

        switch (WaitEvent)
        {
        case 0: //Fermeture Thread
                   pCom->m_bThreadExist=false;
                   AfxEndThread(1);
                   return(0);

        case 1:dwMaskEvent=pCom->GetCommMask();
                    if(dwMaskEvent & EV_RXCHAR) // reception sur la voie.
                         pCom->m_pParent->SendMessage(WM_CCOMRCV,(WPARAM)pCom,dwMaskEvent);

            if ((dwMaskEvent & EV_CTS) ||  // evenements divers.
                (dwMaskEvent & EV_RXFLAG) ||
                (dwMaskEvent & EV_BREAK) ||
                (dwMaskEvent & EV_ERR)
                || (dwMaskEvent & EV_RING))
            {
                pCom->m_pParent->SendMessage(WM_CCOMEVENT,(WPARAM)pCom,dwMaskEvent);
            }
            break;
        }
    };
    return 0;
}

// utilisation : consultation registres Modem Usr:
CCom com;
com.PortOpen(1,57600,'N',8,1);
com.UseRtsCts();

com.SetCommMask(EV_RXCHAR); // spécifie l'événement d'attente.
com.WriteBuffer("ATI7\r\n");

DWORD EvtMask;
com.WaitCommEvent(EvtMask);// attente réception
char sz[1000];
com.ReadBuffer(sz,sizeof(sz));
AfxMessageBox(CString(sz,com.GetCountRead()));
