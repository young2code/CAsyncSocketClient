#include "stdafx.h"

#include "ClientSocket.h"
#include "LogMan.h"

#include <boost/array.hpp>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
 
#pragma warning(disable:4996) //4996: 'std::copy': Function call with parameters that may be unsafe - this call relies on the caller to check that the passed values are correct. To disable this warning, use -D_SCL_SECURE_NO_WARNINGS. See documentation on how to use Visual C++ 'Checked Iterators'

#undef min
#undef max


namespace
{
	const int kMaxDataSize = 1024;
};

ClientSocket::ClientSocket()
	: mRecvBuffer(kMaxDataSize)
	, mSendBuffer(kMaxDataSize)
{
}


ClientSocket::~ClientSocket()
{
}


void ClientSocket::ResolveLocalAddress()
{
	CString address;
	UINT port = 0;
	GetSockName(address, port);
	address.AppendFormat(":%d", port);
	mLocalAddress = address;
}


void ClientSocket::ResolvePeerAddress()
{
	CString address;
	UINT port = 0;
	GetPeerName(address, port);
	address.AppendFormat(":%d", port);
	mPeerAddress = address;
}


void ClientSocket::OnConnect(int nErrorCode)
{
	if (0 != nErrorCode)
	{
		LOG("ClientSocket::OnConnect - error[%d]. [%s]", nErrorCode, mPeerAddress.c_str());
		Close();
		return;
	}

	ResolvePeerAddress();
	LOG("ClientSocket::OnConnect - succeeded! server:[%s]", mPeerAddress.c_str());

	// TEST
	{
		rapidjson::Document data;
		data.SetObject();
		data.AddMember("hello", "world", data.GetAllocator());
		data.AddMember("test", 1234, data.GetAllocator());

		rapidjson::Value testObj(rapidjson::kObjectType);
		testObj.AddMember("testBool", true, data.GetAllocator());
		data.AddMember("testObj", testObj, data.GetAllocator());

		rapidjson::Value testArray(rapidjson::kArrayType);
		testArray.PushBack(0, data.GetAllocator()); 
		testArray.PushBack(1, data.GetAllocator()); 
		testArray.PushBack(2, data.GetAllocator());
		data.AddMember("testArray", testArray, data.GetAllocator());

		PostSend(data);
	}
}


void ClientSocket::OnReceive(int nErrorCode)
{
	if (0 != nErrorCode)
	{
		LOG("ClientSocket::OnReceive - error[%d]. [%s]", nErrorCode, mPeerAddress.c_str());

		Close();
		return;
	}

	char temp[kMaxDataSize];
	int result = Receive(temp, sizeof(temp));

	if (0 == result)
	{
		LOG("ClientSocket::OnReceive - closed. [%s]", mPeerAddress.c_str());
		Close();
		return;
	}

	if (SOCKET_ERROR == result)
	{
		if (WSAEWOULDBLOCK != GetLastError())
		{
			LOG("ClientSocket::OnReceive - receive error[%d]. [%s]", GetLastError(), mPeerAddress.c_str());
			Close();
		}
		return;
	}

	int available = mRecvBuffer.capacity() - mRecvBuffer.size();
	if (available < result)
	{
		mRecvBuffer.set_capacity(mRecvBuffer.capacity() + result*2);
	}
	ASSERT(mRecvBuffer.capacity() - mRecvBuffer.size() >= static_cast<size_t>(result));

	mRecvBuffer.insert(mRecvBuffer.end(), temp, temp + result);

	LOG("ClientSocket::OnReceive - received [%d]. [%s]", result, mPeerAddress.c_str());

	GenerateJSON();
}


void ClientSocket::OnSend(int nErrorCode)
{
	if (0 != nErrorCode)
	{
		LOG("ClientSocket::OnSend - error[%d]. [%s]", nErrorCode, mPeerAddress.c_str());

		Close();
		return;
	}

	TrySend();
}


void ClientSocket::OnClose(int nErrorCode)
{
	LOG("ClientSocket::OnClose - [%s]", mPeerAddress.c_str());
	//Server::Instance()->OnClose(this);
}


void ClientSocket::PostSend(const char* jsonStr, int total)
{
	int available = mSendBuffer.capacity() - mSendBuffer.size();
	if (available < total)
	{
		mSendBuffer.set_capacity(mSendBuffer.capacity() + total*2);
	}
	ASSERT(mSendBuffer.capacity() - mSendBuffer.size() >= static_cast<size_t>(total));

	mSendBuffer.insert(mSendBuffer.end(), jsonStr, jsonStr + total); 

	TrySend();
}


void ClientSocket::PostSend(const rapidjson::Document& data)
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	data.Accept(writer);

	PostSend(buffer.GetString(), buffer.Size()+1);
}


void ClientSocket::GenerateJSON()
{
	RingBuffer::iterator itorEnd = std::find(mRecvBuffer.begin(), mRecvBuffer.end(), '\0');

	while (itorEnd != mRecvBuffer.end())
	{
		++itorEnd;

		boost::array<char, kMaxDataSize> jsonStr;

		ASSERT(std::distance(mRecvBuffer.begin(), itorEnd) <= kMaxDataSize);
		std::copy(mRecvBuffer.begin(), itorEnd, jsonStr.begin());

		mRecvBuffer.erase(mRecvBuffer.begin(), itorEnd);

		rapidjson::Document jsonData;
		jsonData.Parse<0>(jsonStr.data());

		if (jsonData.HasParseError())
		{
			LOG("ClientSocket::GenerateJSON - parsing failed. %s error[%s], %s", jsonStr.data(), jsonData.GetParseError(), mPeerAddress.c_str());
		}
		else
		{
			LOG("ClientSocket::GenerateJSON - parsing succeeded. %s, %s", jsonStr.data(), mPeerAddress.c_str());

			//Server::Instance()->OnReceive(this, jsonData);
		}

		itorEnd = std::find(mRecvBuffer.begin(), mRecvBuffer.end(), '\0');
	}
}


void ClientSocket::TrySend()
{
	while (!mSendBuffer.empty())
	{
		boost::array<char, kMaxDataSize> temp;

		RingBuffer::iterator itorEnd = mSendBuffer.begin();
		size_t total = std::min<size_t>(mSendBuffer.size(), kMaxDataSize);
		std::advance(itorEnd, total);

		ASSERT(std::distance(mSendBuffer.begin(), itorEnd) <= kMaxDataSize);
		std::copy(mSendBuffer.begin(), itorEnd, temp.begin());

		int result = Send(temp.data(), total);

		if (SOCKET_ERROR == result)
		{
			if (WSAEWOULDBLOCK != GetLastError())
			{
				LOG("ClientSocket::OnSend - send error[%d]. [%s]", GetLastError(), mPeerAddress.c_str());
				Close();				
			}
			return;
		}
		
		ASSERT(result > 0);

		LOG("ClientSocket::OnSend - sending succeeded. %d / %d . %s", result, total, mPeerAddress.c_str());

		itorEnd = mSendBuffer.begin();
		std::advance(itorEnd, result);
		mSendBuffer.erase(mSendBuffer.begin(), itorEnd);
	}
}
