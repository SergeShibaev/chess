#pragma once

constexpr int ERR_NOERROR    = 0;
constexpr int ERR_BADPIPE    = -1;
constexpr int ERR_BADPROCESS = -2;
constexpr int ERR_WRITE      = -3;
constexpr int ERR_READ       = -4;
constexpr int ERR_BADMOVE    = -5;
constexpr int ERR_MATE       = -6;
constexpr int ERR_DRAW	     = -7;

#define CHECK_CALL(c)			do{																\
														int Status = c;									\
														if (Status != ERR_NOERROR) {		\
															std::cerr << "Error " << Status << " in " << __func__ << ": " << __LINE__ << std::endl;	\
															return Status;								\
														}																\
													}while(0);

#define STR_BESTMOVE	"bestmove"
#define STR_SCORE			"score cp"
#define STR_MATE			"score mate"

struct CPipe
{
	HANDLE r;
	HANDLE w;

	CPipe()
		: r(INVALID_HANDLE_VALUE)
		, w(INVALID_HANDLE_VALUE)
	{}

	~CPipe()
	{
		if (r != INVALID_HANDLE_VALUE)
			CloseHandle(r);
		if (w != INVALID_HANDLE_VALUE)
			CloseHandle(w);
	}

	CPipe(const CPipe&) = delete;
	CPipe(const CPipe&&) = delete;
	CPipe& operator=(const CPipe&) = delete;
	CPipe& operator=(const CPipe&&) = delete;
};

struct CProcess
{
	PROCESS_INFORMATION m_Info;

	CProcess()
		: m_Info()
	{
		m_Info.dwThreadId = m_Info.dwProcessId = 0;
		m_Info.hThread = m_Info.hProcess = INVALID_HANDLE_VALUE;
	}

	~CProcess()
	{
		if (m_Info.hThread != INVALID_HANDLE_VALUE)
			CloseHandle(m_Info.hThread);
		if (m_Info.hProcess != INVALID_HANDLE_VALUE)
			CloseHandle(m_Info.hProcess);
	}

	CProcess(const CProcess&) = delete;
	CProcess(const CProcess&&) = delete;
	CProcess& operator=(const CProcess&) = delete;
	CProcess& operator=(const CProcess&&) = delete;
};
