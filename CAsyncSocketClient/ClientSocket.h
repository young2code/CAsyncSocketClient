#pragma once

#include <boost/circular_buffer.hpp>
#include <rapidjson/document.h>

class ClientSocket : public CAsyncSocket
{
public:
	ClientSocket();
	virtual ~ClientSocket();

	virtual void OnConnect(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);
	virtual void OnClose(int nErrorCode);

	void ResolveLocalAddress();
	const char* GetLocalAddress() const { return mLocalAddress.c_str(); }

	void ResolvePeerAddress();
	const char* GetPeerAddress() const { return mPeerAddress.c_str(); }

	void PostSend(const char* jsonStr, int total); // // total - size including null character.
	void PostSend(const rapidjson::Document& data);

private:
	void GenerateJSON();
	void TrySend();

private:
	typedef boost::circular_buffer<char> RingBuffer;
	RingBuffer mRecvBuffer;
	RingBuffer mSendBuffer;

	std::string mPeerAddress;
	std::string mLocalAddress;
};