#pragma once
#include "Socket.h"
#include "ThreadPool.h"
#include <string>
#include <map>

enum class ServerStatus
{
	UNINITIALIZED = 0,
	INITIALIZED,
	RUNNING,
	STOPPED
};

enum class RTSPMethod
{
	OPTIONS = 0,
	DESCRIBE,
	SETUP,
	PLAY,
	PAUSE,
	TEARDOWN
};

class RTSPRequest
{
public:
	RTSPRequest();
	RTSPRequest(const RTSPRequest& other);
	RTSPRequest& operator=(const RTSPRequest& other);
	~RTSPRequest();
private:
	int m_method;
};

class RTSPResponse
{
public:
	RTSPResponse();
	RTSPResponse(const RTSPResponse& other);
	RTSPResponse& operator=(const RTSPResponse& other);
	~RTSPResponse();
private:
	int m_method;
};

class RTSPSession
{
public:
	RTSPSession();
	RTSPSession(const RTSPSession& other);
	RTSPSession& operator=(const RTSPSession& other);
	~RTSPSession();
};

class RTSPServer : public ThreadFuncBase
{
public:
	RTSPServer() :
		m_socket(true),
		m_status(ServerStatus::UNINITIALIZED)
	{
	}
	int Init(const std::string& strIP = "0.0.0.0", USHORT port = 8554);
	int Invoke();
	void Stop();
	~RTSPServer();
protected:
	int ThreadWorker();
	RTSPRequest ParseRequest(const std::string &data);
	RTSPResponse MakeResponse(RTSPRequest &request);
	int ThreadSSession();
private:
	ServerStatus m_status;
	CThread m_threadMain;
	SPSocket m_socket;
	ThreadPool m_pool;
	std::map<std::string, RTSPSession> m_mapSessions;
	static SocketInitializer m_initializer;
};
