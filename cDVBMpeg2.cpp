// cDVBMpeg2.cpp : Implementation of CDVBMpeg2

#include "stdafx.h"
#include <fstream>
#include <math.h>
#include <set>
using namespace ATL;
using namespace tal;
using namespace std;
#include <devctlpp.h>
#include <sessionpp.h>
#include <ByteStreampp.h>
#include <TalObjStSaverBase.h>
#include <TalXmlDom.h>
#include <Packs.h>

//#include "..\include\DVBMpeg2UI.h"

//#include "Mpeg2.h"
//#include "Structs.h"
#include "Remains.h"
//#include "IpPackets.h"
#include "..\include\iDVBMpeg2.h"
#include "Section.h"
#include "TSAnalyser.h"
#include "cDVBMpeg2.h"
/****************************************************************************************/
/**/
/****************************************************************************************/
CPacketHandling::CPacketHandling(CDVBMpeg2* pFilter)
:m_pFilter(pFilter)
{
}

CPacketHandling::~CPacketHandling(void)
{
	m_actual.clear();
	m_available.clear();
	m_KeysForDemUnknown.clear();
	m_KeysForIP.clear();
	m_KeysForTV.clear();
	m_KeysForUnknown.clear();
}

short CPacketHandling::GetPid(UCHAR* pack)
{
	short pid = (pack[1] & 0x1f) << 8;
	pid |= pack[2];
	

	/**/
	std::map< short , int >::iterator it = m_pFilter->m_PIDcount.find(pid);
	if(it != m_pFilter->m_PIDcount.end())
		(*it).second++;
	else
		m_pFilter->m_PIDcount.insert(make_pair(pid, 0));
	/**/
	
	return pid;
}

bool CPacketHandling::GetOutputOrder(int InputChannel, short pid, int& outorder, BSM_HOSTCONTROLBLOCK* phcb)
{
	bool q = false;
	PAIR key(InputChannel, pid);
	std::map< PAIR , word32 , Less >::iterator _act;
	_act = m_actual.find(key);
	if(_act == m_actual.end()){
		std::set< PAIR , Less >::iterator _avail;
		_avail = m_available.find(key);
		if(_avail != m_available.end()){
			int or = -1;
			if(m_pFilter)
				m_pFilter->m_pBsFilterSite->get_NewChannelOrder(&or);
			if(or != -1){
				outorder = or;
				m_actual.insert(make_pair(key, outorder));
				phcb->m_ActionType = BSM_AT_SLAVESEQBEGIN;
				q = true;
				CByteStreamHostThunkChannelData* pt = m_pFilter->CreateThunkChannel(outorder, InputChannel, m_pFilter->pHost);
			}
		}
	}
	else{
		phcb->m_ActionType = BSM_AT_SLAVESEQCONTINUE;
		outorder = (*_act).second;
		q = true;
	}
	return q;
}

HRESULT CPacketHandling::CloseOutputChannels(int InputChannel, BSM_HOSTCONTROLBLOCK* phcb)
{
	//m_pFilter->m_IP<<std::endl;
	//m_pFilter->m_TV<<std::endl;
	//m_pFilter->m_UN<<std::endl;
	//m_pFilter->m_DN<<std::endl;
	HRESULT hr = S_OK;
	HRESULT res = S_OK;
	phcb->m_ActionType = BSM_AT_SLAVESEQEND;
	PAIR p;
	std::map< PAIR, word32, Less>::iterator it;
	it = m_actual.begin();
	int i = m_actual.size();
	while(it != m_actual.end()){
		p = (*it).first;
		word32 w = (*it).second;
		if(p.InputChannel == InputChannel){
			std::string str;
			if(phcb->m_ActionType == BSM_AT_SLAVESEQCONTINUE)
				str = "continue";
			if(phcb->m_ActionType == BSM_AT_SLAVESEQBEGIN)
				str = "begin";
			if(phcb->m_ActionType == BSM_AT_SLAVESEQEND)
				str = "end";
			int g = (*it).second;

			if(OnIPOut(p.InputChannel, p.Pid)){
				if(m_pFilter->m_outers[IP]){
					hr = m_pFilter->m_outers[IP]->ByteStreamPut((*it).second, NULL, 0, phcb);
					m_pFilter->CloseThunkChannel((*it).second);
					m_pFilter->m_fclose<<"  ( IP , "<<g<<"  ) "<<str.c_str()<<std::endl;
					m_pFilter->m_IP<<g<<str.c_str()<<std::endl;
					m_pFilter->m_foutput<<"end"<<g<<std::endl;
				}
				if(FAILED(hr))
					res = hr;
			}
			if(OnTVOut(p.InputChannel, p.Pid)){
				//закрыть выходной канал (*it).second у выхода TV
				if(m_pFilter->m_outers[TV]){
					hr = m_pFilter->m_outers[TV]->ByteStreamPut((*it).second, NULL, 0, phcb);
					m_pFilter->CloseThunkChannel(((*it).second));
					m_pFilter->m_fclose<<"  ( TV , "<<g<<"  ) "<<str.c_str()<<std::endl;
					m_pFilter->m_TV<<g<<str.c_str()<<std::endl;
					m_pFilter->m_foutput<<"end"<<g<<std::endl;
				}
				if(FAILED(hr))
					res = hr;
			}
			if(OnUnknownOut(p.InputChannel, p.Pid)){
				//закрыть выходной канал (*it).second у выхода неизвестных
				if(m_pFilter->m_outers[UNKNOWN]){
					hr = m_pFilter->m_outers[UNKNOWN]->ByteStreamPut((*it).second, NULL, 0, phcb);
					m_pFilter->CloseThunkChannel((*it).second);
					m_pFilter->m_fclose<<"  ( UNKNOWN , "<<g<<"  ) "<<str.c_str()<<std::endl;
					m_pFilter->m_UN<<g<<str.c_str()<<std::endl;
					m_pFilter->m_foutput<<"end"<<g<<std::endl;
				}
				if(FAILED(hr))
					res = hr;
			}
			if(OnDemUnknownOut(p.InputChannel, p.Pid)){
				//закрыть выходной канал (*it).second у выхода демультиплексированных неизвестных
				if(m_pFilter->m_outers[DEM_UNKNOWN]){
					hr = m_pFilter->m_outers[DEM_UNKNOWN]->ByteStreamPut((*it).second, NULL, 0, phcb);
					m_pFilter->CloseThunkChannel((*it).second);
					m_pFilter->m_fclose<<"  ( DEM_UNKNOWN , "<<g<<"  ) "<<str.c_str()<<std::endl;
					m_pFilter->m_DN<<g<<str.c_str()<<std::endl;
					m_pFilter->m_foutput<<"end"<<g<<std::endl;
				}
				if(FAILED(hr))
					res = hr;
			}
			m_actual.erase(it);
			it = m_actual.begin();
			continue;
		}
		it++;
	}
	ClearCounters(InputChannel);
	//m_pFilter->m_fclose<<std::endl;
	return res;
}
void CPacketHandling::ClearCounters(int InChannel)
{
	std::map< PAIR, byte, Less >::iterator it;
	it = m_continuity_counters.begin();
	while(it != m_continuity_counters.end()){
		PAIR p = (*it).first;
		if(((*it).first).InputChannel == InChannel){
			m_continuity_counters.erase(it);
			it = m_continuity_counters.begin();
			continue;
		}
		it++;
	}
	return;
}
bool CPacketHandling::OnIPOut(int InputChannel, short pid)
{
	bool q = false;
	PAIR key;
	key.InputChannel = InputChannel;
	key.Pid = pid;
	std::set< PAIR , Less >::const_iterator it;
	it = m_KeysForIP.find(key);
	if(it != m_KeysForIP.end())
		q = true;
	return q;
}

bool CPacketHandling::OnTVOut(int InputChannel, short pid)
{
	bool q = false;
	PAIR key;
	key.InputChannel = InputChannel;
	key.Pid = pid;
	std::set< PAIR , Less>::const_iterator it;
	it = m_KeysForTV.find(key);
	if(it != m_KeysForTV.end())
		q = true;
	return q;
}

bool CPacketHandling::OnUnknownOut(int InputChannel, short pid)
{
	bool q = false;
	PAIR key;
	key.InputChannel = InputChannel;
	key.Pid = pid;
	std::set< PAIR , Less>::const_iterator it;
	it = m_KeysForUnknown.find(key);
	if(it != m_KeysForUnknown.end())
		q = true;
	return q;
}

bool CPacketHandling::OnDemUnknownOut(int InputChannel, short pid)
{
	bool q = false;
	PAIR key;
	key.InputChannel = InputChannel;
	key.Pid = pid;
	std::set< PAIR , Less>::const_iterator it;
	it = m_KeysForDemUnknown.find(key);
	if(it != m_KeysForDemUnknown.end())
		q = true;
	return q;
}

