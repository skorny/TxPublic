#include "StdAfx.h"
#include <map>
using namespace std;
#include "remains.h"

CRemains::CRemains(void)
{
	m_pos  = 0;
	m_size = 0;
	m_newflag = false;
}

CRemains::~CRemains(void)
{
	Free();
}

bool CRemains::Concatenate(int chan, UCHAR*& pbuf, int& ndim)
{
	m_newflag = false;
	UCHAR* newbuf = pbuf;
	int newdim = ndim;
	std::map< int, BLOCK_REM> ::iterator it;
	it = m_map.find(chan);
	if(it != m_map.end()){
		BLOCK_REM& b = (*it).second;
		newdim = ndim + b.m_dim;
		newbuf = NULL;
		newbuf = new UCHAR[newdim];
		if(newbuf == NULL)
			return false;
		memcpy(newbuf, b.m_remain, b.m_dim);
		memcpy(&newbuf[b.m_dim], pbuf, ndim);
		m_newflag = true;
	}
	else{
		BLOCK_REM br;
		br.m_dim = 0;
		br.m_remain = 0;
		m_map.insert(make_pair(chan, br));
	}
	m_chan = chan;
	m_pbuf = newbuf;
	m_size = newdim;
	m_pos  = 0;
	pbuf = m_pbuf;
	ndim = m_size;
	return true;
}

void	CRemains::FreeChannel(int InputChannel)
{
	std::map< int, BLOCK_REM>::iterator it;
	it = m_map.find(InputChannel);
	if(it != m_map.end()){
		UCHAR* p = (*it).second.m_remain;
		delete[] p;
		m_map.erase(it);
	}
	return;
}

bool	CRemains::GetPacket(UCHAR*& pos_beg)
{
	bool q = false;
	/*//работающий вариант, не выполн€ющий поиск синхронизации
	if(m_pos + PACKET_LENGHT < m_size){
		if(m_pbuf[m_pos] != 0x47)
			bool f = false;
		pos_beg = &m_pbuf[m_pos];
		m_pos += PACKET_LENGHT;
		q = true;
	}
	/**/
	bool f = true;
	while( f && ( m_pos + PACKET_LENGHT < m_size)){
		if( (m_pbuf[m_pos] == 0x47) && (m_pbuf[m_pos+PACKET_LENGHT] == 0x47) )
			f = false;
		else
			m_pos++;
	}
	if(!f){
		pos_beg = &m_pbuf[m_pos];
		m_pos += PACKET_LENGHT;
		q = true;
	}
	return q;
}
bool	CRemains::SaveRemain()
{
	std::map<int, BLOCK_REM>::iterator it;
	size_t s = m_map.size();
	if(s < 1)
		s = -1;
	it = m_map.find(m_chan);
	if (it == m_map.end())
		return false;
	BLOCK_REM& b = (*it).second;
	if (b.m_remain != NULL)
		delete[] b.m_remain;
	b.m_dim = m_size - m_pos;
	UCHAR* p = NULL;
	p = new UCHAR[b.m_dim];
	if (p == NULL)
		return false;
	memcpy(p, &m_pbuf[m_pos], b.m_dim);
	b.m_remain = p;
	if (m_newflag) {
		delete[] m_pbuf;
		m_pbuf = NULL;
		m_newflag = false;
	}
	return true;
}

void CRemains::Free()
{
	UCHAR* p;
	std::map< int, BLOCK_REM >::iterator it;
	for (it = m_map.begin(); it != m_map.end(); it++) {
		p = (*it).second.m_remain;
		if(p)
			delete[] (p);
	}
	m_map.clear();
	if(m_newflag){
		if(m_pbuf != NULL)
			delete[] m_pbuf;
		m_newflag = false;
		m_pbuf = NULL;
	}
}