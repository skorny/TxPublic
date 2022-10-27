#include "StdAfx.h"
#include <map>
#include <fstream>
using namespace ATL;
using namespace tal;
using namespace std;
#include <devctlpp.h>
#include <sessionpp.h>

#include "Section.h"
#include "TSanalyser.h"

CTSAnalyser::CTSAnalyser()
{
	HRESULT hr = S_FALSE;
	m_secCount = 0;
	std::string dir = "";
	{
		DWORD wo = 0;
		TCHAR p[200];
		DWORD i = 0;
		bool q = true;
		wo = ::GetCurrentDirectory(200,p);
		for(i = 0; i < wo; i++)
			dir += p[i];
		dir += "\\Section";
		const char* dirname = dir.data();
		hr = ::SetCurrentDirectory(dirname);
		bool q1 = true;
		if(SUCCEEDED(hr)){
		}
	}
}

CTSAnalyser::~CTSAnalyser()
{
	Free();
}

HRESULT CTSAnalyser::PutPacket(const UCHAR* packet, short pid)
{
	std::pair< std::map< short , CSection>::iterator, bool > pr;
	pr.first = m_sections.find(pid);
	if(pr.first == m_sections.end()){
		pr = m_sections.insert(make_pair(pid, CSection()));
		if(pr.second == false)
			return E_FAIL;
	}
	bool q = true;
	CSection& _s_ = (*(pr.first)).second;
	int pos = 0;
	int p = 0;
	UCHAR* section = NULL;
	int dim = 0;
	while(pos < PACKET_LENGHT){
		_s_.PutPacket(packet, pos, p);
		if(_s_.IsComplete()){
			std::ostringstream os;
			os<<m_secCount;
			std::string FileName = "sec_";
			FileName += os.str();
			std::fstream of;
			//of.open(FileName.c_str(), std::ios_base::out);
			//_s_.GetSection(section, dim);
			//int k = 0;
			//for(int i = 0; i < dim; i++){
				//if(k == 16){
					//of<<std::endl;
					//k = 0;
				//}
				//of<<hexw(2)<<(int)section[i]<<" ";
				//k++;
			//}
			
			//в данный момент имеется полная секция PID пакетов, в которых она переносится
			//(и следовательно табличный идентификатор секции)
			//требуется анализировать секцию
			q = AnalyseSection(&_s_, pid);
			m_secCount++;
			if(!q)
				pos = PACKET_LENGHT;
			_s_.Free();

		}
		pos = p;
	}
	if(!q)
		return E_FAIL;
	else
		return S_OK;
}

bool CTSAnalyser::AnalyseSection(CSection* s, short pid)
{
	/*
	std::ostringstream os;
	os<<m_secCount;
	std::string FileName = "sec_";
	FileName += os.str();
	std::fstream of;
	UCHAR* section = NULL;
	int dim = -1;
	int k = 0;
	/**/
	bool q = true;
	if(pid == 0x9c5)
		q = true;
	switch(pid){
		case NIT:
			break;
		case PAT:
			/*			
			of.open(FileName.c_str(), std::ios_base::out);
			s->GetSection(section, dim);
			for(int i = 0; i < dim; i++){
				if(k == 16){
					of<<std::endl;
					k = 0;
				}
				of<<hexw(2)<<(int)section[i]<<" ";				
				k++;
			}
			/**/
			AnalysePAT(s, pid);
			break;
		case CAT:
			break;
		default:
			std::pair< std::map<short, DATA_PID>::iterator , bool > pr;
			pr.first = m_Pids.find(pid);
			if(pr.first == m_Pids.end()){
				pr = m_Pids.insert(make_pair(pid, DATA_PID()));
				if(!pr.second)
					return false;
				DATA_PID& d = (*(pr.first)).second;
				d.desc_in_SI = false;
				d.program_number = -1;
				d.stream_type = "unknown";
				d.table_id = s->table_id;
				d.TS_id = -1;
				d.count = 0;
				d.chek = false;
			}
			else{
				DATA_PID& d = (*(pr.first)).second;
				if(d.stream_type == "PMT_PID")
				{
					AnalysePMT(s, pid);
				}
				else{
				if(d.stream_type == "NIT_PID")
				{}
				else
					d.count ++;
				}
			}
			break;
	}
	return q;
}
bool CTSAnalyser::AnalysePAT(CSection* s, short pid)
{
	if(s->table_id != 0)
		return false;
	bool q = true;
	UCHAR* p = NULL;
	int dim = 0;
	s->GetSection(p, dim);
	int TS_id = p[3]<<8;
	TS_id |= p[4];
	int key = 0;
	key  = (p[3]<<16);
	key |= (p[4]<<8);
	key |= (p[5]&0x3e);
	std::pair< std::map<int, pair<UCHAR*, int> >::iterator , bool> pr;//it;
	pr.first = m_PATs.find(key);
	if(pr.first == m_PATs.end()){
		pair<UCHAR*, int> sub_obj;
		sub_obj.first = new UCHAR[dim];
		sub_obj.second = dim;
		memcpy(sub_obj.first, p, dim);
		std::pair< int, pair<UCHAR*, int> > obj;
		obj.first = key;
		obj.second = sub_obj;
		pr = m_PATs.insert(obj);
		m_Upgrade = true;
		int			program_number = -1;
		short		PMT_Pid = 0x1ff;
		for(int i = 8; i< s->m_ndim-4; ){
			program_number = p[i]<<8;
			program_number |= p[i+1];
			if(program_number == 0){
				PMT_Pid = (p[i+2] & 0x1f) << 8;
				PMT_Pid |= p[i+3];
				if(PMT_Pid == 0x9c5)
					q = true;
				std::pair< std::map< short, DATA_PID>::iterator , bool> pr;
				pr = m_Pids.insert(make_pair(PMT_Pid,DATA_PID()));
				DATA_PID& pd = (*(pr.first)).second;
				pd.count = 0;
				pd.desc_in_SI = true;
				pd.program_number = program_number;
				pd.stream_type = "PMT_NIT";
				pd.table_id = s->table_id;
				pd.TS_id = TS_id;
			}
			else{
				PMT_Pid = (p[i+2] & 0x1f) << 8;
				PMT_Pid |= p[i+3];
				std::pair< std::map< short, DATA_PID>::iterator , bool> pr;
				pr = m_Pids.insert(make_pair(PMT_Pid,DATA_PID()));
				DATA_PID& pd = (*(pr.first)).second;
				pd.count = 0;
				pd.desc_in_SI = true;
				pd.program_number = program_number;
				pd.stream_type = "PMT_PID";
				pd.table_id = s->table_id;
				pd.TS_id = TS_id;
			}
			i += 4;
		}
	}
	return q;
}

