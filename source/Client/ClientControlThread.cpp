// all TODOs in this file...
// TODO: #include "../Net/TCPSocket.h"
// TODO: be able to include the "../Net/TCPSocket.h", and use the DATA_BUFSIZE
// TODO: this should be defined elsewhere, because its part of a protocol
// TODO: implement stufffff!!!!!
// TODO: implement stufffff!!!!!
// TODO: implement stufffff!!!!!
// TODO: TCPSocket->send blahblahblah
// TODO: TCPSocket->send blahblahblah
// TODO: TCPSocket->send blahblahblah
// TODO: TCPSocket->send blahblahblah
// TODO: parse packet, and fill in callback parameters
// TODO: parse packet, and fill in callback parameters
// TODO: parse packet, and fill in callback parameters

#include "ClientControlThread.h"
#include "../handlerHelper.h"
// TODO: #include "../Net/TCPSocket.h"

// TODO: be able to include the "../Net/TCPSocket.h", and use the DATA_BUFSIZE
// from that header file instead of this one
#define DATA_BUFSIZE 8192

/*
 * message queue constructor parameters
 */
#define MSGQ_CAPACITY 30
#define MSGQ_ELEM_SIZE sizeof(MsgqElement)
#define SOCK_MSGQ_CAPACITY 1000
#define SOCK_MSGQ_ELEM_SIZE sizeof(SockMsgqElement)

/*
 * length of a string
 */
#define STR_LEN 128

//////////////////////
// type definitions //
//////////////////////

/*
 * message queue element types
 */
enum class MsgqType
{
    REQUEST_PACKET,
    REQUEST_DOWNLOAD,
    CANCEL_DOWNLOAD,
    CHANGE_STREAM
};

/*
 * socket message queue element types
 */
// TODO: this should be defined elsewhere, because its part of a protocol
enum class SockMsgqType
{
    DOWNLOAD_PACKET,
    RETRANSMISSION_PACKET,
    CHANGE_STREAM
};

/**
 * element that is put into the message queue.
 */
union MsgqElement
{
    int index;
    char string[STR_LEN];
};

/**
 * element that is put into the socket message queue
 */
union SockMsgqElement
{
    char data[DATA_BUFSIZE];
};

/////////////////////
// synchronization //
/////////////////////

static HANDLE access = CreateMutex(NULL, FALSE, NULL);

////////////////////////////////////////
// ClientControlThread implementation //
////////////////////////////////////////

ClientControlThread* ClientControlThread::_instance = 0;

/**
 * returns a pointer to the one and only ClientControlThread instance.
 *
 * @return   pointer to the one and only ClientControlThread instance.
 */
ClientControlThread* ClientControlThread::getInstance()
{
    // acquire synchronization objects
    WaitForSingleObject(access,INFINITE);

    // retrieve singleton and make if needed
    if(ClientControlThread::_instance == 0)
    {
        ClientControlThread::_instance = new ClientControlThread();
    }

    // release synchronization objects
    ReleaseMutex(access);

    return ClientControlThread::_instance;
}

/**
 * constructs a new {ClientControlThread} object, and initializes all its
 *   instance variables.
 */
ClientControlThread::ClientControlThread()
    :_msgq(MSGQ_CAPACITY,MSGQ_ELEM_SIZE)
    ,_sockMsgq(SOCK_MSGQ_CAPACITY,SOCK_MSGQ_ELEM_SIZE)
{
    // initialize instance variables
    _threadStopEv = CreateEvent(NULL,TRUE,FALSE,NULL);
    _thread       = INVALID_HANDLE_VALUE;

    // start the thread
    start();
}

/**
 * destructs the client control thread.
 */
ClientControlThread::~ClientControlThread()
{
    stop();
}

/**
 * posts a message to an internal message queue, informing the control thread
 *   that it should send a packet retransmission request.
 *
 * @date     2015-03-28T11:17:45-0800
 *
 * @author   Eric Tsang
 *
 * @param    index   what position the packet is for.
 */
void ClientControlThread::requestPacketRetransmission(int index)
{
    // prepare the element for insertion into the message queue
    MsgqElement element;
    element.index = index;

    // insert the element into the message queue
    _msgq.enqueue((int)MsgqType::REQUEST_PACKET,&element);
}

/**
 * posts a message to an internal message queue, informing the control thread
 *   that it should send a request to download a song.
 *
 * @date     2015-03-28T11:19:49-0800
 *
 * @author   Eric Tsang
 *
 * @param    file   [description]
 */
void ClientControlThread::requestDownload(char* file)
{
    // prepare the element for insertion into the message queue
    MsgqElement element;
    memcpy(&element.string,file,STR_LEN);

    // insert the element into the message queue
    _msgq.enqueue((int)MsgqType::REQUEST_DOWNLOAD,&element);
}

void ClientControlThread::cancelDownload(char* file)
{
    // prepare the element for insertion into the message queue
    MsgqElement element;
    memcpy(&element.string,file,STR_LEN);

    // insert the element into the message queue
    _msgq.enqueue((int)MsgqType::CANCEL_DOWNLOAD,&element);
}

void ClientControlThread::requestChangeStream(char* file)
{
    // prepare the element for insertion into the message queue
    MsgqElement element;
    memcpy(&element.string,file,STR_LEN);

    // insert the element into the message queue
    _msgq.enqueue((int)MsgqType::CHANGE_STREAM,&element);
}

void ClientControlThread::start()
{
    _startRoutine(&_thread,_threadStopEv,_threadRoutine,this);
}

