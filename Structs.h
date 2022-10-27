#pragma once

//описание типа выхода - или вид трафика
typedef enum{
	IP					= 0x0000,					//IP пакеты
	UNKNOWN			= 0x0001,					//неизвестные MPEG2 пакеты
	DEM_UNKNOWN = 0x0002,					//демультиплексированные неизвестные MPEG2 пакеты
	TV					= 0x0003,					//пакеты относящиеся к телевидению
}OUT_DESCRIPTION;

//определение выхода - описание выхода и набор идентификаторов пакетов, которые следует
//подавать на данный выход
typedef struct{
	std::map<short , std::string >			m_pids;						//набор идентификаторов пакетов (PID)
	OUT_DESCRIPTION										m_type;						//тип выхода
}OUT_DEFINITION;
//фильтр-компонент будет хранить мультикарту(карту с повторениями) индексируемую
//номером логического канала и данными которой будут заполненные структуры OUT_DEFINITION

/********************************************************************************************/
/* класс обработчик данных, поступающих с одного и тогоже логического канала               */
/********************************************************************************************/

typedef std::map<short , word32>::iterator								MAP_ITER;
typedef std::multimap<short , OUT_DESCRIPTION >::iterator OUT_ITER;


class Logical_Channel_Handler{
public:
	std::map < short , word32 >									m_outorders;		//карта порождаемых номеров выходных лог. каналов, индексируемая PID, инициализируется в процессе обработки блока данных
	std::multimap< short , OUT_DESCRIPTION >			m_outers;				//список выходов, на который следует подавать пакет, инициализуется один раз до начала обработки потока данных
	UCHAR														m_packege[PACKET_LENGHT];	//буфер для сохранения начала пакета, из предыдущего блока
	int															m_count;									//количество заполненных байт (остатка предыдущего блока)
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


	//добавление идентификатора (PID) в набор для заданного типа трафика
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
//реализовать GetStatistic(счетчик методов Put)
//и GetStaticticDescrition("Кол-во методов Put полученных фильтром MPEG2")