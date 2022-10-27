// cDVBMpeg2.h : Declaration of the CDVBMpeg2

#pragma once
#include "resource.h"       // main symbols
/**/
#include <devctlpp.h>
#include <sessionpp.h>

#include <ByteStreampp.h>
#include <map>
#include <set>
#include <BsFilterSupport.h>
using namespace bsm;


/**/
#include "..\include\DVBMpeg2.h"

extern const CLSID CLSID_Page1;
/**/
typedef struct IP_BLOCK{
	UCHAR*			pbuf;
	int					ndim;
};
/**/
class Channel_Handler : public TLogicalChannelInstance
{
public:
	std::vector < IP_BLOCK >				m_Ip_Packet;
	std::set< int >									m_all_parts;
	bool														m_opened;
//	IpPackets												m_IpPack;
	Channel_Handler()
	{
		m_opened = false;
	}
	~Channel_Handler()
	{
	}
	std::map<short, CSection>									m_sec;
};
/**/
class ChannKey
{
public:
	int				m_inorder;
	int				m_outnumber;
	short			m_PID;
};

class LessForKey
{
 public:
   bool operator()(const ChannKey& key1, const ChannKey& key2) const
   {
		 if (key1.m_inorder != key2.m_inorder)
			 return key1.m_inorder < key2.m_inorder;
		 else{
			 if(key1.m_PID != key2.m_PID)
				 return key1.m_PID < key2.m_PID;
			 else
			 return key1.m_outnumber < key2.m_outnumber;
		 }
   }
};

/**/
 //CDVBMpeg2
class CByteStreamHostThunkChannelData:
	public bsm::TByteStreamHostThunkChannelData
{
public:
	short Packet_ID;
	ChannKey key;
	CByteStreamHostThunkChannelData(size_t inchan = 0)
		: bsm::TByteStreamHostThunkChannelData(inchan),Packet_ID(0)
		/*, m_pInst(NULL), m_sgncode(0)*/ {}
};

class CDVBMpeg2;
typedef bsm::TBsFilterInputThunk<CDVBMpeg2> CInputThunk;

/****************************************************************************************/
/**/
/****************************************************************************************/
class CPacketHandling
{
public:
	class PAIR{
	public:
		int		InputChannel;
		short	Pid;
		PAIR(int i, short s){InputChannel = i; Pid = s;};
		PAIR(){};
	};
private :
	class Less{
	public:
		bool operator()(const PAIR& key1, const PAIR& key2) const
		{
			if(key1.InputChannel != key2.InputChannel)
				return key1.InputChannel < key2.InputChannel;
			return key1.Pid < key2.Pid;
		}
	};
public:
	
	CPacketHandling(CDVBMpeg2* pFilter);
	~CPacketHandling(void);

	short		GetPid(UCHAR* pack);
	bool		GetOutputOrder(int InputChannel, short pid, int& outorder, BSM_HOSTCONTROLBLOCK* phcb);
	HRESULT	CloseOutputChannels(int InputChannel, BSM_HOSTCONTROLBLOCK* phcb);
	bool		OnIPOut(int InputChannel, short pid);
	bool		OnTVOut(int InputChannel, short pid);
	bool		OnUnknownOut(int InputChannel, short pid);
	bool		OnDemUnknownOut(int InputChannel, short pid);
	bool		InitFromXMLFile();
	bool		ProcessPacket(PAIR k, const UCHAR* packet);
	void		ClearCounters(int InChannel);

private:
	std::map< PAIR, byte, Less>			m_continuity_counters;
	std::set< PAIR, Less>						m_available;
	std::map< PAIR, word32, Less>		m_actual;

	std::set< PAIR, Less>						m_KeysForIP;
	std::set< PAIR, Less>						m_KeysForTV;
	std::set< PAIR, Less>						m_KeysForUnknown;
	std::set< PAIR, Less>						m_KeysForDemUnknown;