void ClientControlThread::stop()
{
    _stopRoutine(&_thread,_threadStopEv);
}

void ClientControlThread::onDownloadPacket(int index, void* data, int len)
{
    // TODO: implement stufffff!!!!!
}

void ClientControlThread::onRetransmissionPacket(int index, void* data, int len)
{
    // TODO: implement stufffff!!!!!
}

void ClientControlThread::onChangeStream(char* file)
{
    // TODO: implement stufffff!!!!!
}

int ClientControlThread::_startRoutine(HANDLE* thread, HANDLE stopEvent,
    LPTHREAD_START_ROUTINE routine, void* params)
{
    // return immediately if the routine is already running
    if(*thread != INVALID_HANDLE_VALUE)
    {
        return 1;
    }

    // reset the stop event
    ResetEvent(stopEvent);

    // start the thread & return
    DWORD useless;
    *thread = CreateThread(0,0,routine,params,0,&useless);
    return (*thread == INVALID_HANDLE_VALUE);
}

int ClientControlThread::_stopRoutine(HANDLE* thread, HANDLE stopEvent)
{
    // return immediately if the routine is already stopped
    if(*thread == INVALID_HANDLE_VALUE)
    {
        return 1;
    }

    // set the stop event to stop the thread
    SetEvent(stopEvent);
    WaitForSingleObject(*thread,INFINITE);

    // invalidate thread handle, so we know it's terminated
    *thread = INVALID_HANDLE_VALUE;
    return 0;
}

DWORD WINAPI ClientControlThread::_threadRoutine(void* params)
{
    printf("Thread started...\n");

    // parse thread parameters
    ClientControlThread* dis = (ClientControlThread*) params;

    // perform the thread routine
    int breakLoop = FALSE;
    while(!breakLoop)
    {
        HANDLE handles[] = {
            dis->_threadStopEv,
            dis->_msgq.hasMessage,
            dis->_sockMsgq.hasMessage
        };
        switch(WaitForMultipleObjects(3,handles,FALSE,INFINITE))
        {
        case WAIT_OBJECT_0+0:   // stop event triggered
            breakLoop = TRUE;
            break;
        case WAIT_OBJECT_0+1:   // message queue has message
            _handleMsgqMsg(dis);
            break;
        case WAIT_OBJECT_0+2:   // socket received something
            _handleSockMsgqMsg(dis);
            break;
        default:
            fatalError("ClientControlThread::_threadRoutine WaitForMultipleObjects");
            break;
        }
    }

    // return...
    printf("Thread stopped...\n");
    return 0;
}

void ClientControlThread::_handleMsgqMsg(ClientControlThread* dis)
{
    TCHAR s[256];

    // allocate memory to hold message queue message
    MsgqType msgType;
    MsgqElement element;

    // get the message queue message
    dis->_msgq.dequeue((int*)&msgType,&element);

    // process the message queue message according to its type
    switch(msgType)
    {
    case MsgqType::REQUEST_PACKET:
        OutputDebugString(L"MsgqType::REQUEST_PACKET\n");
        // TODO: TCPSocket->send blahblahblah
        break;
    case MsgqType::REQUEST_DOWNLOAD:
        swprintf_s(s,L"MsgqType::REQUEST_DOWNLOAD: %S\n",element.string);
        OutputDebugString(s);
        // TODO: TCPSocket->send blahblahblah
        break;
    case MsgqType::CANCEL_DOWNLOAD:
        swprintf_s(s,L"MsgqType::CANCEL_DOWNLOAD: %S\n",element.string);
        OutputDebugString(s);
        // TODO: TCPSocket->send blahblahblah
        break;
    case MsgqType::CHANGE_STREAM:
        swprintf_s(s,L"MsgqType::CHANGE_STREAM: %S\n",element.string);
        OutputDebugString(s);
        // TODO: TCPSocket->send blahblahblah
        break;
    default:
        fprintf(stderr,"WARNING: received unknown message type: %d\n",msgType);
        break;
    }
}

void ClientControlThread::_handleSockMsgqMsg(ClientControlThread* dis)
{
    // allocate memory to hold message queue message
    SockMsgqType msgType;
    MsgqElement element;

    // get the message queue message
    dis->_sockMsgq.dequeue((int*)&msgType,&element);

    // process the message queue message according to its type
    switch(msgType)
    {
    case SockMsgqType::DOWNLOAD_PACKET:
        OutputDebugString(L"SockMsgqType::DOWNLOAD_PACKET\n");
        // TODO: parse packet, and fill in callback parameters
        dis->onDownloadPacket(0,0,0);
        break;
    case SockMsgqType::RETRANSMISSION_PACKET:
        OutputDebugString(L"SockMsgqType::RETRANSMISSION_PACKET\n");
        // TODO: parse packet, and fill in callback parameters
        dis->onRetransmissionPacket(0,0,0);
        break;
    case SockMsgqType::CHANGE_STREAM:
        OutputDebugString(L"SockMsgqType::CHANGE_STREAM\n");
        // TODO: parse packet, and fill in callback parameters
        dis->onChangeStream(0);
        break;
    default:
        fprintf(stderr,"WARNING: received unknown message type: %d\n",msgType);
        break;
    }
}

/*
int main(void)
{
    ClientControlThread cct;
    cct.start();
    cct.start();
    cct.start();
    cct.start();
    Sleep(10000);
    cct.stop();
    cct.stop();
    cct.stop();
    cct.stop();
    Sleep(1000);
    return 0;
}
*/