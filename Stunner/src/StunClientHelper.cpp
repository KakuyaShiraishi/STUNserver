#include "StdAfx.h"
#include "StunClientHelper.h"

CStunClientHelper::CStunClientHelper(SOCKADDR_IN serverAddr): m_pClientTransaction (NULL), m_pBindingRequest (NULL),
	m_bInitialize (false)
{
	m_bInitialize = StunGlobals::Initialize ();
	m_serverAddr = serverAddr;
}

CStunClientHelper::CStunClientHelper (const char *pszServer): m_pClientTransaction (NULL), m_pBindingRequest (NULL),
	m_bInitialize (false)
{
	m_bInitialize = StunGlobals::Initialize ();

	if (m_bInitialize == false)
	{
		return;
	}

	hostent *pHostent = gethostbyname (pszServer);
	if (pHostent == NULL)
	{
		m_bInitialize = false;
		return;
	}

	memcpy_s (&m_serverAddr.sin_addr, sizeof (m_serverAddr.sin_addr), 
		pHostent->h_addr_list [0], sizeof (m_serverAddr.sin_addr));
	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_port = htons (3478);
}

CStunClientHelper::~CStunClientHelper(void)
{
	if (m_pClientTransaction)
	{
		delete m_pClientTransaction;
	}

	if (m_pBindingRequest)
	{
		delete m_pBindingRequest;
	}

	StunGlobals::UnInitialize ();
}

bool CStunClientHelper::TestOne(SOCKADDR_IN serverAddr, SOCKADDR_IN sendFromAddr, CStunMessage **pResponseMessage)
{
	if (m_bInitialize == false)
	{
		return false;
	}

	if (m_pClientTransaction)
	{
		delete m_pClientTransaction;
		m_pClientTransaction = NULL;
	}

	if (m_pBindingRequest)
	{
		delete m_pBindingRequest;
		m_pBindingRequest = NULL;
	}
	
	m_pBindingRequest = new CStunBindingRequestMessage ();
	m_pClientTransaction = new CStunClientTransaction (serverAddr, m_pBindingRequest);
	m_pClientTransaction->BindTo (sendFromAddr);

	*pResponseMessage = NULL;
	int nResult = 0;

	if (m_pClientTransaction->SendRequest (nResult) == false)
	{
		return false;
	}
	
	if (m_pClientTransaction->ReceiveResponse (nResult) == false)
	{		
		return false;
	}

	if (m_pClientTransaction->GetResponseType () == BINDING_RESPONSE)
	{
		*pResponseMessage = m_pClientTransaction->GetBindingResponse();
	}
	else if (m_pClientTransaction->GetResponseType () == BINDING_ERROR_RESPONSE ||
			 m_pClientTransaction->GetResponseType () == SHARED_SECRET_ERROR_RESPONSE)
	{
		*pResponseMessage = m_pClientTransaction->GetErrorResponse ();
	}

	return true;
}

bool CStunClientHelper::TestTwo(SOCKADDR_IN serverAddr, SOCKADDR_IN sendFromAddr, CStunMessage **pResponseMessage)
{
	if (m_bInitialize == false)
	{
		return false;
	}

	if (m_pClientTransaction)
	{
		delete m_pClientTransaction;
		m_pClientTransaction = NULL;
	}

	if (m_pBindingRequest)
	{
		delete m_pBindingRequest;
		m_pBindingRequest = NULL;
	}

	m_pBindingRequest = new CStunBindingRequestMessage ();
	m_pClientTransaction = new CStunClientTransaction (serverAddr, m_pBindingRequest);
	m_pClientTransaction->BindTo (sendFromAddr);

	*pResponseMessage = NULL;
	int nResult = 0;

	m_pBindingRequest->AddChangeRequestAttribute (CHANGE_PORT | CHANGE_IP);

	if (m_pClientTransaction->SendRequest (nResult) == false)
	{
		return false;
	}
	
	if (m_pClientTransaction->ReceiveResponse (nResult) == false)
	{		
		return false;
	}

	if (m_pClientTransaction->GetResponseType () == BINDING_RESPONSE)
	{
		*pResponseMessage = m_pClientTransaction->GetBindingResponse();
	}
	else if (m_pClientTransaction->GetResponseType () == BINDING_ERROR_RESPONSE ||
			 m_pClientTransaction->GetResponseType () == SHARED_SECRET_ERROR_RESPONSE)
	{
		*pResponseMessage = m_pClientTransaction->GetErrorResponse ();
	}

	return true;
}

bool CStunClientHelper::TestThree(SOCKADDR_IN serverAddr, SOCKADDR_IN sendFromAddr, CStunMessage **pResponseMessage)
{
	if (m_bInitialize == false)
	{
		return false;
	}

	if (m_pClientTransaction)
	{
		delete m_pClientTransaction;
		m_pClientTransaction = NULL;
	}

	if (m_pBindingRequest)
	{
		delete m_pBindingRequest;
		m_pBindingRequest = NULL;
	}

	m_pBindingRequest = new CStunBindingRequestMessage ();
	m_pClientTransaction = new CStunClientTransaction (serverAddr, m_pBindingRequest);
	m_pClientTransaction->BindTo (sendFromAddr);

	*pResponseMessage = NULL;
	int nResult = 0;

	m_pBindingRequest->AddChangeRequestAttribute (CHANGE_PORT);

	if (m_pClientTransaction->SendRequest (nResult) == false)
	{
		return false;
	}
	
	if (m_pClientTransaction->ReceiveResponse (nResult) == false)
	{		
		return false;
	}

	if (m_pClientTransaction->GetResponseType () == BINDING_RESPONSE)
	{
		*pResponseMessage = m_pClientTransaction->GetBindingResponse();
	}
	else if (m_pClientTransaction->GetResponseType () == BINDING_ERROR_RESPONSE ||
			 m_pClientTransaction->GetResponseType () == SHARED_SECRET_ERROR_RESPONSE)
	{
		*pResponseMessage = m_pClientTransaction->GetErrorResponse ();
	}

	return true;
}