bool CPacketHandling::InitFromXMLFile()
{
	bool q = false;

	PAIR k;
	k.InputChannel = 0;
	k.Pid = 0x1001;
	m_available.insert(k);
	m_KeysForIP.insert(k);
	m_KeysForTV.insert(k);

	k.Pid = 0x1fff;
	m_available.insert(k);
	m_KeysForDemUnknown.insert(k);

	k.Pid = 0x1002;
	m_available.insert(k);
	m_KeysForUnknown.insert(k);

	k.Pid = 0x1003;
	m_available.insert(k);
	m_KeysForUnknown.insert(k);

	k.Pid = 0x1004;
	m_available.insert(k);
	m_KeysForUnknown.insert(k);

	k.Pid = 0x1005;
	m_available.insert(k);
	m_KeysForUnknown.insert(k);

	k.Pid = 0x1006;
	m_available.insert(k);
	m_KeysForUnknown.insert(k);

	k.Pid = 0x1007;
	m_available.insert(k);
	m_KeysForUnknown.insert(k);

	k.Pid = 0x1008;
	m_available.insert(k);
	m_KeysForIP.insert(k);

	k.Pid = 0x1009;
	m_available.insert(k);m_KeysForUnknown.insert(k);

	k.Pid = 0x1041;
	m_available.insert(k);
	m_KeysForUnknown.insert(k);

	k.Pid = 0x1012;
	m_available.insert(k);
	m_KeysForUnknown.insert(k);

	k.Pid = 0x1013;
	m_available.insert(k);
	m_KeysForUnknown.insert(k);

	k.Pid = 0x1014;
	m_available.insert(k);
	m_KeysForUnknown.insert(k);

	k.Pid = 0x1015;
	m_available.insert(k);
	m_KeysForUnknown.insert(k);

	k.Pid = 0x1016;
	m_available.insert(k);
	m_KeysForUnknown.insert(k);

	k.Pid = 0x1017;
	m_available.insert(k);
	m_KeysForUnknown.insert(k);

	k.Pid = 0x1018;
	m_available.insert(k);
	m_KeysForIP.insert(k);

	k.Pid = 0x1019;
	m_available.insert(k);
	m_KeysForUnknown.insert(k);

	k.Pid = 0x1011;
	m_available.insert(k);
	m_KeysForUnknown.insert(k);

	k.Pid = 0x1010;
	m_available.insert(k);
	m_KeysForUnknown.insert(k);

	k.InputChannel = 1;
	k.Pid = 0x1001;
	m_available.insert(k);
	m_KeysForIP.insert(k);
	
	k.Pid = 0x1019;
	m_available.insert(k);
	m_KeysForIP.insert(k);

	k.Pid = 0x1011;
	m_available.insert(k);

	k.Pid = 0x1010;
	m_available.insert(k);
	m_KeysForUnknown.insert(k);

	/*

	static const char* filename = "PidEdit.xml";
	tal::TXmlDocument document(true);
	document.Open(filename);
	tal::TObjectSaver saver;
	int InLogicalChannel = -1;
	saver.loadvar(InLogicalChannel, document.RootNode, "InLogicalChannel");
	document.Close();

	/**/
	
	return q;
}
bool CPacketHandling::ProcessPacket(PAIR k, const UCHAR* packet)
{
	if(k.Pid == 0x10)
		int i = 0x10;
	bool res = true;
	byte b = packet[3]&0xf;
	std::pair< std::map< PAIR, byte , Less>::iterator , bool > pr;
	pr.first = m_continuity_counters.find(k);
	if(pr.first == m_continuity_counters.end()){
		pr  = m_continuity_counters.insert(make_pair(k,b));
	}
	else{
		byte prev = (*(pr.first)).second;
		if(b == prev){
			if(k.Pid != 0x1fff)
				bool q = true;
			UCHAR w = packet[3]&0x30;
			if((w == 0x30) || (w == 0x10))//дублирующий пакет
				res = false;
		}
		else
			(*(pr.first)).second = b;
	}
	return res;
}
/****************************************************************************************/
/*	CDVBMpeg2		*/
/****************************************************************************************/


CDVBMpeg2::CDVBMpeg2()
:m_PackHandler(this)
{
	m_PutCount = 0;
	opened_flag = false;
	m_printed = false;
	m_outers = NULL;
	for(int i = 0; i < 12; i++)
		m_separator[i] = 0xff;
	for(int i = 0; i < MAX_BUF_LENGHT; i++)
		m_pBuf[i] = 0;
	smart_ip = TRUE;
	pack_headers = TRUE;
	std::string str = "Table.txt";
	m_analyse = true;
	//m_ConfFileName[0] ='T';
	//m_ConfFileName[1] ='a';
	//m_ConfFileName[2] ='b';
	//m_ConfFileName[3] ='l';
	//m_ConfFileName[4] ='e';
	//m_ConfFileName[5] ='.';
	//m_ConfFileName[6] ='t';
	//m_ConfFileName[7] ='x';
	//m_ConfFileName[8] ='t';
	//m_ConfFileName[9] =' ';	
	//m_ConfFileName[10] =' ';	
	//m_ConfFileName[11] =' ';	
}

HRESULT CDVBMpeg2::FinalConstruct()
{
	opened_flag = false;
	HRESULT hr = CoCreateFreeThreadedMarshaler(GetControllingUnknown(), &m_pFreeMarshaler.p);
	m_decount = 0;
	m_outers = new CComQIPtr<IByteStream>[4];
	m_statfile.open("Statistic.log", std::ios_base::out);
	m_NIT.open("NIT.txt", std::ios_base::out);
	m_PAT.open("PAT.txt", std::ios_base::out);
	m_PMT.open("PMT.txt", std::ios_base::out);
	m_CAT.open("CAT.txt", std::ios_base::out);
	//m_foutput.open("OutputOrder&Puts&Outs.log", std::ios_base::out);
	//m_fclose.open("СписокЗакрытых.log", std::ios_base::out);
	//m_fclo.open("Закрываемые.log", std::ios_base::out);

	//m_IP.open("_IP_out.log", std::ios_base::out);
	//m_TV.open("_TV_out.log", std::ios_base::out);
	//m_UN.open("_UNKNOWN_out.log", std::ios_base::out);
	//m_DN.open("_DEM_UNKNOWN_out.log", std::ios_base::out);

	//m_1012.open("0_0_1003_out.log", std::ios_base::out);

	//m_1003.open("0_0_1003_out.log", std::ios_base::out);
	return S_OK;
}

void CDVBMpeg2::FinalRelease() 
{
	if (m_outers) 
		delete[] m_outers;
	m_pFreeMarshaler.Release();
}

