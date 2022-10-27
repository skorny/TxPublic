#pragma once
//���������� ������ ����� ���� � ����� �� ���������
//0 - ������ ��������� ���������
//1 - ������ �� ���������, ������� ��������� �����
//2 - ������ ���������� ������
typedef enum
{
	SECTION_COMPLETE	= 0x0,
	SECTION_WAITING		= 0x1,
	SECTION_ERROR			= 0x2,
	SECTION_STUFF			= 0x3,
	SECTION_EMPTY			= 0x4
}SECTION_STATE;

//����� ���������� ������ ������, ����� ������ � �������� ����
class CSection
{
public:
	CSection(void);
	~CSection(void);

public:
	UCHAR*					m_pbuf;							//����� �������� ������
	unsigned short	m_ndim;							//����� ������
	unsigned short	m_count;						//���������� ���������� ���� ������, ��� ������ m_ndim=m_count, ������ ��������� ��������
	UCHAR						table_id;						//������������� �������
private:
	SECTION_STATE		m_state;						//������������� ��������� ���������� ������

public:

	//�������� �����(� ����� �������������) � �������, � �������
	//���������� �������� ������ ������, ���������� ������� ��������� ������ ������
	//��� ����� ������
	bool PutPacket(const UCHAR* packet, int pos_begin, int& pos_end);

	SECTION_STATE GetState();

	//���������� ������ ���� ������ ��������� ���������
	bool IsComplete();

	//���������� ��������� �� ������ � �� �����
	void GetSection(UCHAR*& pbuf, int& ndim);

	//����������� ������ ������� ������ �������
	void Free();

private:
	//�������� ������ ��� ����� (���������) ������ (������������ � ������� � ������ PID)
	//�������������� ����� ������
	//���������� ������� ��������� ������ ��� ����� ������
	bool CreateNew(const UCHAR* packet, int pos_begin, int& pos_end);
};