bool CTSAnalyser::AnalysePMT(CSection* s, short pid)
{
	if(s->table_id != 0x2)
		return false;
	UCHAR* p = NULL;
	int dim  = -1;
	s->GetSection(p, dim);
	int k = 0;
	int key = 0;						//ключ - объединение номера программы и версии таблицы
	short prog_number = (p[3]<<8);
	prog_number |= p[4];
	key  = (p[3]<<16);
	key |= (p[4]<<8);
	key |= (p[5]&0x3e);
	k = (prog_number<<8);
	k |= (p[5]&0x3e);
	if(k != key)
		bool q = false;
	std::pair<std::map<int, pair<UCHAR*, int> >::iterator , bool> pr;
	pr.first = m_PMTs.find(key);
	if(pr.first == m_PMTs.end()){
		//сохранить секцию PMT
		pair<UCHAR*, int> sub_obj;
		sub_obj.first = new UCHAR[dim];
		sub_obj.second = dim;
		memcpy(sub_obj.first, p, dim);
		pair<int, pair<UCHAR*, int> > obj;
		obj.first = key;
		obj.second = sub_obj;
		m_PMTs.insert(obj);
		//анализировать секцию PMT
		/**/
		std::ostringstream os;
		os<<m_secCount;
		std::string FileName = "sec_";
		FileName += os.str();
		std::fstream of;
		of.open(FileName.c_str(), std::ios_base::out);
		of<<hexw(2)<<(int)prog_number<<std::endl;
		for(int i = 0; i < dim; i++){
			if(k == 16){
				of<<std::endl;
					k = 0;
				}
				of<<hexw(2)<<(int)p[i]<<" ";				
				k++;
			}
		/**/

	}
	return true;
}

bool CTSAnalyser::UpgradeData()
{
	bool q = false;
	q = m_Upgrade;
	return q;
}

HRESULT CTSAnalyser::DoDataExchangeDB()
{
	HRESULT hr = S_FALSE;
	m_Upgrade = false;
	return hr;
}

void CTSAnalyser::Free()
{
	std::map<short, CSection>::iterator it;
	for(it = m_sections.begin(); it != m_sections.end(); it++){
		CSection&	s = (*it).second;
		s.Free();
	}
	std::map<int, pair<UCHAR*, int> >::iterator i;
	for(i = m_PATs.begin(); i != m_PATs.end(); i++){
		std::pair<UCHAR*, int>& p = (*i).second;
		if(p.second > 0)
			delete p.first;
	}
	m_PATs.clear();

	for(i = m_PMTs.begin(); i != m_PMTs.end(); i++){
		std::pair<UCHAR*, int>& p = (*i).second;
		if(p.second > 0)
			delete p.first;
	}
	m_PMTs.clear();
}

/*****************************************************************************/
/*			CAnalyser																														 */
/*****************************************************************************/
CAnalyser::CAnalyser()
{
}

CAnalyser::~CAnalyser()
{
}

HRESULT CAnalyser::PutPacket(int InChann, const UCHAR* packet, short pid)
{
	HRESULT hr = S_OK;
	std::pair< std::map<int, CTSAnalyser>::iterator , bool > pr;
	pr.first = m_TsAnalyser.find(InChann);
	if(pr.first == m_TsAnalyser.end()){
	CTSAnalyser a;
	a.m_prev_counter = packet[3]&0xf;
	pr = m_TsAnalyser.insert(make_pair(InChann, a));
	if(!pr.second)
		hr = E_FAIL;
	}
	if(SUCCEEDED(hr)){
		std::map<int, CTSAnalyser>::iterator it = pr.first;
		//m_current = &(((pr.first)).second);
		CTSAnalyser& an = (*it).second;
		m_current = &an;
		m_current = &((*(pr.first)).second);
		if(m_current->m_prev_counter != (packet[3]&0xf)){
			m_current->m_prev_counter = packet[3]&0xf;
			hr = m_current->PutPacket(packet, pid);
		}
	}
	return hr;
}

bool CAnalyser::UpgradeData()
{
	bool q = false;
	q = m_current->UpgradeData();
	return q;
}

HRESULT CAnalyser::DoDataExchangeDB()
{
	HRESULT hr = S_FALSE;
	return hr;
}

void CAnalyser::Free()
{
	std::map<int, CTSAnalyser>::iterator it;
	for(it = m_TsAnalyser.begin(); it != m_TsAnalyser.end(); it++){
		CTSAnalyser& ts = (*it).second;
		ts.Free();
	}
}