/*
HRESULT CDVBMpeg2::ByteStreamOpen(CInputThunk* pThunk)
HRESULT CDVBMpeg2::ByteStreamPut(CInputThunk* pThunk, int chan, word8* pbuf, int ndim)
HRESULT CDVBMpeg2::ByteStreamClose(CInputThunk* pThunk)
/**//*
HRESULT CDVBMpeg2::ByteStreamOpen(CInputThunk* pThunk)
{
	return S_OK;
}

HRESULT CDVBMpeg2::ByteStreamPut(CInputThunk* pThunk, int chan, word8* pbuf, int ndim)
{
	if(m_outers){
		BSM_HOSTCONTROLBLOCK hcb;
		hcb.m_ActionType = BSM_AT_MAINSEQCONTINUE;
		for(int i = 0; i < 2; i++)
			m_outers->p->ByteStreamPut( chan, pbuf, ndim, &hcb);
	}
	return S_OK;
}

HRESULT CDVBMpeg2::ByteStreamClose(CInputThunk* pThunk)
{
	return S_OK;
}


/**//*
//До начала обработки необходимо проинициализировать структуру конфигурации выходов фильтра
//структура инициализируется из файла
STDMETHODIMP CDVBMpeg2::ByteStreamOpen(const BSM_HOSTRESOURCES* phr, IByteStreamHost* pi, BSM_HOSTCONTROLBLOCK* phcb, int n)
{
	m_debug = 0;
	HRESULT hr = S_OK;
	HRESULT hrr = S_OK;
	if(m_PIDcount.size()>0)
		m_PIDcount.clear();
	std::map<int, CComQIPtr<IByteStream> >::iterator it;
  //for(it = m_outers.begin(); it != m_outers.end(); it++){
	//	hr = (*it).second->ByteStreamOpen(phr, pi, phcb, n);
	//	if(SUCCEEDED(hr)){}
	// else hrr = E_FAIL;
 //	}

	for(int i = 0; i < 4; i++){
		hr  = m_outers[i]->ByteStreamOpen(phr, pi, phcb, n);
		if(FAILED(hr))
			return hr;
	}
	return hr;
}

/**//*
STDMETHODIMP CDVBMpeg2::ByteStreamPut(int chan, UCHAR* pbuf, int ndim, BSM_HOSTCONTROLBLOCK* phcb)
{
	int v = TLogicalChannelMap<word32, Logical_Channel_Handler>::m_map.size();
	HRESULT hr = S_OK;
	LOGICALCHANNELMAP::iterator it;
	LOGICALCHANNELMAP::iterator b = begin();
	LOGICALCHANNELMAP::iterator e = end();
	LOGICALCHANNELMAP::iterator p;
	p = TLogicalChannelMap<word32, Logical_Channel_Handler>::m_map.find(chan);
	if( (pbuf == NULL) || (ndim == 0)){	//закрыть все выходные логические каналы, порождаемые данным входным логическим каналом
		if(p != e)
			EraseOrder(p);
	}
	else{
		//обработка блока данных
		
		int pos = 0;
		short PID = 0;
		int order = 0;
		std::map< short , word32 >::iterator _order;
		std::multimap< short , OUT_DESCRIPTION>::iterator mt;
		std::pair < OUT_ITER, OUT_ITER > par;

		if(p == e){//создание и инициализация (списка выходов) обработчика нового входного логического канала
			Logical_Channel_Handler h;	//данные обработчика этого л.к.
			std::multimap<int , OUT_DEFINITION>::iterator config;
			std::pair<std::multimap<int , OUT_DEFINITION>::iterator, std::multimap<int , OUT_DEFINITION>::iterator> pr;
			pr = m_outers_configuration.equal_range(PID);
			if(pr.first != pr.second){
				for(config = pr.first; config != pr.second; config++){
					OUT_DEFINITION& def = (*config).second;
					std::map<short , std::string >::iterator y;
					for(y = def.m_pids.begin(); y != def.m_pids.end(); y++){
						short b = (*y).first;
						h.m_outers.insert(make_pair(b, def.m_type));
					}
				}
				std::pair< LOGICALCHANNELMAP::iterator , bool> t;
				t = TLogicalChannelMap<word32, Logical_Channel_Handler>::m_map.insert(make_pair(chan, h));
				if(t.second)
					p = t.first;
				h.m_outers;
			}
		}

		//добавить еще одно условие - провеку на существование обработчика для данного л.к.
		Logical_Channel_Handler& handler = (*p).second;
		if(handler.m_count > 0){
			memcpy(&handler.m_packege[handler.m_count], pbuf, PACKET_LENGHT - handler.m_count);
			PID = (handler.m_packege[pos + 1] & 0x1f) << 8;
			PID |= (handler.m_packege[pos+2]);
			/**//*
			std::map<int, int>::iterator tr;
			tr = m_PIDcount.find(PID);
			if(tr == m_PIDcount.end()){
				std::pair<int, int> prr;
				prr.first = PID;
				prr.second = 1;
				std::pair<std::map<int, int>::iterator, bool> pr1;
				pr1 = m_PIDcount.insert(prr);
			}
			else
				(*tr).second++;
			/**//*

			par = handler.m_outers.equal_range(PID);
			if(par.first != par.second){
				_order = handler.m_outorders.end();
				_order = handler.m_outorders.find(PID);
				if(_order == handler.m_outorders.end()){
					if (m_pBsFilterSite) m_pBsFilterSite->get_NewChannelOrder(&order);
					handler.AddOutOrder(PID, order);
				}
				for(mt = par.first; mt != par.second; mt++){
					int index = (*mt).second;
					HRESULT hr = S_OK;
					if(index == 0)
						bool q = true;
					if(m_outers[index])
						hr = m_outers[index]->ByteStreamPut(order, &pbuf[pos], PACKET_LENGHT, phcb);
					phcb->m_ActionType = BSM_AT_SLAVESEQCONTINUE;
				}
			}
			handler.m_count = 0;
			pos = PACKET_LENGHT - handler.m_count;
		}

		while(pos+PACKET_LENGHT < ndim){
			PID = (pbuf[pos+1] & 0x1f) << 8;
			PID |=pbuf[pos+2];
			/**//*
			std::map<int, int>::iterator tr;
			tr = m_PIDcount.find(PID);
			if(tr == m_PIDcount.end()){
				std::pair<int, int> prr;
				prr.first = PID;
				prr.second = 1;
				std::pair<std::map<int, int>::iterator, bool> pr1;
				pr1 = m_PIDcount.insert(prr);
			}
			else
				(*tr).second++;
			/**//*

			par = handler.m_outers.equal_range(PID);
			if(par.first != par.second){
				_order = handler.m_outorders.end();
				_order = handler.m_outorders.find(PID);
				if(_order == handler.m_outorders.end()){
					if (m_pBsFilterSite) m_pBsFilterSite->get_NewChannelOrder(&order);
					handler.AddOutOrder(PID, order);
				}		
				par = handler.m_outers.equal_range(PID);
				for(mt = par.first; mt != par.second; mt++){
					int index = (*mt).second;
					HRESULT hr = S_OK;
					if(index == 0)
						bool q = true;
					if(m_outers[index])
						hr = m_outers[index]->ByteStreamPut(order, &pbuf[pos], PACKET_LENGHT, phcb);
					phcb->m_ActionType = BSM_AT_SLAVESEQCONTINUE;
				}
			}		

			pos += PACKET_LENGHT;
		}
		if(pos != ndim - 1){
			handler.m_count = ndim - pos;
			memcpy(handler.m_packege, &pbuf[pos], handler.m_count);
		}
	}
	return hr;
}

/**/
/*
STDMETHODIMP CDVBMpeg2::ByteStreamPut(int chan, UCHAR* pbuf, int ndim, BSM_HOSTCONTROLBLOCK* phcb)
{
	HRESULT hr = S_OK;
	LOGICALCHANNELMAP::iterator it;
	if( (pbuf == NULL) || (ndim == 0)){	//закрыть все выходные логические каналы, порождаемые данным входным логическим каналом
		it = TLogicalChannelMap<ChannKey, Channel_Handler, LessForKey>::m_map.begin();
		while(it != TLogicalChannelMap<ChannKey, Channel_Handler, LessForKey>::m_map.end()){
			if(it->first.m_inorder == chan){
				EraseOrder(it);
				it = TLogicalChannelMap<ChannKey, Channel_Handler, LessForKey>::m_map.begin();
				continue;
			}
			it++;
		}
	}
	else{
		//обработка блока данных
	}
	return hr;
}
/**/
/*второй вариант
STDMETHODIMP CDVBMpeg2::ByteStreamPut(int chan, UCHAR* pbuf, int ndim, BSM_HOSTCONTROLBLOCK* phcb)
{
	HRESULT hr = S_OK;
	HRESULT hr1 = S_OK;
	HRESULT hr2 = S_OK;
	bool empty = true;
	int pos_b = 0;																							//позиция начала пакета
	std::map<int,Channel_Proc>::iterator it;
	if( (phcb->m_ActionType ==  BSM_AT_SLAVESEQBEGIN) || 
		(phcb->m_ActionType == BSM_AT_SLAVESEQEND)){
			for(int i = 0; i < 2; i++){
				hr = m_outers[i]->ByteStreamPut(chan, NULL, 0, phcb);	//старт или финиш логического канала
				if(SUCCEEDED(hr)){}
				else
					hr1 = hr;
			}
		}
	else{
		if(ndim >0){
		it = m_channel_proc.find(chan);
		if(it == m_channel_proc.end()){
			std::pair<int, Channel_Proc> p;
			p.first = chan;
			std::pair< std::map<int,Channel_Proc>::iterator, bool > pr;
			pr = m_channel_proc.insert(p);
			it = pr.first;
		}
		else{
			if((*it).second.m_defcount>0)
				empty = false;
		}
		int i = 0;
		int j = 0;
		if(empty == false){//для данного канала имеется остаток предыдущего блока
			j = (*it).second.m_defcount;
			while((j < PACKET_LENGHT)){
				(*it).second.m_buf[j] = pbuf[i];
				j++;
				i++;
			}
			hr2 = m_outers[1]->ByteStreamPut(chan, (*it).second.m_buf, PACKET_LENGHT, phcb);
			hr1 = m_outers[0]->ByteStreamPut(chan, (*it).second.m_buf, PACKET_LENGHT, phcb);
		}
		pos_b = i;
		int count = 0;
		while( i < ndim ){
			if(count == PACKET_LENGHT){
				/**//*
				int PID = 0;
				PID = ((pbuf[pos_b+1])&0x1f)<<8;
				PID |= (pbuf[pos_b+2]);
				std::map<int, int>::iterator t;
				t = m_PIDcount.find(PID);
				if(t == m_PIDcount.end()){
					std::pair<int, int> p;
					p.first = PID;
					p.second = 1;
					std::pair<std::map<int, int>::iterator, bool> pr;
					pr = m_PIDcount.insert(p);
				}
				else
					(*t).second++;
				/**//*
				hr2 = m_outers[1]->ByteStreamPut(chan, &pbuf[pos_b], PACKET_LENGHT, phcb);
				int g = 0;
				g = (pbuf[pos_b+19]<<8);
				g |= pbuf[pos_b+20];
				if((pbuf[pos_b+5] == 0x3e)&&(((pbuf[pos_b+17]&0xf0) >> 4)== 0x4)&&(g >= 20))
					hr1 = m_outers[0]->ByteStreamPut(chan, &pbuf[pos_b], PACKET_LENGHT, phcb);
				phcb->m_ActionType = BSM_AT_SLAVESEQCONTINUE;
				m_hcb = *phcb;
				count = 0;
				pos_b = i;
			}
		i++;
		count++;
		}
		if(pos_b - ndim != 0){
			(*it).second.m_defcount = ndim - pos_b;
			for(int k = 0; k < (*it).second.m_defcount ; k++)
				(*it).second.m_buf[k] = pbuf[pos_b + k];
		}
	}
		else{
			std::map<int, Channel_Proc>::iterator it;
			for(it = m_channel_proc.begin(); it != m_channel_proc.end(); it++){
				hr1 = m_outers[0]->ByteStreamPut((*it).first, &((*it).second.m_buf[0]), (*it).second.m_defcount, &m_hcb);
				hr = m_outers[1]->ByteStreamPut((*it).first, &((*it).second.m_buf[0]), (*it).second.m_defcount, &m_hcb);
			}
		}
	}
	if(SUCCEEDED(hr2))
		return hr1;
	return hr2;
		/**//*
}
/**/
/*первый вариант
STDMETHODIMP CDVBMpeg2::ByteStreamPut(int chan, UCHAR* pbuf, int ndim, BSM_HOSTCONTROLBLOCK* phcb)
{
	HRESULT hr = S_OK;
	HRESULT hr1 = S_OK;
	HRESULT hr2 = S_OK;
	std::map<int, CComQIPtr<IByteStream> >::iterator it;
	if( (phcb->m_ActionType ==  BSM_AT_SLAVESEQBEGIN) || (phcb->m_ActionType == BSM_AT_SLAVESEQEND)){
//		for(it = m_outers.begin(); it != m_outers.end(); it++)
//			(*it).second->ByteStreamPut(chan, NULL, 0, phcb);	//старт или финиш логического канала
		for(int i = 0; i < 2; i++){
			hr = m_outers[i]->ByteStreamPut(chan, NULL, 0, phcb);	//старт или финиш логического канала
			if(SUCCEEDED(hr)){}
			else
				hr1 = hr;
		}
	}
	else{
		std::map<int, Channel_Proc>::iterator it;
		it = m_channel_proc.find(chan);
		if(it == m_channel_proc.end()){
			std::pair<int, Channel_Proc> par;
			par.first = chan;
			m_channel_proc.insert(par);
		}
		it = m_channel_proc.find(chan);
		int dim = ndim;
		int i = 0;
		while(i<ndim){
			if((*it).second.m_defcount == PACKET_LENGHT){
				//передать на обработку
				double x, y, n;
				x = (double)(m_debug)/10;
				y = modf( x, &n );    //y - дробная часть
				int PID = 0;
				PID = (((*it).second.m_buf[1])&0x1f)<<8;
				PID |= ((*it).second.m_buf[2]);
				//m_statfile<<hexw(1)<<" "<<PID<<" ";
				std::map<int, int>::iterator t;
				t = m_PIDcount.find(PID);
				if(t == m_PIDcount.end()){
					std::pair<int, int> p;
					p.first = PID;
					p.second = 0;
					m_PIDcount.insert(p);
				}
				else
					(*t).second++;
				if( y == 0){
					hr2 = m_outers[1]->ByteStreamPut(chan, &((*it).second.m_buf[0]), PACKET_LENGHT, phcb);
					//m_statfile<<std::endl;
				}
				m_debug++;
				(*it).second.m_defcount = 0;
			}
			int defcount = (*it).second.m_defcount;
			(*it).second.m_buf[(*it).second.m_defcount] = pbuf[i];
			(*it).second.m_defcount++;
			i++;
		}
	}
	if(SUCCEEDED(hr1))
		return hr2;
	return hr1;
}
/**//*
STDMETHODIMP CDVBMpeg2::ByteStreamClose()
{
	HRESULT hr  = S_OK;
	HRESULT hrr = S_OK;
	std::map<int, int>::iterator it = m_PIDcount.begin();
	double x,y,n;
	x = 0;
	while(it != m_PIDcount.end()){
		//y = modf(x/3, &n);
		//if((y == 0.0)&&(x>3))
			//m_statfile<<std::endl;
		m_statfile<<hexw(1)<<x<<"       PID="<<(*it).first<<" "<<decw(1)<<(*it).second<<" times"<<std::endl;
		it++;
		x++;
	}

	LOGICALCHANNELMAP::iterator p;
	for(p = TLogicalChannelMap<word32, Logical_Channel_Handler>::m_map.begin(); p != TLogicalChannelMap<word32, Logical_Channel_Handler>::m_map.end(); p++){
		Logical_Channel_Handler& h = (*p).second;
		OnOrderClose(0, &h);
	}
	//p = TLogicalChannelMap<word32, Logical_Channel_Handler>::m_map.find(chan);
	for(int i = 0; i < 4; i++){
		if(m_outers[i])
			m_outers[i]->ByteStreamClose();
	}
	if(SUCCEEDED(hr))
		return hrr;
	return hr;
}
/**/
STDMETHODIMP CDVBMpeg2::InitNew()
{
	//TestInitConfiguration();
	return S_OK;
}


