#include <Windows.h>

#include <string>
#include <vector>
#include <iostream>

#include "chess.h"
#include "engine.h"

///////////////////////////////////////////////////////////////////////////////
CChessEngine::CChessEngine(DWORD TimePerMove, bool bWhite, const std::string& FirstMove)
	: m_bInited(false)
	, m_bWhite(bWhite)
	, m_Estimate(0)
	, m_Move(0)
	, m_TimePerMove(TimePerMove)
  , m_FirstMove(FirstMove)
{
	m_Position.reserve(30);
	m_Response.resize(0x1000);
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

	m_bInited = true;

	return ERR_NOERROR;
}


///////////////////////////////////////////////////////////////////////////////
int CChessEngine::Query(const std::string& query) const
{
	//std::cout << query << std::endl;

	DWORD Bytes;
	if (!WriteFile(m_pIn.w, query.c_str(), static_cast<DWORD>(query.size()), &Bytes, nullptr))
		return ERR_WRITE;

	return ERR_NOERROR;
}


///////////////////////////////////////////////////////////////////////////////
int CChessEngine::GetResponse(const std::vector<std::string>& expect)
{
	DWORD Bytes;
	DWORD BytesAvail;
	DWORD BytesLeft;
	DWORD cnt = 0;

	m_Response.clear();
	m_Response.resize(0x1000);

	while (true)
	{
		if (!PeekNamedPipe(m_pOut.r, (LPVOID)m_Response.data(), static_cast<DWORD>(m_Response.size()), &Bytes, &BytesAvail, &BytesLeft))
			return ERR_READ;

		if (BytesLeft == 0)
			break;

		if (Bytes > m_Response.size())
		{
			m_Response.resize(Bytes);
			continue;
		}

		if (!ReadFile(m_pOut.r, (LPVOID)m_Response.data(), static_cast<DWORD>(m_Response.size()), &Bytes, nullptr))
			return ERR_READ;

		if (expect.empty())
			break;

		for (const std::string s : expect) {
			if (m_Response.rfind(s) != std::string::npos)
				return ERR_NOERROR;
		}

		if (++cnt > 1000)
		{
			std::cerr << "Failed to get response";

		  if (!expect.empty())
				std::cerr << " for";

			for (const std::string& s : expect)
				std::cerr << " " << s;

		  std::cerr << std::endl;
		}
	}

	return ERR_NOERROR;
}


///////////////////////////////////////////////////////////////////////////////
int CChessEngine::GetMove(std::string& move)
{
	const std::string query = m_Position.empty() && !m_FirstMove.empty() ? " searchmoves " + m_FirstMove : "";

	CHECK_CALL(Query("position startpos moves " + m_Position + "\ngo" + query + "\n"));

	Sleep(m_TimePerMove);

	CHECK_CALL(GetResponse({ STR_BESTMOVE, STR_MATE }));

	size_t pos;

	if ((pos = m_Response.rfind(STR_BESTMOVE)) == std::string::npos)
	{
		if ((pos = m_Response.rfind(STR_MATE)) == std::string::npos)
		{
			std::cout << "Unable to find best move..." << std::endl;
			std::cout << m_Response.c_str() << std::endl;
			return ERR_BADMOVE;
		}
		m_Estimate = std::stoi(m_Response.c_str() + pos + sizeof(STR_MATE));
		return ERR_MATE;
	}

	pos += sizeof(STR_BESTMOVE);
	const size_t length = m_Response.find_first_of(' ', pos) - pos;
	move = m_Response.substr(pos, length);
	AddMove(move);

	if ((pos = m_Response.rfind(STR_SCORE)) == std::string::npos)
	{
		if ((pos = m_Response.rfind(STR_MATE)) == std::string::npos)
		{
			std::cout << "Unable to find score cp..." << std::endl;
			std::cout << m_Response.c_str() << std::endl;
			return ERR_BADMOVE;
		}
		m_Estimate = std::stoi(m_Response.c_str() + pos + sizeof(STR_MATE));
		return ERR_MATE;
	}

	m_Estimate = std::stoi(m_Response.c_str() + pos + sizeof(STR_SCORE));

	++m_Move;

	return ERR_NOERROR;
}