	CDVBMpeg2* m_pFilter;
};
/****************************************************************************************/
/**/
/****************************************************************************************/

class ATL_NO_VTABLE  CDVBMpeg2: 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDVBMpeg2, &CLSID_DVBMpeg2>,
	public IPersistStorageImpl<CDVBMpeg2>,
	public ISpecifyPropertyPagesImpl<CDVBMpeg2>,
	public IDVBMpeg2,
	public bsm::TByteStreamHostThunkImpl<CByteStreamHostThunkChannelData>,
	public bsm::IBsFilterImpl<CDVBMpeg2>,
	public IComponentControlImpl<CDVBMpeg2>,
	public bsm::TByteStreamBase,
	public TLogicalChannelMap< ChannKey, Channel_Handler, LessForKey>,
	public DVBMPEG2_PARAMS
{
	friend class CPacketHandling;
public:
	CDVBMpeg2();

	DECLARE_REGISTRY_RESOURCEID(IDR_DVBMPEG21)
	DECLARE_GET_CONTROLLING_UNKNOWN()


	BEGIN_COM_MAP(CDVBMpeg2)
		COM_INTERFACE_ENTRY(IDVBMpeg2)
		COM_INTERFACE_ENTRY(IBsFilter)
		COM_INTERFACE_ENTRY(IComponentControl)
		COM_INTERFACE_ENTRY(IByteStreamHost)
		COM_INTERFACE_ENTRY(IByteStream)
		/**/
		COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
		COM_INTERFACE_ENTRY(IPersistStorage)
		COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFreeMarshaler.p)
		/**/
	END_COM_MAP()

	BEGIN_CATEGORY_MAP(CDVBMpeg2)
		IMPLEMENTED_CATEGORY(CATID_ControllingComponent)
		IMPLEMENTED_CATEGORY(bsm::CATID_BsFilter)
	END_CATEGORY_MAP()

	BEGIN_PROP_MAP(CDVBMpeg2)
		PROP_PAGE(CLSID_Page1)
	END_PROP_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()
	HRESULT FinalConstruct();
	HRESULT ProcessPacketForThisOutputChannel(ChannKey key, UCHAR* packet);
	HRESULT PutPacket(int outnumber, bool headers, int outorder, const UCHAR* packet, int packet_lenght, BSM_HOSTCONTROLBLOCK* phcb);
	void FinalRelease();

	//virtual void OnOrderClose(const int* pkey, Logical_Channel_Handler* pdata);
	void	OnOrderClose(const ChannKey* pkey, Channel_Handler* pdata);
	void	TestInitConfiguration();
	void	PrintOutputOrders();
	void	TruncateString(std::string& s); 

private:
	bool																			m_printed;
	std::fstream															m_1012;
	int																				m_PutCount;
	UCHAR																			m_separator[12];
	UCHAR																			m_pBuf[MAX_BUF_LENGHT]; //4096 - макс-я длина секции + 36 - длина основного заголовка пакета и заголовка регистрации
	bool																			opened_flag;
	std::fstream															m_fclo;
	std::fstream															m_IP;
	std::fstream															m_TV;
	std::fstream															m_UN;
	std::fstream															m_DN;
	std::fstream															m_statfile;
	std::fstream															m_foutput;
	std::fstream															m_fclose;
	std::fstream															m_NIT;
	std::fstream															m_PAT;
	std::fstream															m_PMT;
	std::fstream															m_CAT;
//	std::map<int, Channel_Proc>								m_channel_proc;
	std::map< short , int>												m_PIDcount;
	unsigned int															m_decount;
	int																				m_param;
	BSM_HOSTCONTROLBLOCK											m_hcb;
	BSM_HOSTRESOURCES													m_h;
	BSM_FILTERMODE														m_Mode;
	//bool																			m_SMART_IP;
	CComPtr<IUnknown>													m_pFreeMarshaler;
	CComQIPtr<IBsFilterSite>									m_pBsFilterSite;
	//std::map< int, CComQIPtr<IByteStream> >		m_outers1;