//Проинициализировать структуру конфигурации фильтров
STDMETHODIMP CDVBMpeg2::InitInstance()
{
	TestInitConfiguration();
	HRESULT hr = S_OK;
  if (DCF_CHECKINITED) return E_FAIL;
  IComponentControlImpl<CDVBMpeg2>::InitInstance(hr);
	return hr;
}

//Проинициализировать структуру конфигурации выходов фильтра
STDMETHODIMP CDVBMpeg2::EnableInstance()
{
	TestInitConfiguration();
	if (!DCF_CHECKINITED || DCF_CHECKENABLED) return E_FAIL;
	IComponentControlImpl<CDVBMpeg2>::EnableInstance(S_OK);
	return 0;
}

STDMETHODIMP CDVBMpeg2::DisableInstance()
{
//	m_handlers.clear();
/**/
	m_analyser.Free();
	LOGICALCHANNELMAP::iterator it = begin();
	while(it != end()){
		//OnOrderClose(&((*it).first), &((*it).second));
		ChannKey k = (*it).first;
		m_PackHandler.ClearCounters(k.m_inorder);
		Channel_Handler& h = (*it).second;
		std::map<short, CSection>::iterator t ;
		for(t = h.m_sec.begin(); t != h.m_sec.end(); t++)
			(*t).second.Free();
		EraseOrder(it);
		it = begin();
	}
	m_remains.Free();
	
	/**/
	if(opened_flag){
		if(m_outers[0])
			m_outers[0]->ByteStreamClose();
		if(m_outers[1])
			m_outers[1]->ByteStreamClose();
		if(m_outers[2])
			m_outers[2]->ByteStreamClose();
		if(m_outers[3])
			m_outers[3]->ByteStreamClose();
		opened_flag = false;
	}
	if (!DCF_CHECKENABLED) return E_FAIL;
	IComponentControlImpl<CDVBMpeg2>::DisableInstance();
	return S_OK;
}

HRESULT CDVBMpeg2::Save(IStorage* ps, BOOL SameAsLoad)
{
	USES_CONVERSION;
	HRESULT hr = S_OK;
	CComPtr<IStream> pStream;
	CComPtr<IStorage> pStorage = ps;
	DVBMPEG2_PARAMS* pr;
	pr = this;
	hr = pStorage->CreateStream(L"DMPEG2_PARAMS1",STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,0, 0,&pStream);
	if (SUCCEEDED(hr)){
		ULONG cWritten = 0;
		hr = pStream->Write(pr, sizeof(DVBMPEG2_PARAMS), &cWritten);
		pStream = NULL;
	}
	pStorage = NULL;
	return S_OK;
}

HRESULT CDVBMpeg2::Load(IStorage* ps)
{
	USES_CONVERSION;
	HRESULT hr = S_OK;
	CComPtr<IStream> pStream;
	CComPtr<IStorage> pStorage = ps;
	DVBMPEG2_PARAMS* p = this;
	hr = pStorage->OpenStream(L"DMPEG2_PARAMS1",0,STGM_READWRITE | STGM_SHARE_EXCLUSIVE,0,&pStream);
	if (SUCCEEDED(hr)) {
    ULONG cRead = 0;
		hr = pStream->Read(p, sizeof(DVBMPEG2_PARAMS), &cRead);
		ULONG r = cRead;
	}
	InitNew();
	pStream = NULL;
	pStorage = NULL;
	return S_OK;
}

STDMETHODIMP CDVBMpeg2::put_Site(IUnknown* punkn)
{
	CComPtr<IUnknown> p = punkn;
	if (p)
		m_pBsFilterSite = p;
	return 0;
}

STDMETHODIMP CDVBMpeg2::put_Bind(int outer, IUnknown* punkn)
{
	CComPtr<IUnknown> p = punkn;
	if( (outer < 4)&& (p) ){
		m_outers[outer] = p;
	//std::pair< int, CComQIPtr<IByteStream> > par;
	//par.first = outer;
	//par.second = pbs;
	//m_outers.insert(par);
	}
	return 0;
}

STDMETHODIMP CDVBMpeg2::get_Bind(int outer, IUnknown** punkn)
{
	m_decount++;
	//if(m_decount == 9)
	//{
	//	bool q = true;
	//}
	//CComPtr<IUnknown> p = NULL;
	//int s = m_outers.size();
	//std::map<int, CComQIPtr<IByteStream> >::iterator the;
	//the = m_outers.find(outer);
	//if(the != m_outers.end()){
	//	p = (*the).second;
	//	*punkn = p;
	//}
	CComPtr<IUnknown> p = m_outers[outer];
	if((outer<4)&&(p)){
		p.CopyTo(punkn);
	}
	return 0;
}

STDMETHODIMP CDVBMpeg2::put_Mode(BSM_FILTERMODE mode)
{
	m_Mode = mode;
	return 0;
}
STDMETHODIMP CDVBMpeg2::get_Mode(BSM_FILTERMODE* mode)
{
	*mode = m_Mode;
	return 0;
}

STDMETHODIMP CDVBMpeg2::get_OuterCount(int* outer_count)
{
	*outer_count = 4;
	return 0;
}

STDMETHODIMP CDVBMpeg2::Flush()
{
	//m_hcb.m_ActionType = BSM_AT_SLAVESEQEND;
	HRESULT hr = S_OK;
	FlushMap();
//	if(m_channel_proc.size() > 0){
//		m_channel_proc.clear();
//	}
	return hr;
}
/**/
STDMETHODIMP CDVBMpeg2::get_Statistics([out] BSTR *str)
{
	std::ostringstream os;
	os<<m_PutCount;
	std::string s = os.str().c_str();
	/**/
	//BSTR bs;
	//bs = SysAllocString(L"");
	CComBSTR bstrString(s.data());
	bstrString.CopyTo(str);
	/**/
	return S_OK;
}

STDMETHODIMP CDVBMpeg2::get_StatisticsDescription([out] BSTR *str)
{
	std::string s = "Количество пришедших методов ByteStreamPut";
	CComBSTR bstrString(s.data());
	bstrString.CopyTo(str);
	return S_OK;
}

STDMETHODIMP CDVBMpeg2::ResetStatistics()
{
	return 0;
}

STDMETHODIMP CDVBMpeg2::get_OuterDescription (int outer, BSTR *description)
{
	USES_CONVERSION;
	switch(outer){
		case 0:
			*description = ::SysAllocString(L"Выход№0 - IP дейтаграммы");
			break;
		case 1:
			*description = ::SysAllocString(L"Выход№1 - неизвестный MPEG2");
			break;
		case 2:
			*description = ::SysAllocString(L"Выход№2 - дем. неизвестный MPEG2");
			break;
		case 3:
			*description = ::SysAllocString(L"Выход№3 - TV");
			break;
	}
	/*
	if(outer == 0)
		*description = ::SysAllocString(L"Выход1 - Для IP дейтаграмм");
	if(outer == 1)
		*description = ::SysAllocString(L"Выход2 - Для не IP дейтаграмм");
		/**/
	return 0;
}
/**/

//void CDVBMpeg2::OnOrderClose(const int* pkey, Logical_Channel_Handler* pdata)
//{
//	BSM_HOSTCONTROLBLOCK phcb;
//	phcb.m_ActionType = BSM_AT_MAINSEQEND;
//	std::map < short, word32 >::iterator it;
//	OUT_ITER mt;
//	std::pair< OUT_ITER, OUT_ITER > p;
//	for(it = pdata->m_outorders.begin(); it != pdata->m_outorders.end(); it++){
//		int chan = (*it).second;
//		short pid = (*it).first;
//		p = pdata->m_outers.equal_range(pid);
//		for(mt = p.first; mt != p.second; mt++){
//			int index = (*mt).second;
//			if(m_outers[index]){
//				if(index == 0)
//					bool q = true;
//				m_outers[index]->ByteStreamPut(chan, NULL, 0, &phcb);
//			}
//		}
//	}
//}

