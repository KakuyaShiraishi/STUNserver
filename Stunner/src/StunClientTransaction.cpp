#include "StdAfx.h"
#include "StunClientTransaction.h"

CStunClientTransaction::CStunClientTransaction (SOCKADDR_IN serverAddr, CStunBindingRequestMessage *pRequestMessage):
	CStunTransaction (serverAddr, pRequestMessage)
{
}

CStunClientTransaction::~CStunClientTransaction(void)
{
}

bool CStunClientTransaction::ReceiveResponse(int nResult)
{
	int nRetransmissionIntervals [] = {100, 300, 700, 1500, 3100, 4700, 6300, 7900, 9500};
	int nRetransmitCount = 1;
	
	int nMaxTries = 0;

#ifdef STRICT_IMPLEMENTATION
	nMaxTries = sizeof (nRetransmissionIntervals)/sizeof (*nRetransmissionIntervals);
#else
	nMaxTries = 4;
#endif

	nResult = 0;
	fd_set fdRead;
	timeval timeInterval = {0, 0};

	while (nRetransmitCount < nMaxTries)
	{
		FD_ZERO (&fdRead);
		FD_SET (m_SendSock, &fdRead);

		timeInterval.tv_usec = nRetransmissionIntervals[nRetransmitCount] * 1000;
		if ((nResult = select (0, &fdRead, NULL, NULL, &timeInterval)) 
			== SOCKET_ERROR)
		{
			nResult = WSAGetLastError ();
			return false;		
		}

		if (nResult > 0 && FD_ISSET (m_SendSock, &fdRead))
		{
			if (ReceiveStunMessage (&m_pMessageReceived, nResult) == false)
			{
				return false;
			}

			if (ValidateMessage () == false)
			{
				return false;
			}

#ifdef STRICT_IMPLEMENTATION
			if (WaitAndValidate (nResult) == false)
			{
				delete m_pMessageReceived;
				m_pMessageReceived = NULL;
				return false;
			}
#endif
			if (m_pMessageReceived->GetMessageType () == BINDING_ERROR_RESPONSE ||
				m_pMessageReceived->GetMessageType () == SHARED_SECRET_ERROR_RESPONSE)
			{
				CStunErrorResponseMessage *pErrorResponseMessage = 
					(CStunErrorResponseMessage *)m_pMessageReceived;
				
				int nErrorCode = 0;
				if (pErrorResponseMessage->GetErrorCode (nErrorCode) == false)
				{
					return true;
				}

				switch (nErrorCode)
				{
				case 430:
					break;

				case 432:
					break;

				case 401:
					break;

				case 420:
					break;
				}
			}

			return true;
		}
		else
		{
			++nRetransmitCount;
				nRetransmitCount - 1 << endl;
		
			if (SendStunMessage (nResult) == false)
			{
				return false;
			}
		}

		timeInterval.tv_usec = nRetransmissionIntervals [nRetransmitCount];
	}

	return false;
}

bool CStunClientTransaction::SendRequest (int nResult)
{
	nResult = 0;

	if (SendStunMessage (nResult) == false)
	{
		return false;
	}

	if (ReceiveResponse (nResult) == false)
	{
		return false;
	}

	return true;
}

CStunBindingResponseMessage *CStunClientTransaction::GetBindingResponse()
{
	if (m_pMessageReceived && m_pMessageReceived->GetMessageType () == BINDING_RESPONSE)
	{
		return (CStunBindingResponseMessage *)m_pMessageReceived;
	}

	return NULL;
}

STUN_MESSAGE_TYPE CStunClientTransaction::GetResponseType()
{
	if (m_pMessageReceived)
	{
		return m_pMessageReceived->GetMessageType ();
	}

	return -1;
}

CStunErrorResponseMessage *CStunClientTransaction::GetErrorResponse()
{
	if (m_pMessageReceived && m_pMessageReceived->GetMessageType () != BINDING_RESPONSE)
	{
		return (CStunErrorResponseMessage *)m_pMessageReceived;
	}

	return NULL;
}

bool CStunClientTransaction::WaitAndValidate(int nResult)
{
	SYSTEMTIME CurrentTime, StartingTime;
	::GetSystemTime (&StartingTime);
	
	int nElapsedSeconds = 0;
	bool bRet = true, bFlag = false;

	fd_set fdRead;
	timeval timeInterval = {0, 0};

	CStunMessage *pPendingResponse = NULL;
	do
	{
		::GetSystemTime (&CurrentTime);
		nElapsedSeconds = CurrentTime.wSecond - StartingTime.wSecond;

		if (nElapsedSeconds < 0)
		{
			nElapsedSeconds += 60;
		}

		if (nElapsedSeconds == 10)
		{
			bFlag = true;
		}

		FD_ZERO (&fdRead);
		FD_SET (m_SendSock, &fdRead);

		timeInterval.tv_sec = 10 - nElapsedSeconds;

		if ((nResult = select (0, &fdRead, NULL, NULL, &timeInterval))
			== SOCKET_ERROR)
		{
			bFlag = true;		
			bRet = false;
		}

		if (nResult > 0 && FD_ISSET (m_SendSock, &fdRead))
		{			
			if (ReceiveStunMessage (&pPendingResponse, nResult) == false)
			{
				bRet = false;
				bFlag = true;
			}

			if (strcmp (pPendingResponse->GetBuffer (), m_pMessageReceived->GetBuffer ()) != 0)
			{
					endl;
				bRet = false;
				bFlag = true;
			}

			delete pPendingResponse;
		}
	}
	while (bFlag == false);

	return bRet;
}