NAT_TYPE CStunClientHelper::GetNatType()
{
	
	if (m_bInitialize == false)
	{
		return NAT_TYPE::ERROR_DETECTING_NAT;
	}

	bool bRet = false;
	
	SOCKADDR_IN sendFromAddr;
	char szHostName [MAX_PATH];
	if (gethostname (szHostName, MAX_PATH) == SOCKET_ERROR)
	{
		return NAT_TYPE::ERROR_DETECTING_NAT;
	}

	hostent *pHostent = gethostbyname (szHostName);
	if (pHostent == 0)
	{
		return NAT_TYPE::ERROR_DETECTING_NAT;
	}

	memcpy_s (&sendFromAddr.sin_addr, sizeof (sendFromAddr.sin_addr), 
		pHostent->h_addr_list [0], sizeof (sendFromAddr.sin_addr));

	sendFromAddr.sin_family = AF_INET;
	sendFromAddr.sin_port = GetRandomPort ();

	CStunMessage *pResponseMessage = NULL;

	bRet = TestOne (m_serverAddr, sendFromAddr, &pResponseMessage);

	if (bRet == false)
	{
		return NAT_TYPE::FIREWALL_BLOCKS_UDP;
	}

	if (pResponseMessage->GetMessageType () != BINDING_RESPONSE)
	{
		return NAT_TYPE::ERROR_DETECTING_NAT;
	}

	SOCKADDR_IN mappedAddress = {0}, sourceAddress = {0}, changedAddress = {0}, mappedAddress1 = {0};
	((CStunBindingResponseMessage *)pResponseMessage)->GetMappedAddress (&mappedAddress);
	((CStunBindingResponseMessage *)pResponseMessage)->GetSourceAddress (&sourceAddress);
	((CStunBindingResponseMessage *)pResponseMessage)->GetChangedAddress (&changedAddress);

	if (mappedAddress.sin_port == sourceAddress.sin_port &&
		mappedAddress.sin_family == sourceAddress.sin_family &&
		mappedAddress.sin_addr.S_un.S_addr == sourceAddress.sin_addr.S_un.S_addr)
	{
		bRet = TestTwo (m_serverAddr, sendFromAddr, &pResponseMessage);

		if (bRet == true)
		{
			return NAT_TYPE::OPEN_INTERNET;
		}
		else
		{
			return NAT_TYPE::SYMMETRIC_UDP_FIREWALL;
		}
	}
	else
	{
		bRet = TestTwo (m_serverAddr, sendFromAddr, &pResponseMessage);

		if (bRet == true)
		{
			return NAT_TYPE::FULL_CONE_NAT;
		}
		else
		{
			bRet = TestOne (changedAddress, sendFromAddr, &pResponseMessage);
		
			if (bRet == false)
			{				
				return NAT_TYPE::ERROR_DETECTING_NAT;
			}

			if (pResponseMessage->GetMessageType () != BINDING_RESPONSE)
			{				
				return NAT_TYPE::ERROR_DETECTING_NAT;
			}

			((CStunBindingResponseMessage *)pResponseMessage)->GetMappedAddress (&mappedAddress1);

			if (mappedAddress.sin_port != mappedAddress1.sin_port ||
				mappedAddress.sin_family != mappedAddress1.sin_family ||
				mappedAddress.sin_addr.S_un.S_addr != mappedAddress1.sin_addr.S_un.S_addr)
			{
				return NAT_TYPE::SYMMETRIC_NAT;
			}

			bRet = TestThree (m_serverAddr, sendFromAddr, &pResponseMessage);

			if (bRet == false)
			{
				return NAT_TYPE::RESTRICTED_PORT_CONE_NAT;
			}
			else
			{
				return NAT_TYPE::RESTRICTED_CONE_NAT;
			}
		}
	}
}

bool CStunClientHelper::GetStunMappedAddress (SOCKADDR_IN *pAddr)
{
	if (m_bInitialize == false)
	{
		return false;
	}

	if (m_pClientTransaction)
	{
		delete m_pClientTransaction;
		m_pClientTransaction = NULL;
	}

	if (m_pBindingRequest)
	{
		delete m_pBindingRequest;
		m_pBindingRequest = NULL;
	}
	
	m_pBindingRequest = new CStunBindingRequestMessage ();
	m_pClientTransaction = new CStunClientTransaction (m_serverAddr, m_pBindingRequest);

	int nResult = 0;

	if (m_pClientTransaction->SendRequest (nResult) == false)
	{
		return false;
	}
	
	if (m_pClientTransaction->ReceiveResponse (nResult) == false)
	{		
		return false;
	}

	if (m_pClientTransaction->GetResponseType () == BINDING_RESPONSE)
	{
		if (m_pClientTransaction->GetBindingResponse ()->GetMappedAddress (pAddr) == false)
		{
			return false;
		}
		return true;
	}
	else if (m_pClientTransaction->GetResponseType () == BINDING_ERROR_RESPONSE ||
			 m_pClientTransaction->GetResponseType () == SHARED_SECRET_ERROR_RESPONSE)
	{
		unsigned short nErrorCode = 0;
		if (m_pClientTransaction->GetErrorResponse ()->GetErrorCode (nErrorCode) == false)
		{
			return false;
		}

		return false;
	}

	return true;
}

unsigned int CStunClientHelper::GetRandomPort ()
{
	SYSTEMTIME time;
	::GetSystemTime (&time);

	srand (time.wMilliseconds);
	return rand () + 10000;
}