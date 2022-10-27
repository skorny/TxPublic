#pragma once
typedef enum
{
	NIT			=			0x0010,
	PAT			=			0x0000,
	CAT			=			0x0001
}PID_TITLE;

typedef struct
{
	bool				actual;
	std::string	descriptors;
	std::string	name;
	ULONG				key;
	bool				chek;
	void DATA_NET(){};
}DATA_NET;

typedef struct
{
	short				original_network_id;
	std::string	descriptors;
	short				described_in_network;
	ULONG				key;
	bool				chek;
	void DATA_TS(){};
}DATA_TS;

typedef struct
{
	short				TS_id;
	bool				desc_in_SI;
	short				program_number;
	std::string	stream_type;
	byte				table_id;
	int					count;
	ULONG				key;
	bool				chek;
	void DATA_PID(){};
}DATA_PID;
/**********************************************************************/
/* ���������� TS ��� ����������� ������ �������� ����������� ������   */
/**********************************************************************/
class CTSAnalyser
{
public:
	CTSAnalyser(void);
	~CTSAnalyser(void);


public:
	HRESULT		PutPacket(const UCHAR* packet, short pid);
	bool			UpgradeData();
	HRESULT		DoDataExchangeDB();

	void			Free();
	
	byte			m_prev_counter;

private:
	int				m_secCount;
	bool			AnalyseSection(CSection* s, short pid);
	bool			m_Upgrade;
	bool			AnalysePAT(CSection* s, short pid);
	bool			AnalysePMT(CSection* s, short pid);
	
	//������������ ��� ������ ������
	std::map< short , CSection>					m_sections;

	//������ ���������� �� ��������� ������, �������������� �������
	std::map< short, DATA_NET>					m_Nets;
	std::map< short, DATA_TS>						m_Tss;
	std::map< short, DATA_PID>					m_Pids;


	//��� �������� ������� ������ PAT, ����� �������������
	//��������������� TS ������������ � ������� ������
	std::map<int, pair<UCHAR*, int> >			m_PATs;

	//��� �������� ������� ������ PAT, ����� �������������
	//��������������� ������ ������������ � ������� ������
	std::map<int, pair<UCHAR*, int> >			m_PMTs;

	//std::vector<pair<UCHAR*, int> >			m_NITs;
};

/**********************************************************************/
/* ����� ������������ TS, ��� ���� ������� �. �.								      */
/**********************************************************************/
class CAnalyser
{
	public:
	CAnalyser(void);
	~CAnalyser(void);

public:
	HRESULT		PutPacket(int InChann,const UCHAR* packet, short pid);
	bool			UpgradeData();
	HRESULT		DoDataExchangeDB();
	void			Free();

private:
	std::map<int, CTSAnalyser>			m_TsAnalyser;
	CTSAnalyser*										m_current;
};