#include "StdAfx.h"
#include "StunGlobals.h"

bool StunGlobals::UnInitialize ()
{
#ifdef LOG_TO_FILE
	if (m_LogFile.is_open () == true)
	{
		m_LogFile.close ();
	}
#endif
	if (WSACleanup () == SOCKET_ERROR)
	{
		return false;
	}

	return true;
}

CStunMessage* StunGlobals::CreateMessage(char *pBuffer)
{
	unsigned short nMessageType = 0;
	memcpy_s (&nMessageType, sizeof (nMessageType), pBuffer, sizeof (nMessageType));
	nMessageType = ntohs (nMessageType);

	CStunMessage *pMessage = NULL;

	switch (nMessageType)
	{
		case BINDING_REQUEST:
			pMessage = new CStunBindingRequestMessage (pBuffer);
			break;

		case BINDING_RESPONSE:
			pMessage = new CStunBindingResponseMessage (pBuffer);
			break;

		case BINDING_ERROR_RESPONSE:
			pMessage = new CStunBindingErrorResponseMessage (pBuffer);
			break;

		case SHARED_SECRET_ERROR_RESPONSE:
			pMessage = new CStunSharedSecretErrorResponseMessage (pBuffer);
			break;

		case SHARED_SECRET_REQUEST:
			pMessage = new CStunSharedSecretRequestMessage (pBuffer);
			break;

		case SHARED_SECRET_RESPONSE:
			pMessage = new CStunSharedSecretResponseMessage (pBuffer);
			break;

		default:
			break;
	}

	return pMessage;
}

bool StunGlobals::Initialize()
{
#ifdef LOG_TO_FILE
	m_LogFile.open ("stunner.log", ios_base::out | ios_base::trunc);
	if (m_LogFile.is_open () == false)
	{
		return false;
	}
	clog.rdbuf (m_LogFile.rdbuf ());
#endif
	
	int nRet = 0;
	WSADATA wsaData;

	nRet = WSAStartup (MAKEWORD (2, 2), &wsaData);
	if (nRet != 0)
	{
		nRet = WSAGetLastError ();
		return false;
	}

	return true;
}