void	CDVBMpeg2::TestInitConfiguration()
{

	/**/
	//DVBMPEG2_PARAMS* pr = this;
	//std::string str = "";
	//str = pr->m_ConfFileName;
	//TruncateString(str);

	//чтение таблицы задания на отбор из файла
	std::string file = "c:\\DVB\\MPEG2\\TABLE\\";
	//file += str;
	file = "c:\\DVB\\MPEG2\\TABLE\\Table.txt";
	/**/
	if(file.size() > 0){
		std::fstream f;
		f.open(file.c_str(), std::ios_base::in);
		std::string str = "";
		int z = 0;
		ChannKey ky;
		ky.m_inorder = 0;
		while (!f.eof()) {
			str = "";
			f>>str;
			if(str.size() > 0){
				z = atox<int>(str.data(), 16);
				ky.m_PID = z;
				if(!f.eof()){
					str = "";
					f>>str;
					if(str.size() > 0){
						z = atox<int>(str.data(), 16);
						if ((z >= 0)&&(z <= 3)){
							ky.m_outnumber = z;
							m_availablekeys.insert(ky);
						}
					}//size2 > 0
				}//!eof
			}//size1 > 0
		}//while
	}

/**//*
	std::pair<int, OUT_DEFINITION> p;
	m_outers_configuration.clear();
	OUT_DEFINITION& data = p.second;

	
/*
	//конфигурация выходов для логического канала номер 1
	p.first = 0;
	data.m_type = IP;
	data.m_pids.clear();
	data.m_pids.insert(make_pair(0x1001, ""));
	data.m_pids.insert(make_pair(0x1002, ""));
	data.m_pids.insert(make_pair(0x1003, ""));
	data.m_pids.insert(make_pair(0x1004, ""));
	data.m_pids.insert(make_pair(0x1005, ""));
	data.m_pids.insert(make_pair(0x1006, ""));
	data.m_pids.insert(make_pair(0x1007, ""));
	data.m_pids.insert(make_pair(0x1008, ""));
	data.m_pids.insert(make_pair(0x1009, ""));
	data.m_pids.insert(make_pair(0x1010, ""));
	data.m_pids.insert(make_pair(0x1041, ""));
	m_outers_configuration.insert(p);

	/*LAST
	ChannKey k;
	k.m_inorder = 0;
	k.m_outnumber = IP;
	k.m_PID = 0x1001;
	m_availablekeys.insert(k);
	k.m_PID = 0x1002;
	m_availablekeys.insert(k);
	k.m_PID = 0x1003;
	m_availablekeys.insert(k);
	k.m_PID = 0x1004;
	m_availablekeys.insert(k);
	k.m_PID = 0x1005;
	m_availablekeys.insert(k);
	k.m_PID = 0x1006;
	m_availablekeys.insert(k);
	k.m_PID = 0x1007;
	m_availablekeys.insert(k);
	k.m_PID = 0x1008;
	m_availablekeys.insert(k);
	k.m_PID = 0x1009;
	m_availablekeys.insert(k);
	k.m_PID = 0x1010;
	m_availablekeys.insert(k);
	k.m_PID = 0x1011;
	m_availablekeys.insert(k);
	k.m_PID = 0x1012;
	m_availablekeys.insert(k);
	k.m_PID = 0x1013;
	m_availablekeys.insert(k);
	/**//*

	data.m_type = TV;
	data.m_pids.clear();
	data.m_pids.insert(make_pair(0x0, ""));
	data.m_pids.insert(make_pair(0x0001, ""));
	data.m_pids.insert(make_pair(0x0002, ""));
	data.m_pids.insert(make_pair(0x10, ""));
	data.m_pids.insert(make_pair(0x1ffe, ""));
	m_outers_configuration.insert(p);

	/*LAST
	k.m_inorder = 0;
	k.m_outnumber = TV;
	k.m_PID = 0x0bc3;
	m_availablekeys.insert(k);
	k.m_PID = 0x0bc2;
	m_availablekeys.insert(k);
	k.m_PID = 0x07d4;
	m_availablekeys.insert(k);
	k.m_PID = 0x07da;
	m_availablekeys.insert(k);
	k.m_PID = 0x0a2a;
	m_availablekeys.insert(k);
	k.m_PID = 0x0a29;
	m_availablekeys.insert(k);
	/**//*

	data.m_type = UNKNOWN;
	data.m_pids.clear();
	data.m_pids.insert(make_pair(0x1001, ""));
	data.m_pids.insert(make_pair(0x1002, ""));
	data.m_pids.insert(make_pair(0x1003, ""));	
	data.m_pids.insert(make_pair(0x1004, ""));	
	data.m_pids.insert(make_pair(0x1005, ""));	
	data.m_pids.insert(make_pair(0x1006, ""));	
	data.m_pids.insert(make_pair(0x1007, ""));	
	data.m_pids.insert(make_pair(0x1008, ""));
	data.m_pids.insert(make_pair(0x1009, ""));
	data.m_pids.insert(make_pair(0x1010, ""));
	data.m_pids.insert(make_pair(0x1041, ""));
	m_outers_configuration.insert(p);
	/*LAST
	k.m_outnumber = UNKNOWN;
	k.m_PID = 0x1001;
	m_availablekeys.insert(k);
	k.m_outnumber = UNKNOWN;
	k.m_PID = 0x1002;
	m_availablekeys.insert(k);
	k.m_outnumber = UNKNOWN;
	k.m_PID = 0x1003;
	m_availablekeys.insert(k);
	k.m_outnumber = UNKNOWN;
	k.m_PID = 0x1004;
	m_availablekeys.insert(k);
	k.m_outnumber = UNKNOWN;
	k.m_PID = 0x1005;
	m_availablekeys.insert(k);
	k.m_outnumber = UNKNOWN;
	k.m_PID = 0x1006;
	m_availablekeys.insert(k);
	k.m_outnumber = UNKNOWN;
	k.m_PID = 0x1007;
	m_availablekeys.insert(k);
	k.m_outnumber = UNKNOWN;
	k.m_PID = 0x1008;
	m_availablekeys.insert(k);
	k.m_outnumber = UNKNOWN;
	k.m_PID = 0x1009;
	m_availablekeys.insert(k);
	k.m_outnumber = UNKNOWN;
	k.m_PID = 0x1010;
	m_availablekeys.insert(k);
	k.m_outnumber = UNKNOWN;
	k.m_PID = 0x1041;
	m_availablekeys.insert(k);

	k.m_PID = 0x0600;
	m_availablekeys.insert(k);
	k.m_PID = 0x0200;
	m_availablekeys.insert(k);
	k.m_PID = 0x028a;
	m_availablekeys.insert(k);
	k.m_PID = 0x05ff;
	m_availablekeys.insert(k);
	k.m_PID = 0x0207;
	m_availablekeys.insert(k);
	k.m_PID = 0x02d0;
	m_availablekeys.insert(k);
	/**//*


	data.m_type = DEM_UNKNOWN;
	data.m_pids.clear();
	data.m_pids.insert(make_pair(0x1fff, ""));
	m_outers_configuration.insert(p);

	/*LAST
	k.m_inorder = 0;
	k.m_outnumber = DEM_UNKNOWN;
	k.m_PID = 0x09d9;
	m_availablekeys.insert(k);

	k.m_outnumber = DEM_UNKNOWN;
	k.m_PID = 0x09ed;
	m_availablekeys.insert(k);

  
 	k.m_outnumber = DEM_UNKNOWN;
	k.m_PID = 0x09f7;
	m_availablekeys.insert(k);

 	k.m_outnumber = DEM_UNKNOWN;
	k.m_PID = 0x0356;
	m_availablekeys.insert(k);

 	k.m_outnumber = DEM_UNKNOWN;
	k.m_PID = 0x0c53;
	m_availablekeys.insert(k);

 	k.m_outnumber = DEM_UNKNOWN;
	k.m_PID = 0x0c54;
	m_availablekeys.insert(k);

 	k.m_outnumber = DEM_UNKNOWN;
	k.m_PID = 0x0258;
	m_availablekeys.insert(k);

	k.m_outnumber = DEM_UNKNOWN;
	k.m_PID = 0x0259;
	m_availablekeys.insert(k);

	k.m_outnumber = DEM_UNKNOWN;
	k.m_PID = 0x025d;
	m_availablekeys.insert(k);

	k.m_outnumber = DEM_UNKNOWN;
	k.m_PID = 0x042f;
	m_availablekeys.insert(k);
	
	k.m_outnumber = DEM_UNKNOWN;
	k.m_PID = 0x0438;
	m_availablekeys.insert(k);
	/**/
	/**//*
	ChannKey k1;
	k1.m_inorder = 0;
	k1.m_outnumber = IP;
	k1.m_PID = 0x1001;
	m_availablekeys.insert(k1);
	k1.m_PID = 0x1002;
	m_availablekeys.insert(k1);
	k1.m_PID = 0x1003;
	m_availablekeys.insert(k1);
	k1.m_PID = 0x1004;
	m_availablekeys.insert(k1);
	k1.m_PID = 0x1005;
	m_availablekeys.insert(k1);
	k1.m_PID = 0x1006;
	m_availablekeys.insert(k1);
	k1.m_PID = 0x1007;
	m_availablekeys.insert(k1);
	k1.m_PID = 0x1008;
	m_availablekeys.insert(k1);
	k1.m_PID = 0x1009;
	m_availablekeys.insert(k1);
	k1.m_PID = 0x1010;
	m_availablekeys.insert(k1);
	k1.m_PID = 0x1011;
	m_availablekeys.insert(k1);
	k1.m_PID = 0x1012;
	m_availablekeys.insert(k1);
	k1.m_PID = 0x1013;
	m_availablekeys.insert(k1);

	k1.m_inorder = 0;
	k1.m_outnumber = TV;
	k1.m_PID = 0x0;
	m_availablekeys.insert(k1);
	k1.m_PID = 0x0001;
	m_availablekeys.insert(k1);
	k1.m_PID = 0x0002;
	m_availablekeys.insert(k1);
	k1.m_PID = 0x10;
	m_availablekeys.insert(k1);
	k1.m_PID = 0x1ffe;
	m_availablekeys.insert(k1);


	k1.m_outnumber = UNKNOWN;
	k1.m_PID = 0x1001;
	m_availablekeys.insert(k1);
	k1.m_outnumber = UNKNOWN;
	k1.m_PID = 0x1002;
	m_availablekeys.insert(k1);
	k1.m_outnumber = UNKNOWN;
	k1.m_PID = 0x1003;
	m_availablekeys.insert(k1);
	k1.m_outnumber = UNKNOWN;
	k1.m_PID = 0x1004;
	m_availablekeys.insert(k1);
	k1.m_outnumber = UNKNOWN;
	k1.m_PID = 0x1005;
	m_availablekeys.insert(k1);
	k1.m_outnumber = UNKNOWN;
	k1.m_PID = 0x1006;
	m_availablekeys.insert(k1);
	k1.m_outnumber = UNKNOWN;
	k1.m_PID = 0x1007;
	m_availablekeys.insert(k1);
	k1.m_outnumber = UNKNOWN;
	k1.m_PID = 0x1008;
	m_availablekeys.insert(k1);
	k1.m_outnumber = UNKNOWN;
	k1.m_PID = 0x1009;
	m_availablekeys.insert(k1);
	k1.m_outnumber = UNKNOWN;
	k1.m_PID = 0x1010;
	m_availablekeys.insert(k1);
	k1.m_outnumber = UNKNOWN;
	k1.m_PID = 0x1041;
	m_availablekeys.insert(k1);

	k1.m_inorder = 0;
	k1.m_outnumber = DEM_UNKNOWN;
	k1.m_PID = 0x1fff;
	m_availablekeys.insert(k1);
	/**/
	/**/
}

