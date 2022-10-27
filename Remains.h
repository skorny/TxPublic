#pragma once

typedef struct BLOCK_REM
{
	UCHAR*	m_remain;
	int			m_dim;
	BLOCK_REM(){m_remain = NULL; m_dim = 0;};
};

class CRemains
{
public:
	std::map<int, BLOCK_REM>	m_map;
private:
	UCHAR*										m_pbuf;
	int												m_size;
public:
	int												m_pos;
private:
	int												m_chan;
	bool											m_newflag;
	

public:
	bool	Concatenate(int chan, UCHAR*& pbuf, int& ndim);
	void	FreeChannel(int InputChannel);
	bool	GetPacket(UCHAR*& pos_beg);
	bool	SaveRemain();
	void	Free();
	CRemains(void);
	~CRemains(void);
};
