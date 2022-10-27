#pragma once

//#define PACKET_LENGHT 0xBC	//188
//#define PACKET_LENGHT 0xc8		//200

typedef struct Channel_Proc
{
	UCHAR			m_buf[PACKET_LENGHT];			//массив накопления пакета
	int				m_defcount;								//количество заполненных байт пакета
	int				m_TimeOut;								//таймаут завершения логического канала
	Channel_Proc(){
		m_defcount = 0;
		m_TimeOut = 0;
		for(int i = 0; i < PACKET_LENGHT; i++){
			m_buf[i] = 0x0;
		}
	}
};


//структура для возможных значений идентификатора пакета транспортного уровня
typedef enum{
	PAT	=	0x0,
	CAT = 0x1,
	TSDT = 0x2
}PID;

typedef struct{
	UCHAR*					m_buf;			//указатель на буфер хранения IP-дейтаграммы, до ее полного завершения
	//std::map<UCHAR*> m_part;	//набор буферов содержащих части IP-дейтаграммы
	int							m_time;			//велечина для освобождения ресурсов, занимаемых данной IP
	int							m_counter;	//счетчик оставшегося для полноты числа пакетов,если = 0, то IP собрана
}Seans;