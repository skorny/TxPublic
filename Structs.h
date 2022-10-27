#pragma once

//�������� ���� ������ - ��� ��� �������
typedef enum{
	IP					= 0x0000,					//IP ������
	UNKNOWN			= 0x0001,					//����������� MPEG2 ������
	DEM_UNKNOWN = 0x0002,					//���������������������� ����������� MPEG2 ������
	TV					= 0x0003,					//������ ����������� � �����������
}OUT_DESCRIPTION;

//����������� ������ - �������� ������ � ����� ��������������� �������, ������� �������
//�������� �� ������ �����
typedef struct{
	std::map<short , std::string >			m_pids;						//����� ��������������� ������� (PID)
	OUT_DESCRIPTION										m_type;						//��� ������
}OUT_DEFINITION;
//������-��������� ����� ������� �����������(����� � ������������) �������������
//������� ����������� ������ � ������� ������� ����� ����������� ��������� OUT_DEFINITION

/********************************************************************************************/
/* ����� ���������� ������, ����������� � ������ � ������ ����������� ������               */
/********************************************************************************************/

typedef std::map<short , word32>::iterator								MAP_ITER;
typedef std::multimap<short , OUT_DESCRIPTION >::iterator OUT_ITER;


class Logical_Channel_Handler{
public:
	std::map < short , word32 >									m_outorders;		//����� ����������� ������� �������� ���. �������, ������������� PID, ���������������� � �������� ��������� ����� ������
	std::multimap< short , OUT_DESCRIPTION >			m_outers;				//������ �������, �� ������� ������� �������� �����, �������������� ���� ��� �� ������ ��������� ������ ������
	UCHAR														m_packege[PACKET_LENGHT];	//����� ��� ���������� ������ ������, �� ����������� �����
	int															m_count;									//���������� ����������� ���� (������� ����������� �����)
	Logical_Channel_Handler(){
		m_count = 0;
	}
	bool AddOutOrder(short pid, word32 outorder){
		std::pair< std::map< short , word32>::iterator , bool > p;
		p = m_outorders.insert(make_pair(pid, outorder));
		return p.second;
	};
	void DeleteOutOrder(word32 outorder){
		MAP_ITER it;
		it = m_outorders.find(outorder);
		if(it != m_outorders.end())
			m_outorders.erase(it);
	};


	//���������� �������������� (PID) � ����� ��� ��������� ���� �������
	/*bool AddPIDForTraffic(short pid, OUT_DESCRIPTION type){
		std::pair< short, std::string> p;
		p.first = pid;
		p.second = "";
		std::list< OUT_DEFINITION >::iterator it;
		for(it = m_outers.begin(); it != m_outers.end(); it++){
			if( (*it).m_type == type )
				(*it).m_pids.insert(p);
		}
		return true;
	}
	bool AddTrafficType(OUT_DESCRIPTION type){
		std::map< short , std::string> new_map;
		OUT_DEFINITION out;
		out.m_pids = new_map;
		out.m_type = type;
		bool flag = true;
		std::list< OUT_DEFINITION >::iterator it;
		for(it = m_outers.begin(); it != m_outers.end(); it++){
			if( (*it).m_type == type )
				flag = false;
		}
		if(flag)
			m_outers.push_back(out);
	}*/
};

//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//����������� GetStatistic(������� ������� Put)
//� GetStaticticDescrition("���-�� ������� Put ���������� �������� MPEG2")