#include <Windows.h>

#include <string>
#include <vector>
#include <iostream>

#include "chess.h"
#include "engine.h"

///////////////////////////////////////////////////////////////////////////////
CChessEngine::CChessEngine(DWORD TimePerMove, bool bWhite)
	: m_bInited(false)
	, m_bWhite(bWhite)
	, m_Estimate(0)
	, m_Move(0)
	, m_TimePerMove(TimePerMove)
{
	m_Position.reserve(30);
}


///////////////////////////////////////////////////////////////////////////////
CChessEngine::~CChessEngine()
{
	WriteFile(m_pIn.w, "quit\n", 5, nullptr, nullptr);
}



///////////////////////////////////////////////////////////////////////////////
int CChessEngine::Init(const std::wstring& path)
{
	if (m_bInited)
		return ERR_NOERROR;

	SECURITY_ATTRIBUTES	sa{ 0 };

	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = nullptr;

	if (!CreatePipe(&m_pIn.r, &m_pIn.w, &sa, 0) ||
		!CreatePipe(&m_pOut.r, &m_pOut.w, &sa, 0))
	{
		//GetLastError();
		MessageBox(nullptr, L"Failed to init the engine", path.c_str(), MB_ICONERROR | MB_OK);
		return ERR_BADPIPE;
	}

	STARTUPINFO st{ 0 };

	st.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	st.wShowWindow = SW_HIDE;
	st.hStdInput = m_pIn.r;
	st.hStdOutput = m_pOut.w;
	st.hStdError = m_pOut.w;

	if (!CreateProcess(nullptr, const_cast<LPWSTR>(path.c_str()), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &st, &m_Proc.m_Info))
	{
		//GetLastError();
		MessageBox(nullptr, L"Failed to init the engine", path.c_str(), MB_ICONERROR | MB_OK);
		return ERR_BADPROCESS;
	}

	CHECK_CALL(Query("uci"));

	m_bInited = true;

	return ERR_NOERROR;
}


///////////////////////////////////////////////////////////////////////////////
int CChessEngine::Query(const std::string& query) const
{
	DWORD Bytes;
	if (!WriteFile(m_pIn.w, query.c_str(), static_cast<DWORD>(query.size()), &Bytes, nullptr))
		return ERR_WRITE;

	return ERR_NOERROR;
}


///////////////////////////////////////////////////////////////////////////////
int CChessEngine::GetResponse(std::string& response, const std::vector<std::string>& expect) const
{
	DWORD Bytes;
	DWORD BytesAvail;
	DWORD BytesLeft;
	std::string Response;

	Response.resize(0x10000);

	while (true)
	{
		if (!PeekNamedPipe(m_pOut.r, (LPVOID)Response.data(), static_cast<DWORD>(Response.size()), &Bytes, &BytesAvail, &BytesLeft))
			return ERR_READ;

		if (!ReadFile(m_pOut.r, (LPVOID)Response.data(), static_cast<DWORD>(Response.size()), &Bytes, nullptr))
			return ERR_READ;

		if (expect.empty())
			break;

		for (const std::string s : expect) {
			if (Response.rfind(s) != std::string::npos)
				return ERR_NOERROR;
		}
	}

	return ERR_NOERROR;
}


///////////////////////////////////////////////////////////////////////////////
int CChessEngine::GetMove(std::string& move)
{
	CHECK_CALL(Query("position startpos moves " + m_Position + "\ngo\n"));
	/*std::string query = "position startpos moves " + m_Position + "\ngo\n";

	DWORD Bytes;
	if (!WriteFile(m_pIn.w, query.c_str(), static_cast<DWORD>(query.size()), &Bytes, nullptr))
		return ERR_WRITE;*/

	Sleep(m_TimePerMove);

	DWORD Bytes;
	DWORD BytesAvail;
	DWORD BytesLeft;
	std::string Response;

	Response.resize(0x10000);

	CHECK_CALL(GetResponse(Response, { STR_BESTMOVE, STR_MATE }));

	/*if (!PeekNamedPipe(m_pOut.r, (LPVOID)Response.data(), static_cast<DWORD>(Response.size()), &Bytes, &BytesAvail, &BytesLeft))
		return ERR_READ;

	if (!ReadFile(m_pOut.r, (LPVOID)Response.data(), static_cast<DWORD>(Response.size()), &Bytes, nullptr))
		return ERR_READ;*/

	size_t pos;

	if ((pos = Response.rfind(STR_BESTMOVE)) == std::string::npos)
	{
		if ((pos = Response.rfind(STR_MATE)) == std::string::npos)
		{
			std::cout << "Unable to find best move..." << std::endl;
			std::cout << Response.c_str() << std::endl;
			return ERR_BADMOVE;
		}
		m_Estimate = std::stoi(Response.c_str() + pos + sizeof(STR_MATE));
		return ERR_MATE;
	}

	pos += sizeof(STR_BESTMOVE);
	const size_t length = Response.find_first_of(' ', pos) - pos;
	move = Response.substr(pos, length);
	AddMove(move);

	if ((pos = Response.rfind(STR_SCORE)) == std::string::npos)
	{
		if ((pos = Response.rfind(STR_MATE)) == std::string::npos)
		{
			std::cout << "Unable to find score cp..." << std::endl;
			std::cout << Response.c_str() << std::endl;
			return ERR_BADMOVE;
		}
		m_Estimate = std::stoi(Response.c_str() + pos + sizeof(STR_MATE));
		return ERR_MATE;
	}

	m_Estimate = std::stoi(Response.c_str() + pos + sizeof(STR_SCORE));

	++m_Move;

	/*if ((pos = Response.rfind("info depth")) != std::string::npos)
		std::cout << "\n" << Response.data() + pos << std::endl;
	else
		std::cout << "\n" << Response << std::endl;*/

	return ERR_NOERROR;
}
