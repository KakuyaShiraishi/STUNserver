#include "StdAfx.h"
#include "StunTransaction.h"
#include "StunClientTransaction.h"
#include "StunGlobals.h"

CStunTransaction::CStunTransaction(SOCKADDR_IN sendToAddr, CStunMessage *pMessageToSend):
	m_SendToAddr (sendToAddr), m_pMessageToSend (pMessageToSend)
{
	Initialize ();
}

CStunTransaction::~CStunTransaction(void)
{
	if (m_pMessageReceived)
	{
		delete m_pMessageReceived;
	}
	closesocket (m_SendSock);
}

bool CStunTransaction::SendStunMessage (int nResult)
{
	nResult = 0;
	
	int nRetries = 0;
	fd_set fdWrite;
	timeval timeInterval = {0, 10000};
	
	while (nRetries < 5)
	{
		FD_ZERO (&fdWrite);
		FD_SET (m_SendSock, &fdWrite);

		if ((nResult = select (0, NULL, &fdWrite, NULL, &timeInterval)) 
			== SOCKET_ERROR)
		{
			nResult = WSAGetLastError ();
			return false;		
		}

		if (nResult > 0 && FD_ISSET (m_SendSock, &fdWrite))
		{
			clog << m_pMessageToSend->ToString ();

			if (sendto (m_SendSock, m_pMessageToSend->GetBuffer (), m_pMessageToSend->GetTotalLength (),
				0, (SOCKADDR*)&m_SendToAddr, sizeof (m_SendToAddr)) == SOCKET_ERROR)
			{
				nResult = WSAGetLastError ();
				return false;
			}
			return true;
		}

		--nRetries;
	}

	return false;
}



bool CStunTransaction::ValidateMessage()
{	
	if (m_pMessageReceived->IsMessageValid () == false &&
		m_pMessageReceived->HasAttribute (MESSAGE_INTEGRITY) != 
		m_pMessageToSend->HasAttribute (MESSAGE_INTEGRITY))
	{

		delete m_pMessageReceived;
		m_pMessageReceived = NULL;
		return false;
	}

	if (m_pMessageReceived->HasAttribute (MESSAGE_INTEGRITY) == true)
	{
	}

	return true;
}

bool CStunTransaction::ReceiveStunMessage(CStunMessage **pMessageReceived, int nResult)
{
	int nSizeFromAddr = sizeof (m_SendToAddr);
	
	char pBuffer [2400];
	int nBufferSize = 2400;

	if (nResult = recvfrom (m_SendSock, pBuffer, nBufferSize, 0, (SOCKADDR *)&m_SendToAddr, 
		&nSizeFromAddr) == SOCKET_ERROR)
	{
		nResult = WSAGetLastError ();
			nResult << endl;
		return false;
	}

	*pMessageReceived = StunGlobals::CreateMessage (pBuffer);

	
	return true;
}

bool CStunTransaction::Initialize()
{
	m_SendSock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	m_pMessageReceived = NULL;

	return true;
}

bool CStunTransaction::BindTo (SOCKADDR_IN sendFromAddr)
{
	if (bind (m_SendSock, (SOCKADDR *)&sendFromAddr, sizeof (SOCKADDR)) == 
		SOCKET_ERROR)
	{
		return false;
	}

	return true;
}