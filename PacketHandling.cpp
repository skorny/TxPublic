#include "StdAfx.h"

using namespace ATL;
using namespace tal;
using namespace std;
#include <ByteStreampp.h>


#include "packethandling.h"

//CPacketHandling::CPacketHandling(CDVBMpeg2* pFilter)
//:m_pFilter(pFilter)
//{
//}
//
//CPacketHandling::~CPacketHandling(void)
//{
//}
//
//bool	CPacketHandling::GetOutputOrder(int InputChannel, short pid, BSM_HOSTCONTROLBLOCK* phcb)
//{
//	bool q = false;
//	return q;
//}
//
//void	CPacketHandling::FreeChannels(int InputChannel)
//{
//	BSM_HOSTCONTROLBLOCK hcb;
//	hcb.m_ActionType = BSM_AT_SLAVESEQEND;
//	PAIR p;
//	std::map< PAIR, word32, Less>::iterator it;
//	while(it != m_actual.end()){
//		p = (*it).first; 
//		if(p.InputChannel == InputChannel){
//			if(OnIPOut(p.InputChannel, p.Pid)){
//				//������� ����� ����� (*it).second � ������ IP
//				if(m_pFilter->m_outers[IP]){}
//			}
//			if(OnTVOut(p.InputChannel, p.Pid)){
//				//������� �������� ����� (*it).second � ������ TV
//			}
//			if(OnUnknownOut(p.InputChannel, p.Pid)){
//				//������� �������� ����� (*it).second � ������ �����������
//			}
//			if(OnDemUnknownOut(p.InputChannel, p.Pid)){
//				//������� �������� ����� (*it).second � ������ ���������������������� �����������
//			}
//			m_actual.erase(it);
//			continue;
//		}
//		it++;
//	}
//	return;
//}
//
//bool	CPacketHandling::OnIPOut(int InputChannel, short pid)
//{
//	bool q = false;
//	return q;
//}
//
//bool	CPacketHandling::OnTVOut(int InputChannel, short pid)
//{
//	bool q = false;
//	return q;
//}
//
//bool	CPacketHandling::OnUnknownOut(int InputChannel, short pid)
//{
//	bool q = false;
//	return q;
//}
//
//bool	CPacketHandling::OnDemUnknownOut(int InputChannel, short pid)
//{
//	bool q = false;
//	return q;
//}