//STDMETHODIMP CDVBMpeg2::GetHeaderSignatures(int outchan, ULONG** ppSignature, ULONG* pnDim)
//{
//}
//
//STDMETHODIMP CDVBMpeg2::GetHeader(int outchan, ULONG Signature, BSM_HEOF* ph, BOOL* pApplyToDst)
//{
//		CByteStreamHostThunkChannelData* p = FindThunkChannel(outchan);
//  ATLASSERT(p);
//	HRESULT hr = p->m_pHost ? p->m_pHost->GetHeader(p->m_inchan, Signature, ph, pApplyToDst) : E_POINTER;
//  return hr;
//}
/**/
STDMETHODIMP CDVBMpeg2::GetHeader(int outchan, ULONG Signature, BSM_HEOF* ph, BOOL* pApplyToDst)
{
 if (Signature == UMF_HT_SRCSIGNATURE) {
	 CByteStreamHostThunkChannelData* p = FindThunkChannel(outchan);
	 if (p) {
		 //tal::wstring s("I" + xtoa(p->m_pInst->m_Priority, 16));
		 //tal::wstring s1(xtoa(p->m_sgnasm, 36));
		 tal::wstring s("I_");
		 tal::wstring s1(xtoa(p->key.m_inorder, 10));
		 tal::wstring s2(xtoa(p->Packet_ID, 16));
		 tal::wstring s3;//(xtoa(p->key.m_outnumber, 10));
		 switch(p->key.m_outnumber){
			 case 0:
				 s3 = L"IP";
				 break;
			 case 1:
				 s3 = L"UN";
				 break;
			 case 2:
				 s3 = L"DN";
				 break;
			 case 3:
				 s3 = L"TV";
				 break;
		 }
		 int order = 0;
		 if(m_pBsFilterSite)
			 m_pBsFilterSite->get_NextFileOrder(outchan, &order);
		 tal::wstring s4(xtoa(order, 16));
		 tal::wstring s5(xtoa(outchan, 16));
		 s += s3 + L"_" + s5 + L"_" + s2 + L"_" + s1 + L"_" + s4 + L".dat";
		 /*
		 size_t n = max(0, 2 - int(s1.size()));
		 if (n)
			 s1.insert(std::string::size_type(0), n, L'0');
		 s += s1;
		 int order = 0;
		 if(m_pBsFilterSite)
			 m_pBsFilterSite->get_NextFileOrder(outchan, &order);
		 s1 = xtoa(order, 16);
		 n = max(0, 8 - int(s1.size()));
		 if (n)
			 s1.insert(std::string::size_type(0), n, L'0');
		 s += s1 + L".dat";
		 /**/
		 bsm::TBSMSource h;
		 h.m_TypeOfSource = BSM_TS_UNKNOWN;
		 s.copyto(h.m_Filename, MAX_URLPATH-1);
		 ph->m_Signature = UMF_HT_SRCSIGNATURE;
		 ph->m_Type = BSM_HT_NONE;
		 ph->m_Version = UMF_HT_SRCVERSION;
		 h.CopyTo(&ph->m_pBuf, &ph->m_Size);      return S_OK;
	 } else {
		 tal::wstring s("I____X__");
		 tal::wstring s1(xtoa(outchan, 16));
		 size_t n = max(0, 8 - int(s1.size()));
		 if (n)
			 s1.insert(std::string::size_type(0), n, L'0');
		 s += s1 + L".dat";
		 bsm::TBSMSource h;
		 h.m_TypeOfSource = BSM_TS_UNKNOWN;
		 s.copyto(h.m_Filename, MAX_URLPATH);
		 ph->m_Signature = UMF_HT_SRCSIGNATURE;
		 ph->m_Type = BSM_HT_NONE;
		 ph->m_Version = UMF_HT_SRCVERSION;
		 h.CopyTo(&ph->m_pBuf, &ph->m_Size);
		 return S_OK;
	 }
 } else
	 return bsm::TByteStreamHostThunkImpl<CByteStreamHostThunkChannelData>::GetHeader(outchan, Signature, ph, pApplyToDst);
 return E_POINTER;
}

/*
////////////////////////////////////////////////////////////////////
STDMETHODIMP CDVBMpeg2::ByteStreamOpen(const BSM_HOSTRESOURCES* phr, IByteStreamHost* pi, BSM_HOSTCONTROLBLOCK* phcb, int n)
{
	m_PutCount = 0;
	m_debug = 0;
	HRESULT hr = S_OK;
	HRESULT hrr = S_OK;
	m_PackHandler.InitFromXMLFile();
	if(m_outers[IP])
		hr = m_outers[IP]->ByteStreamOpen(&m_h, this, NULL, 0);
	if(FAILED(hr))
		hrr = hr;
	if(m_outers[TV])
		m_outers[TV]->ByteStreamOpen(&m_h, this, NULL, 0);
	if(FAILED(hr))
		hrr = hr;
	if(m_outers[UNKNOWN])
		m_outers[UNKNOWN]->ByteStreamOpen(&m_h, this, NULL, 0);
	if(FAILED(hr))
		hrr = hr;
	if(m_outers[DEM_UNKNOWN])
		m_outers[DEM_UNKNOWN]->ByteStreamOpen(phr, pi, phcb, n);
	if(FAILED(hr))
		hrr = hr;
	if(FAILED(hrr))
		bool q = false;
	return hrr;
}


STDMETHODIMP CDVBMpeg2::ByteStreamPut(int chan, UCHAR* pbuf, int ndim, BSM_HOSTCONTROLBLOCK* phcb)
{
	m_PutCount++;
	HRESULT hr = S_OK;
	HRESULT res= S_OK;
	if( (pbuf == NULL) || (ndim == 0) || (phcb->m_ActionType == BSM_AT_SLAVESEQEND)){
		m_remains.FreeChannel(chan);
		res = 	m_PackHandler.CloseOutputChannels(chan, phcb);
	}
	else{
		//m_foutput<<std::endl;
		//m_IP<<std::endl;
		//m_TV<<std::endl;
		//m_UN<<std::endl;
		//m_DN<<std::endl;
		m_remains.Concatenate(chan, pbuf, ndim);
		UCHAR* pos_begin = NULL;
		while(m_remains.GetPacket(pos_begin)){
			int outputorder = -1;
			short pid = m_PackHandler.GetPid(pos_begin);
			if(m_PackHandler.GetOutputOrder(chan, pid, outputorder, phcb)){
				std::string str;
				bool make_open = false;
				if(phcb->m_ActionType == BSM_AT_SLAVESEQCONTINUE)
					str = "continue";
				if(phcb->m_ActionType == BSM_AT_SLAVESEQBEGIN){
					str = "begin";
					make_open = true;
				}
				if(phcb->m_ActionType == BSM_AT_SLAVESEQEND)
					str = "end";
				if(m_PackHandler.OnIPOut(chan, pid)){
					if(m_outers[IP]){
						hr = m_outers[IP]->ByteStreamPut(outputorder, pos_begin, PACKET_LENGHT, phcb);
						m_foutput<<"   ( IP  , "<<(int)outputorder<<" ) "<<str.c_str()<<std::endl;
						m_IP<<" "<<(int)outputorder<<" "<<str.c_str()<<std::endl;
					}
					if(FAILED(hr))
						res = hr;
				}
				if(m_PackHandler.OnTVOut(chan, pid)){
					if(m_outers[TV]){
						hr = m_outers[TV]->ByteStreamPut(outputorder, pos_begin, PACKET_LENGHT, phcb);
						//m_outers[TV]->ByteStreamPut(outputorder, m_separator, 12, phcb);
						m_foutput<<"   ( TV  , "<<(int)outputorder<<" ) "<<str.c_str()<<std::endl;
						m_TV<<" "<<(int)outputorder<<" "<<str.c_str()<<std::endl;
					}
					if(FAILED(hr))
						res = hr;
				}
				if(m_PackHandler.OnUnknownOut(chan, pid)){
					if(m_outers[UNKNOWN]){
						hr = m_outers[UNKNOWN]->ByteStreamPut(outputorder, pos_begin, PACKET_LENGHT, phcb);
						//m_outers[UNKNOWN]->ByteStreamPut(outputorder, m_separator, 12, phcb);
						m_foutput<<"   ( UNKNOWN  , "<<(int)outputorder<<" ) "<<str.c_str()<<std::endl;
						m_UN<<" "<<(int)outputorder<<" "<<str.c_str()<<std::endl;
					}
					if(FAILED(hr))
						res = hr;
				}
				if(m_PackHandler.OnDemUnknownOut(chan, pid)){
					if(m_outers[DEM_UNKNOWN]){
						hr = m_outers[DEM_UNKNOWN]->ByteStreamPut(outputorder, pos_begin, PACKET_LENGHT, phcb);
						//m_outers[DEM_UNKNOWN]->ByteStreamPut(outputorder, m_separator, 12, phcb);
						m_foutput<<"   ( DEM_UNKNOWN  , "<<(int)outputorder<<" ) "<<str.c_str()<<std::endl;
						m_DN<<" "<<(int)outputorder<<" "<<str.c_str()<<std::endl;
					}
					if(FAILED(hr))
						res = hr;
				}
			}
		}//while
		if(!m_remains.SaveRemain())
			res = E_FAIL;
	}
	if(FAILED(res))
		bool r = false;
	return res;
}

STDMETHODIMP CDVBMpeg2::ByteStreamClose()
{
 	HRESULT hr = S_OK;
	HRESULT hrr = S_OK;
	//std::map<int, BLOCK_REM>::iterator it;
	//for(it = m_remains.m_map.begin(); it != m_remains.m_map.end(); it++){
	//	int i = (*it).first;
	//	m_PackHandler.CloseOutputChannels(i);
	//}
	/*if(m_outers[IP])
		hr = m_outers[IP]->ByteStreamClose();
	if(FAILED(hr))
		hrr = hr;
	if(m_outers[TV])
		hr = m_outers[TV]->ByteStreamClose();
	if(FAILED(hr))
		hrr = hr;
	if(m_outers[UNKNOWN])
		hr = m_outers[UNKNOWN]->ByteStreamClose();
	if(FAILED(hr))
		hrr = hr;
	if(m_outers[DEM_UNKNOWN])
		hr = m_outers[DEM_UNKNOWN]->ByteStreamClose();
	if(FAILED(hr))
		hrr = hr;*/
