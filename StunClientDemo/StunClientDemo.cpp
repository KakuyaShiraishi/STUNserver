#include "stdafx.h"
#include "StunClientHelper.h"

int _tmain(int argc, _TCHAR* argv[])
{
	CStunClientHelper clientHelper ("stun.xten.com");
	
	NAT_TYPE Nat = clientHelper.GetNatType ();
	cout << endl;
	switch (Nat)
	{
	case NAT_TYPE::ERROR_DETECTING_NAT:
		cout << "There was an error detecting NAT." << endl;
		break;

	case NAT_TYPE::FIREWALL_BLOCKS_UDP:
		cout << "There is a firewall that blocks UDP." << endl;
		break;

	case NAT_TYPE::FULL_CONE_NAT:
		cout << "The NAT type is Full Cone NAT." << endl;
		break;

	case NAT_TYPE::OPEN_INTERNET:
		cout << "There is no NAT and directly on Open Internet." << endl;
		break;

	case NAT_TYPE::RESTRICTED_CONE_NAT:
		cout << "The NAT type is Restricted Cone NAT." << endl;
		break;

	case NAT_TYPE::RESTRICTED_PORT_CONE_NAT:
		cout << "The NAT type is Restricted Port Cone NAT." << endl;
		break;

	case NAT_TYPE::SYMMETRIC_NAT:
		cout << "The NAT type is Symmetric NAT." << endl;
		break;

	case NAT_TYPE::SYMMETRIC_UDP_FIREWALL:
		cout << "There is a symmetric UDP firewall." << endl;
		break;
	}

	return 0;
}

