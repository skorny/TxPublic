#pragma once
//накопитель секции может быть в одном из состоянии
//0 - секция полностью накоплена
//1 - секция не накоплена, ожидает следующую часть
//2 - ошибка накопления секции
typedef enum
{
	SECTION_COMPLETE	= 0x0,
	SECTION_WAITING		= 0x1,
	SECTION_ERROR			= 0x2,
	SECTION_STUFF			= 0x3,
	SECTION_EMPTY			= 0x4
}SECTION_STATE;

//класс накопления полной секции, буфер секции в исходном виде
class CSection
{
public:
	CSection(void);
	~CSection(void);

public:
	UCHAR*					m_pbuf;							//буфер хранения секции
	unsigned short	m_ndim;							//длина секции
	unsigned short	m_count;						//количество полученных байт секции, как только m_ndim=m_count, секция полностью получена
	UCHAR						table_id;						//идентификатор таблицы
private:
	SECTION_STATE		m_state;						//характиризует состояние накопителя секции

public:

	//получает пакет(с байта синхронизации) и позицию, с которой
	//начинается передача данной секции, возвращает позицию окончания данной секции
	//или длину пакета
	bool PutPacket(const UCHAR* packet, int pos_begin, int& pos_end);

	SECTION_STATE GetState();

	//возвращает истину если секция полностью накоплена
	bool IsComplete();

	//возвращает указатель на секцию и ее длину
	void GetSection(UCHAR*& pbuf, int& ndim);

	//освобождает память занятую данной секцией
	void Free();

private:
	//выделяет память под новую (следующую) секцию (передаваемую в пакетах с данным PID)
	//инициализирует новую секцию
	//возвращает позицию окончания секции или длину пакета
	bool CreateNew(const UCHAR* packet, int pos_begin, int& pos_end);
};
