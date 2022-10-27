//#pragma once
////extern class CDVBMpeg2;
//class CPacketHandling
//{
//private :
//	class PAIR{
//	public:
//		int		InputChannel;
//		short	Pid;
//	};
//	class Less{
//	public:
//		bool operator()(const PAIR& key1, const PAIR& key2) const
//		{
//			if(key1.InputChannel != key2.InputChannel)
//				return key1.InputChannel < key2.InputChannel;
//			return key1.Pid < key2.Pid;
//		}
//	};
//public:
//	CPacketHandling(CDVBMpeg2* pFilter);
//	~CPacketHandling(void);
//
//	bool	GetOutputOrder(int InputChannel, short pid, BSM_HOSTCONTROLBLOCK* phcb);
//	void	FreeChannels(int InputChannel);
//	bool	OnIPOut(int InputChannel, short pid);
//	bool	OnTVOut(int InputChannel, short pid);
//	bool	OnUnknownOut(int InputChannel, short pid);
//	bool	OnDemUnknownOut(int InputChannel, short pid);
//
//private:
//	std::map< PAIR, word32, Less>		m_available;
//	std::map< PAIR, word32, Less>		m_actual;
//
//	CDVBMpeg2* m_pFilter;
//};
