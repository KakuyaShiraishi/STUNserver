#pragma once
#include "stuntransaction.h"
#include "StunBindingRequestMessage.h"
#include "StunBindingResponseMessage.h"
#include "StunErrorResponseMessage.h"

class CStunClientTransaction :
	public CStunTransaction
{
public:
	CStunClientTransaction (SOCKADDR_IN serverAddr, CStunBindingRequestMessage *pRequestMessage);
	~CStunClientTransaction(void);

	bool SendRequest (int nResult);
	CStunBindingResponseMessage *GetBindingResponse ();
	CStunErrorResponseMessage *GetErrorResponse ();
	STUN_MESSAGE_TYPE GetResponseType ();
	bool ReceiveResponse (int nResult);

private:
	int m_nRetransmitCount;
	bool WaitAndValidate(int nResult);
};
