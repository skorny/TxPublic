#pragma once

//#define PACKET_LENGHT 0xBC	//188
//#define PACKET_LENGHT 0xc8		//200

typedef struct Channel_Proc
{
	UCHAR			m_buf[PACKET_LENGHT];			//������ ���������� ������
	int				m_defcount;								//���������� ����������� ���� ������
	int				m_TimeOut;								//������� ���������� ����������� ������
	Channel_Proc(){
		m_defcount = 0;
		m_TimeOut = 0;
		for(int i = 0; i < PACKET_LENGHT; i++){
			m_buf[i] = 0x0;
		}
	}
};


//��������� ��� ��������� �������� �������������� ������ ������������� ������
typedef enum{
	PAT	=	0x0,
	CAT = 0x1,
	TSDT = 0x2
}PID;

typedef struct{
	UCHAR*					m_buf;			//��������� �� ����� �������� IP-�����������, �� �� ������� ����������
	//std::map<UCHAR*> m_part;	//����� ������� ���������� ����� IP-�����������
	int							m_time;			//�������� ��� ������������ ��������, ���������� ������ IP
	int							m_counter;	//������� ����������� ��� ������� ����� �������,���� = 0, �� IP �������
}Seans;