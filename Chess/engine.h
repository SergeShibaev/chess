#pragma once

class CChessEngine
{
	CPipe					m_pIn;
	CPipe					m_pOut;
	CProcess			m_Proc;

	bool					m_bInited;
	std::string		m_Position;
	bool					m_bWhite;
	int						m_Estimate;
	size_t				m_Move;
	DWORD					m_TimePerMove;
public:
	CChessEngine(DWORD TimePerMove, bool bWhite);
	~CChessEngine();

	CChessEngine(const CChessEngine&) = delete;
	CChessEngine(const CChessEngine&&) = delete;
	CChessEngine& operator=(const CChessEngine&) = delete;
	CChessEngine& operator=(const CChessEngine&&) = delete;

	int Init(const std::wstring& path);
	int GetMove(std::string& move);
	size_t GetMove() const { return m_Move; }
	void AddMove(const std::string& move)
	{
		m_Position += move + " ";
	}
	int GetEstimate() const
	{
		return m_bWhite ? m_Estimate : -1 * m_Estimate;
	}
private:
	int Query(const std::string& query) const;
	int GetResponse(std::string& response, const std::vector<std::string>& expect = "") const;
};