/*
	std::map< short , int >::iterator t;
	int i = 0;
	int x = m_PIDcount.size();
	for(t = m_PIDcount.begin(); t != m_PIDcount.end(); t++){
		short pid = (*t).first;
		int c = (*t).second;
		m_statfile<<(int)i<<" PID="<<hexw(1)<<(int)pid<<decw(1)<<"     count="<<(int)c<<std::endl;
		i++;
	}
	if(FAILED(hrr))
		bool q = false;
	return hrr;
}

STDMETHODIMP CDVBMpeg2::CreateInputThunk(int outer, IByteStream** ppBs, VARIANT_BOOL* pbMultiInput)
{
  *pbMultiInput = VARIANT_TRUE;
  return CInputThunk::CreateInstance<CInputThunk>(outer, this, ppBs);
}
/**/

//HRESULT CDVBMpeg2::ByteStreamOpen(CInputThunk* pThunk)
STDMETHODIMP CDVBMpeg2::ByteStreamOpen(const BSM_HOSTRESOURCES* phr, IByteStreamHost* pi, BSM_HOSTCONTROLBLOCK* phcb, int n)
{
	m_PutCount = 0;
	m_debug = 0;
	HRESULT hr = S_OK;
	HRESULT hrr = S_OK;
	/**/
	//m_PackHandler.InitFromXMLFile();
	if(!opened_flag){
		opened_flag = true;
		if(m_outers[IP])
			hr = m_outers[IP]->ByteStreamOpen(&m_h, this, NULL, 0);
		if(FAILED(hr))
			hrr = hr;
		if(m_outers[TV])
			m_outers[TV]->ByteStreamOpen(&m_h, this, NULL, 0);
		if(FAILED(hr))
			hrr = hr;
		if(m_outers[UNKNOWN])
			m_outers[UNKNOWN]->ByteStreamOpen(&m_h, this, NULL, 0);
		if(FAILED(hr))
			hrr = hr;
		if(m_outers[DEM_UNKNOWN])
			m_outers[DEM_UNKNOWN]->ByteStreamOpen(&m_h, this, NULL, 0);
		if(FAILED(hr))
			hrr = hr;
	}
	if(FAILED(hrr))
		bool q = false;
	/**/
	return hrr;
}

//HRESULT CDVBMpeg2::ByteStreamPut(CInputThunk* pThunk, int chan, word8* pbuf, int ndim)
STDMETHODIMP CDVBMpeg2::ByteStreamPut(int chan, UCHAR* pbuf, int ndim, BSM_HOSTCONTROLBLOCK* phcb)
{
	m_PutCount++;
	HRESULT hr = S_OK;
	HRESULT res= S_OK;
	/**/
	//if( (pbuf == NULL) || (ndim == 0) || (phcb->m_ActionType == BSM_AT_SLAVESEQEND)){
	if( (pbuf == NULL) || (ndim == 0) ){
		PrintOutputOrders();
		LOGICALCHANNELMAP::iterator it = begin();
		while(it != end()){
			if((*it).first.m_inorder == chan){
				//OnOrderClose(&((*it).first), &((*it).second));
				ChannKey k = (*it).first;
				Channel_Handler& h = (*it).second;
				std::map<short, CSection>::iterator t ;
				for(t = h.m_sec.begin(); t != h.m_sec.end(); t++)
					(*t).second.Free();
				if(k.m_PID == 0x1fff)
					bool q = true;
				EraseOrder(it);
				it = begin();
				continue;
			}
			it++;
		}
		m_PackHandler.ClearCounters(chan);
	}
	else{
		m_remains.Concatenate(chan, pbuf, ndim);
		ChannKey key;
		key.m_inorder = chan;
		UCHAR* pos_begin = NULL;
		while(m_remains.GetPacket(pos_begin)){
			int outputorder = -1;
			short pid = m_PackHandler.GetPid(pos_begin);
			key.m_PID = pid;
			if(m_PackHandler.ProcessPacket(CPacketHandling::PAIR(key.m_inorder,key.m_PID),pos_begin)){//проверка счетчика
				//if(m_analyse){
					//m_analyser.PutPacket(chan, pos_begin, pid);
					//if(m_analyser.UpgradeData())
						//m_analyser.DoDataExchangeDB();
				//}
				key.m_outnumber = IP;
				hr = ProcessPacketForThisOutputChannel(key, pos_begin);
				if(SUCCEEDED(hr))
					if(hr != S_FALSE){
						m_IP<<"put"<<std::endl;
					}
					key.m_outnumber = TV;
					hr = ProcessPacketForThisOutputChannel(key, pos_begin);
					if(SUCCEEDED(hr))
						if(hr != S_FALSE){
							m_TV<<"put"<<std::endl;
						}
						key.m_outnumber = UNKNOWN;
						hr = ProcessPacketForThisOutputChannel(key, pos_begin);
						if(SUCCEEDED(hr))
							if(hr != S_FALSE){
								m_UN<<"put"<<std::endl;
							}
							key.m_outnumber = DEM_UNKNOWN;
							hr = ProcessPacketForThisOutputChannel(key, pos_begin);
							if(SUCCEEDED(hr))
								if(hr != S_FALSE){
									m_DN<<"put"<<std::endl;
								}
			}//проверка счетчика
		}//while
		if(m_PutCount == 11)
			bool w = false;
		if(!m_remains.SaveRemain())
			res = E_FAIL;
	}//else
	if(FAILED(res))
		bool r = false;
	/**/
	return res;
}

//HRESULT CDVBMpeg2::ByteStreamClose(CInputThunk* pThunk)
STDMETHODIMP CDVBMpeg2::ByteStreamClose()
{
	HRESULT hr = S_OK;
	HRESULT hrr = S_OK;
	/*
	m_NIT<<std::endl<<"----------------------------------------"<<std::endl<<"end of file"<<std::endl;
	m_PAT<<std::endl<<"----------------------------------------"<<std::endl<<"end of file"<<std::endl;
	m_PMT<<std::endl<<"----------------------------------------"<<std::endl<<"end of file"<<std::endl;
	m_CAT<<std::endl<<"----------------------------------------"<<std::endl<<"end of file"<<std::endl;
	m_remains.Free();
	//std::map<int, BLOCK_REM>::iterator it;
	//for(it = m_remains.m_map.begin(); it != m_remains.m_map.end(); it++){
	//	int i = (*it).first;
	//	m_PackHandler.CloseOutputChannels(i);
	//}
	std::map< short , int >::iterator t;
	int i = 0;
	int x = m_PIDcount.size();
	for(t = m_PIDcount.begin(); t != m_PIDcount.end(); t++){
		short pid = (*t).first;
		int c = (*t).second;
		m_statfile<<(int)i<<" PID="<<hexw(1)<<(int)pid<<decw(1)<<"     count="<<(int)c<<std::endl;
		i++;
	}
	/**/
	if(FAILED(hrr))
		bool q = false;
	return hrr;
}

