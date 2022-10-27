#pragma once

class IP_Handler
{
public:
	IP_Handler(void);
	~IP_Handler(void);
private:
	int																			m_sectionnumber;
	bool																		m_startsection;
	std::vector< std::pair<UCHAR* , int > > m_IpParts;
	std::set< int >													m_all;

	
	void Delete();
	
public:
	bool PutPacket(UCHAR* ip_packet, UCHAR*& ip_out, int& ip_size);
};

class IpPackets
{
public:
	IpPackets(void);
	~IpPackets(void);
	bool Put(UCHAR* pbuf, short pid, UCHAR*& ip_packet, int& ndim);
private:
	std::map< short, IP_Handler >	m_IpHandlers;
};
