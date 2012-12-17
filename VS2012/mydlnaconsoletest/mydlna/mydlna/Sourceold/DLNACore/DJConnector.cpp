#include "DJConnector.h"

namespace deejay {

HttpConnector::HttpConnector(Callback *callback, const NPT_IpAddress& ifAddr)
	: m_callback(callback), m_ifAddr(ifAddr), m_started(false), m_abortFlag(false)
{
}

HttpConnector::~HttpConnector()
{
	stop();
}

NPT_UInt16 HttpConnector::port() const
{
	return m_BoundPort;
}

NPT_Result HttpConnector::start()
{
	if (m_started) {
		return NPT_ERROR_INVALID_STATE;
	}

	NPT_Result nr;

	Config cfg;
	cfg.m_ListenAddress     = m_ifAddr;
	cfg.m_ListenPort        = 0;
	cfg.m_IoTimeout         = NPT_HTTP_SERVER_DEFAULT_IO_TIMEOUT;
	cfg.m_ConnectionTimeout = NPT_HTTP_SERVER_DEFAULT_CONNECTION_TIMEOUT;
	nr = SetConfig(cfg);
	if (NPT_FAILED(nr)) {
		return nr;
	}

	m_abortFlag = false;
	nr = NPT_Thread::Start();
	if (NPT_FAILED(nr)) {
		return nr;
	}

	m_started = true;
	return NPT_SUCCESS;
}

void HttpConnector::stop()
{
	if (m_started) {
		m_abortFlag = true;
		NPT_HttpServer::Abort();
		NPT_Thread::Wait();
		m_started = false;
	}
}

void HttpConnector::Run()
{
	NPT_Result nr;
	for (;;) {
		NPT_Socket *client = NULL;
		nr = m_Socket.WaitForNewClient(client, m_Config.m_ConnectionTimeout, NPT_SOCKET_FLAG_CANCELLABLE);
		if (m_abortFlag) {
			if (client) {
				delete client;
			}
			break;
		}

		if (NPT_SUCCEEDED(nr)) {
			client->SetReadTimeout(m_Config.m_IoTimeout);
			client->SetWriteTimeout(m_Config.m_IoTimeout);
			m_callback->httpConnectorOnNewClient(this, client);
		} else {
			NPT_System::Sleep(1.0);
		}
	}
}

SsdpConnector::SsdpConnector(Callback *callback, const NPT_IpAddress& ifAddr)
	: m_callback(callback), m_ifAddr(ifAddr), m_started(false), m_abortFlag(false), m_socket(NULL)
	, m_port(0)
{
}

SsdpConnector::~SsdpConnector()
{
	stop();
}

NPT_UInt16 SsdpConnector::port() const
{
	return m_port;
}

NPT_Result SsdpConnector::start()
{
	if (m_started) {
		return NPT_ERROR_INVALID_STATE;
	}

	m_socket = new NPT_UdpMulticastSocket(NPT_SOCKET_FLAG_CANCELLABLE);
	m_socket->SetInterface(m_ifAddr);

	NPT_Result nr;

	nr = m_socket->Bind(NPT_SocketAddress(NPT_IpAddress::Any, 1900), true);
	if (NPT_FAILED(nr)) {
		delete m_socket;
		m_socket = NULL;
		return nr;
	}

	NPT_IpAddress groupAddr(239, 255, 255, 250);
	nr = m_socket->JoinGroup(groupAddr, m_ifAddr);
	if (NPT_FAILED(nr)) {
		delete m_socket;
		m_socket = NULL;
		return nr;
	}

	NPT_SocketInfo socketInfo;
	m_socket->GetInfo(socketInfo);
	m_port = socketInfo.local_address.GetPort();

	m_abortFlag = false;
	nr = NPT_Thread::Start();
	if (NPT_FAILED(nr)) {
		delete m_socket;
		m_socket = NULL;
		return nr;
	}

	m_started = true;
	return NPT_SUCCESS;
}

void SsdpConnector::stop()
{
	if (m_started) {
		m_abortFlag = true;
		m_socket->Cancel();
		NPT_Thread::Wait();
		m_started = false;
		delete m_socket;
		m_socket = NULL;
	}
}

void SsdpConnector::Run()
{
	NPT_Result nr;
	NPT_DataBuffer packet(4096 * 4); // TODO: should be enough ?
	NPT_SocketAddress fromAddr;

	for (;;) {
		nr = m_socket->Receive(packet, &fromAddr);
		if (m_abortFlag) {
			break;
		}

		if (NPT_SUCCEEDED(nr)) {
			char *data = reinterpret_cast<char*>(packet.UseData());
			*(data + packet.GetDataSize()) = 0;
			m_callback->ssdpConnectorOnPacket(this, data, packet.GetDataSize(), fromAddr);
		} else {
			NPT_System::Sleep(1.0);
		}
	}
}

} // namespace deejay