HRESULT CDVBMpeg2::ProcessPacketForThisOutputChannel(ChannKey key, UCHAR* packet)
{
	HRESULT hr = S_FALSE;
	std::set < ChannKey , LessForKey >::iterator t = m_availablekeys.find(key);
	if(t != m_availablekeys.end()){
		LOGICALCHANNELMAP::iterator instance;
		BSM_HOSTCONTROLBLOCK hcb;
		hcb.m_ActionType = BSM_AT_SLAVESEQCONTINUE;
		Channel_Handler* handler = get_OutputOrder(key, &instance);
		int outorder = -1;
		if(handler == NULL){
			if(m_pBsFilterSite)
				m_pBsFilterSite->get_NewChannelOrder(&outorder);
			else
				return E_FAIL;
			handler = put_OutputOrder(key, outorder, &instance);
			handler->m_opened = true;
			hcb.m_ActionType = BSM_AT_SLAVESEQBEGIN;
			CByteStreamHostThunkChannelData* p = CreateThunkChannel(outorder, key.m_inorder, NULL);
			p->Packet_ID = key.m_PID;
			p->key = key;
			if(handler == NULL)
				return E_FAIL;
		}
		outorder = handler->m_outchan;
		if(m_outers[key.m_outnumber]){
			if(key.m_outnumber == IP){

				UCHAR* ip_packet = NULL;
				int    lenght = 0;
				//if(handler->m_IpPack.Put(packet, key.m_PID, ip_packet, lenght)){
					//hr = m_outers[key.m_outnumber]->ByteStreamPut(outorder, packet, 188, &hcb);
					//delete[] ip_packet;
				//}
					//if((key.m_inorder == 0)  && (key.m_PID == 0x1003)){
						/**/
						//for(int i = 0; i < 188; i++)
							//m_1012<<hexw(2)<<(int)packet[i]<<" ";
						//m_1012<<std::endl<<std::endl<<std::endl<<"--------------------"<<std::endl;
						/**/
						std::pair<std::map<short,CSection>::iterator , bool > the;
						size_t siz = handler->m_sec.size();
						if(siz > 1)
							siz = 999;
						the.first = handler->m_sec.find(key.m_PID);
						bool q;
						if(the.first == handler->m_sec.end()){
							the = handler->m_sec.insert(make_pair(key.m_PID,CSection()));
							q = the.second;
						}
						int pos = 0;
						int p = 0;
						CSection& _s_ = the.first->second;
						while(pos < PACKET_LENGHT){
							_s_.PutPacket(packet, pos, p);
							if(_s_.IsComplete()){
								_s_.GetSection(ip_packet, lenght);
								/*if( (key.m_inorder == 0)  && (key.m_PID = 0x1003)){
									if(lenght > 0){
										UCHAR c = ip_packet[0];
										m_1012<<hexw(2)<<(int)c<<std::endl;
										m_1012<<hexw(2)<<(int)ip_packet[1]<<std::endl;
										m_1012<<hexw(2)<<(int)ip_packet[2]<<std::endl;
										int k = 0;
										for(int i = 3; i < lenght; i++){
											if(k == 16){k = 0;m_1012<<std::endl;};
											m_1012<<hexw(2)<<(int)ip_packet[i]<<" ";
											k++;
										}
										m_1012<<std::endl<<std::endl;
									}
									else{
										m_1012<<"секция нулевой длины!"<<std::endl;
									}
								}*/
								if(lenght == 0)
									bool q = false;
								if(smart_ip){
									if( (ip_packet[0] == 0x3e) && ((ip_packet[12]>>4)== 4) && (lenght >=20) ){
										hr = PutPacket(key.m_outnumber, pack_headers, outorder, &ip_packet[12], lenght-12, &hcb);
										hcb.m_ActionType = BSM_AT_SLAVESEQCONTINUE;
										hr = m_outers[key.m_outnumber]->ByteStreamPut(outorder, m_separator, 12, &hcb);
									}
								}
								else{
									hr = PutPacket(key.m_outnumber, pack_headers, outorder, ip_packet, lenght, &hcb);
									hcb.m_ActionType = BSM_AT_SLAVESEQCONTINUE;
									hr = m_outers[key.m_outnumber]->ByteStreamPut(outorder, m_separator, 12, &hcb);
								}
								_s_.Free();
							}
							pos = p;
						}
					//}//if(m_1003)
			}
			else{
				hr = PutPacket(key.m_outnumber, pack_headers, outorder, packet, PACKET_LENGHT, &hcb);
				switch(key.m_PID){
					case 0x10://NIT
						for(int i = 0; i<4; i++)
							m_NIT<<hexw(2)<<(int)packet[i]<<" ";
						m_NIT<<std::endl;
						for(int i = 4; i <PACKET_LENGHT; i++)
							m_NIT<<hexw(2)<<(int)packet[i]<<" ";
						m_NIT<<std::endl;
						m_NIT<<std::endl;
						break;
					case 0x0://PAT
						for(int i = 0; i<4; i++)
							m_PAT<<hexw(2)<<(int)packet[i]<<" ";
						m_PAT<<std::endl;
						for(int i = 0; i <PACKET_LENGHT; i++)
							m_PAT<<hexw(2)<<(int)packet[i]<<" ";
						m_PAT<<std::endl;
						m_PAT<<std::endl;
						break;
					case 0x1://CAT
						for(int i = 0; i<4; i++)
							m_CAT<<hexw(2)<<(int)packet[i]<<" ";
						m_CAT<<std::endl;
						m_CAT<<std::endl;
						for(int i = 0; i <PACKET_LENGHT; i++)
							m_CAT<<hexw(2)<<(int)packet[i];
						m_CAT<<std::endl;
						break;
				}
			}
		}
	}
	return hr;
}

void CDVBMpeg2::OnOrderClose(const ChannKey* pkey, Channel_Handler* pdata)
{
	HRESULT hr = S_OK;
	BSM_HOSTCONTROLBLOCK phcb;
	phcb.m_ActionType = BSM_AT_SLAVESEQEND;
	pdata->m_all_parts.clear();
	size_t size = pdata->m_Ip_Packet.size();
	for(size_t i = 0; i < size; i++){
		delete[] pdata->m_Ip_Packet[i].pbuf;
	}
	pdata->m_Ip_Packet.clear();
	BSM_HOSTCONTROLBLOCK hcb;
	hcb.m_ActionType = BSM_AT_SLAVESEQEND;
	m_fclo<<(int)pdata->m_outchan<<std::endl;
	if(m_outers[pkey->m_outnumber]){
		hr = m_outers[pkey->m_outnumber]->ByteStreamPut(pdata->m_outchan, NULL, 0, &hcb);
	}
	pdata->m_opened = false;
	CloseThunkChannel(pdata->m_outchan);
	m_fclose<<(int)pdata->m_outchan<<std::endl;
}
void CDVBMpeg2::PrintOutputOrders()
{
	if(!m_printed){
		m_printed = true;
		std::fstream f;
		f.open("Выходные номера логических каналов", std::ios_base::out);
		LOGICALCHANNELMAP::iterator it;
		for(it = begin(); it != end(); it++){
			f<<"("<<(*it).first.m_inorder<<" , "<<(*it).first.m_outnumber<<" , "<<hexw(2)<<(*it).first.m_PID<<decw(2)<<")  ";
			f<<(*it).second.m_outchan<<std::endl;
		}
	}
	return;
}

HRESULT CDVBMpeg2::PutPacket(int outnumber, bool headers, int outorder, const UCHAR* packet, int packet_lenght, BSM_HOSTCONTROLBLOCK* phcb)
{
	HRESULT hr = S_OK;
	if (headers) {
		for(int i = 0; i < 36; i++)
			m_pBuf[i] = 0;
		memcpy(m_pBuf+36, packet, PACKET_LENGHT);
		TPackPtr<TPackMain> pMain;
		pMain.unsafe(m_pBuf);
		pMain->m_packtotal = PACKET_LENGHT + sizeof(TPackMain) + sizeof(TPackReg);
		int packtotal = PACKET_LENGHT + 36;
		if(packtotal != pMain->m_packtotal)
			bool q = false;
		pMain->m_headertotal = 36;
		pMain->m_sign = 0xFF;
		pMain->m_length = 0x0C;
		pMain->m_checksum = pMain->checksum();
		TPackPtr<TPackReg> pReg;
		pReg.unsafe(m_pBuf+12);
		pReg->m_check = 3;
		pReg->m_prot1 = 0xf;
		pReg->m_prot2 = 0x1;
		pReg->m_offs1 = 0x0;
		pReg->m_offs2 = 0x0;
		pReg->m_offs3 = 0x0;
		pReg->m_sign = 0x01;
		pReg->m_length = 0x18;
		if( m_outers[outnumber] )
			hr = m_outers[outnumber]->ByteStreamPut(outorder, m_pBuf, pMain->m_packtotal, phcb);
	}
	else{
		UCHAR* p = const_cast<UCHAR*>(packet);
		if( m_outers[outnumber] )
			hr = m_outers[outnumber]->ByteStreamPut(outorder, p, packet_lenght, phcb);
	}
	return hr;
}
STDMETHODIMP CDVBMpeg2::get_prop(DVBMPEG2_PARAMS* params)
{
	const DVBMPEG2_PARAMS* p = this;
	*params = *p;
	return S_OK;
}

STDMETHODIMP CDVBMpeg2::put_prop(const DVBMPEG2_PARAMS* params)
{
	DVBMPEG2_PARAMS* p = this;
	*p = *params;
	return S_OK;
}

// удаляет начальные и конечные пробелы из переданной строки
void CDVBMpeg2::TruncateString(std::string& st1)
{
	size_t t = st1.size();
	int i = 0; int sz = 0;
	if( t > 0 )
	{
	sz = (int)t; i = sz; bool q = true;
	//удаление последних пробелов
	while( q && (st1.size() > 0) )
		{
		t = st1.size();
		if(st1[t-1] == ' ')
			st1.erase(t-1);
		else q = false;
		t = st1.size();
		}
	//удаление первых пробелов
	t = st1.size();	sz = (int)t;
	i = 0; int count = 0; q = true;
	//посчитать число пробелов
	while( q && i<sz ){
		if(st1[i] == ' ')
			count++;
		else 
			q = false;
		i++;
	}
	if( count >0){
		std::string st11;
		for( i = count; i<sz; i++)
			st11 += st1[i];
		st1.clear(); st1 = st11;
		}
	}
}
