#pragma once

#include <fstream>
#include <iostream>

#include "StunBindingRequestMessage.h"
#include "StunSharedSecretRequestMessage.h"
#include "StunSharedSecretResponseMessage.h"
#include "StunSharedSecretErrorResponseMessage.h"
#include "StunBindingResponseMessage.h"
#include "StunBindingErrorResponseMessage.h"
#include "StunMessage.h"

using namespace std;

namespace StunGlobals
{
#ifdef LOG_TO_FILE
	ofstream m_LogFile;
#endif
	CStunMessage* CreateMessage(char *pBuffer);
	bool UnInitialize ();
	bool Initialize();
};
