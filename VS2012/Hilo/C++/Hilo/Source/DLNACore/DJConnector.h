#ifndef __DJConnector_h__
#define __DJConnector_h__

#include <Neptune.h>

namespace deejay {

class HttpConnector
	: public NPT_HttpServer
	, public NPT_Thread
{
public:
	class Callback
	{
	public:
		virtual void httpConnectorOnNewClient(HttpConnector *connector, NPT_Socket *client) = 0;
	};

	HttpConnector(Callback *callback, const NPT_IpAddress& ifAddr);
	virtual ~HttpConnector();

	NPT_Result start();
	void stop();

	virtual void Run();

	NPT_UInt16 port() const;

private:
	Callback *m_callback;
	NPT_IpAddress m_ifAddr;
	bool m_started;
	bool m_abortFlag;
};

class SsdpConnector
	: public NPT_Thread
{
public:
	class Callback
	{
	public:
		virtual void ssdpConnectorOnPacket(SsdpConnector *connector, const char *data, NPT_Size length, const NPT_SocketAddress& fromAddr) = 0;
	};

	SsdpConnector(Callback *callback, const NPT_IpAddress& ifAddr);
	virtual ~SsdpConnector();

	NPT_Result start();
	void stop();

	virtual void Run();

	NPT_UInt16 port() const;

private:
	Callback *m_callback;
	NPT_IpAddress m_ifAddr;
	bool m_started;
	bool m_abortFlag;
	NPT_UdpMulticastSocket *m_socket;
	NPT_UInt16 m_port;
};

} // namespace deejay

#endif // __DJConnector_h__