//	std::map< int, Logical_Channel_Handler>		m_handlers;
	CPacketHandling														m_PackHandler;
	CRemains																	m_remains;

	CAnalyser																	m_analyser;
	BOOL																			m_analyse;

//public:
	//4 выхода:
	//m_outers[0] - тип трафика IP
	//m_outers[1] - тип трафика неизвестный MPEG2
	//m_outers[2] - тип трафика демультиплексированный неизвестный MPEG2
	//m_outers[3] - тип трафика TV
	CComQIPtr<IByteStream>*										m_outers;
	int																				m_debug;

public:
//	интерфейс IByteStream
	//HRESULT ByteStreamOpen(CInputThunk* pThunk);
	//HRESULT ByteStreamPut(CInputThunk* pThunk, int chan, word8* pbuf, int ndim);
	//HRESULT ByteStreamClose(CInputThunk* pThunk);
	//STDMETHOD(CreateInputThunk)(int outer, IByteStream** ppBs, VARIANT_BOOL* pbMultiInput);

	STDMETHOD(ByteStreamOpen)(const BSM_HOSTRESOURCES* phr, IByteStreamHost* pi, BSM_HOSTCONTROLBLOCK* phcb, int n);
	STDMETHOD(ByteStreamPut)(int chan, UCHAR* pbuf, int ndim, BSM_HOSTCONTROLBLOCK* phcb);
	STDMETHOD(ByteStreamClose)();

	//IComponentControl
	STDMETHOD(Save)(IStorage *pStorage, BOOL bSameAsLoad);
	STDMETHOD(Load)(IStorage *pStorage);
	STDMETHOD(InitNew)();
	STDMETHOD(InitInstance)();
	STDMETHOD(EnableInstance)();
	STDMETHOD(DisableInstance)();
	/**/
	//Интерфейс IBsFilter
	STDMETHOD(put_Bind)(int outer, IUnknown* punkn);
	STDMETHOD(get_Bind)(int outer, IUnknown** punkn);
	STDMETHOD(put_Mode)(BSM_FILTERMODE mode);
	STDMETHOD(get_Mode)(BSM_FILTERMODE* mode);
	STDMETHOD(put_Site)(IUnknown* punkn);
	STDMETHOD(get_OuterCount)(int* outer_count);
	STDMETHOD(get_OuterDescription)(int outer, BSTR *description);
	STDMETHOD(get_Statistics)(BSTR *str);
	STDMETHOD(get_StatisticsDescription)(BSTR *str);
	STDMETHOD(ResetStatistics)();
	STDMETHOD(Flush)();
	STDMETHOD(GetHeader)(int outchan, ULONG Signature, BSM_HEOF* ph, BOOL* pApplyToDst);
	STDMETHOD(get_prop)(DVBMPEG2_PARAMS* params);
	/**//*
	STDMETHOD(GetHeaderSignatures)(int outchan, ULONG** ppSignature, ULONG* pnDim);
	STDMETHOD(GetHeader)(int outchan, ULONG Signature, BSM_HEOF* ph, BOOL* pApplyToDst);
	/**/

	//карта с повторениями для хранения описания заданных наборов
	//идентификаторов пакетов, подаваемых на выходы фильтра
	//индексируется номером входного логиеского канала
	//данные - описание типа выхода и карта идентификаторов пакетов, подаваемых на 
	//выход данного типа
//	std::multimap<int , OUT_DEFINITION>			m_outers_configuration;		//инициализируется из XML файла, редактируемого пользователем
	std::set< ChannKey , LessForKey >				m_availablekeys;					//карта допустимых значений ключей для обработчиков 
	
	STDMETHOD(put_prop)(const DVBMPEG2_PARAMS* params);
};

OBJECT_ENTRY_AUTO(__uuidof(DVBMpeg2), CDVBMpeg2)
