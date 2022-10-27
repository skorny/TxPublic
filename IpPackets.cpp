#include "StdAfx.h"
#include <map>
#include <vector>
#include <set>
using namespace std;
#include "ippackets.h"

IP_Handler::IP_Handler(void)
{
}

IP_Handler::~IP_Handler(void)
{
}
bool IP_Handler::PutPacket(UCHAR* ip_packet, UCHAR*& ip_out, int& ip_size)
{
	bool q = false;
	ip_size = -1;
	if((ip_packet[1] & 0x40) == 0x0)
		m_startsection = false;
	else
		m_startsection = true;

	int the = ip_packet[11];
	if(m_startsection){
		//дообрабатывать предыдущий пакет
		if(m_all.size() > 0){
			std::pair<UCHAR*, int> pr;
			pr.second = ip_packet[4];
			pr.first = new UCHAR[ip_packet[4]];
			memcpy(pr.first, &ip_packet[4], pr.second);
			m_IpParts[the] = pr;
			m_all.erase(the);
			/**/
			if(m_all.size() == 0){
				int dim = 0;
				std::vector<std::pair<UCHAR* , int > >::iterator it;
				for(it = m_IpParts.begin(); it != m_IpParts.end(); it++){
					dim += (*it).second;
				}
				UCHAR* p = NULL;
				p = new UCHAR[dim];
				if(p == NULL)
					return false;
				ip_size = dim;
				int pos = 0;
				for(it = m_IpParts.begin(); it != m_IpParts.end(); it++){
					memcpy( &p[pos], (*it).first, (*it).second);
					pos += (*it).second;
				}
				ip_out = p;
				q = true;
			}
			/**/
		}
		
		Delete();
		int all = ip_packet[12]+1;
		for(int i = 0; i < all; i++){
			m_all.insert(i);
			m_IpParts.push_back(make_pair((UCHAR*)NULL, 0));
		}
		std::pair<UCHAR*, int> pr;
		pr.second = PACKET_LENGHT - 12;
		pr.first = new UCHAR[PACKET_LENGHT - 12];
		memcpy(pr.first, ip_packet, pr.second);
		m_IpParts[the] = pr;
		m_all.erase(the);
	}
	else{
		std::pair<UCHAR*, int> pr;
		pr.second = PACKET_LENGHT - 12;
		pr.first = new UCHAR[PACKET_LENGHT - 12];
		memcpy(pr.first, ip_packet, pr.second);
		m_IpParts[the] = pr;
		m_all.erase(the);
	}
	if(m_all.size() == 0){
		int dim = 0;
		std::vector<std::pair<UCHAR* , int > >::iterator it;
		for(it = m_IpParts.begin(); it != m_IpParts.end(); it++){
			dim += (*it).second;
		}
		UCHAR* p = NULL;
		p = new UCHAR[dim];
		if(p == NULL)
			return false;
		ip_size = dim;
		int pos = 0;
		for(it = m_IpParts.begin(); it != m_IpParts.end(); it++){
			memcpy( &p[pos], (*it).first, (*it).second);
			pos += (*it).second;
		}
		ip_out = p;
		q = true;
	}
	return q;
}
void IP_Handler::Delete()
{
	m_all.clear();
	std::vector< pair<UCHAR*, int> >::iterator it;
	for( it = m_IpParts.begin(); it != m_IpParts.end(); it++){
		UCHAR*& p = (*it).first;
		if(p)
			delete[] p;
	}
	m_IpParts.clear();
}
/////////////////////////////////

IpPackets::IpPackets(void)
{
}

IpPackets::~IpPackets(void)
{
}
bool IpPackets::Put(UCHAR* pbuf, short pid, UCHAR*& ip_packet, int& ndim)
{
	bool q = false;
	std::map< short, IP_Handler >::iterator it;
	it = m_IpHandlers.find(pid);
	if(it == m_IpHandlers.end()){
		std::pair< std::map< short, IP_Handler >::iterator , bool> pr;
		pr = m_IpHandlers.insert(make_pair(pid, IP_Handler()));
		it = pr.first;
	}
	IP_Handler& h = (*it).second;
	if(h.PutPacket(pbuf, ip_packet, ndim)){
		q = true;
	}
	return q;
}