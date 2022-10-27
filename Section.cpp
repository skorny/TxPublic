#include "StdAfx.h"
#include "section.h"

CSection::CSection(void)
:m_count(0),m_pbuf(NULL),m_ndim(0),table_id(0xff),m_state(SECTION_EMPTY)
{
}

CSection::~CSection(void)
{
	Free();
}

bool CSection::PutPacket(const UCHAR* packet, int pos_begin, int& pos_end)
{
	std::string er = "";
	int k = -1;
	bool q = true;
	bool payload_unit_start_indicator = false;
	if((packet[1] & 0x40) == 0x0)
		payload_unit_start_indicator = false;
	else
		payload_unit_start_indicator = true;
	if(payload_unit_start_indicator){
		if(pos_begin == 0){  //соответствует ситуации, когда в начале пакета может быть остаток предыдущей секции
			unsigned short rest = packet[4];
			if( (m_count + rest)!= m_ndim){
				q = false;
				m_state = SECTION_ERROR;
			}
			else{
				if(5+rest > PACKET_LENGHT)
					er = "error";
				memcpy(&m_pbuf[m_count], &packet[5], rest);//1
				m_count += rest;
			}
			pos_end = 4 + rest + 1;
		}
		else{//pos_begin - указывает на начало новой секции
			q = CreateNew(packet, pos_begin, pos_end);
		}
	}//если в данном пакете ни одна из секций не начинается
	else{
		int rest = PACKET_LENGHT - 4;
		if((m_count + PACKET_LENGHT - 4) > m_ndim)
			rest = m_ndim - m_count;
		memcpy(&m_pbuf[m_count], &packet[4], rest);//2
		m_count += rest;
		pos_end = PACKET_LENGHT;
	}
	return q;
}

bool CSection::CreateNew(const UCHAR* packet, int pos_begin, int& pos_end)
{
	bool q = true;
	table_id = packet[pos_begin];
	if(table_id != 0xff){
		unsigned short lenght = (packet[pos_begin + 1] & 0xf) << 8;
		lenght |= packet[pos_begin + 2];
		if(lenght == 0x345)
			bool l = true;
		lenght += 3;
		Free();	
		table_id = packet[pos_begin];
		m_ndim = lenght;
		m_pbuf = new UCHAR[m_ndim];
		if(m_pbuf == NULL)
			return false;	
		
		if(pos_begin + m_ndim <= PACKET_LENGHT){
			memcpy(m_pbuf, &packet[pos_begin], m_ndim);//3
			m_count = m_ndim;
			pos_end = pos_begin + m_ndim;
			m_state = SECTION_COMPLETE;
		}
		else{
			m_count = PACKET_LENGHT - pos_begin;
			memcpy(m_pbuf, &packet[pos_begin], m_count);//4
			pos_end = PACKET_LENGHT;
			m_state = SECTION_WAITING;
		}
	}
	else{
		q = false;
		m_state = SECTION_STUFF;
		pos_end = PACKET_LENGHT;
	}
	return q;
}

void CSection::GetSection(UCHAR*& pbuf, int& ndim)
{
	pbuf = m_pbuf;
	ndim = m_ndim;
}
SECTION_STATE CSection::GetState()
{
	if(table_id == 0xff)
		bool q = false;
	else
		bool q = true;
	if( (m_state == SECTION_COMPLETE) && (m_count != m_ndim) )
		m_state = SECTION_ERROR;
	if(m_count < m_ndim)
		m_state = SECTION_WAITING;
	if( (m_count == m_ndim) && (m_ndim > 0))
		m_state = SECTION_COMPLETE;
	if(m_count > m_ndim)
		m_state = SECTION_ERROR;
	return m_state;
}
bool CSection::IsComplete()
{
	if(GetState() == SECTION_COMPLETE)
		return true;
	else
		return false;
}
void CSection::Free()
{
	if(m_pbuf)
		delete[] m_pbuf;
	m_count = 0;
	m_ndim = 0;
	m_pbuf = NULL;
	table_id = 0xff;
	m_state = SECTION_EMPTY;
}