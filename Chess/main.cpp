#include <SFML\Graphics.hpp>
#include <Windows.h>

#include <iostream>

#include "chess.h"
#include "engine.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "Winmm.lib")

void LoadPosition(std::vector<sf::Sprite>& sFigures)
{
	constexpr int Board[8][8] =
	{ {-1,-2,-3,-4,-5,-3,-2,-1},
	  {-6,-6,-6,-6,-6,-6,-6,-6},
	  {0, 0, 0, 0, 0, 0, 0, 0},
	  {0, 0, 0, 0, 0, 0, 0, 0},
	  {0, 0, 0, 0, 0, 0, 0, 0},
	  {0, 0, 0, 0, 0, 0, 0, 0},
	  {6, 6, 6, 6, 6, 6, 6, 6},
	  {1, 2, 3, 4, 5, 3, 2, 1} };

	size_t cnt = 0;

	for (unsigned int i = 0; i < 8; ++i)
	{
		for (unsigned int j = 0; j < 8; ++j)
		{
			int n = Board[i][j];

			if (n != 0)
			{
				int x = abs(n) - 1;
				int y = n > 0;
				constexpr int CellSize = 56;

				sFigures[cnt].setTextureRect(sf::IntRect(CellSize * x, CellSize * y, CellSize, CellSize));
				sFigures[cnt].setPosition(CellSize * j, CellSize * i);
				++cnt;
			}
		}
	}
}

int main()
{
	sf::Texture tBoard;
	if (!tBoard.loadFromFile("..\\images\\board.png"))
		return -1;

	sf::Texture tFigures;
	if (!tFigures.loadFromFile("..\\images\\figures.png"))
		return -2;

	sf::RenderWindow MainWindow(sf::VideoMode(tBoard.getSize().x + 200, tBoard.getSize().y + 200), "Chess");
	MainWindow.setFramerateLimit(60);

	sf::Sprite sBoard(tBoard);;
	std::vector<sf::Sprite> sFigures(32);

	for (auto& f : sFigures)
		f.setTexture(tFigures);

	LoadPosition(sFigures);

	int Status;
	CChessEngine Engine1(1000, true);
	const std::wstring EnginePath1 = L"..\\engines\\stockfish.exe";

	if ((Status = Engine1.Init(EnginePath1)) != ERR_NOERROR)
		return Status;

	const std::wstring EnginePath2 = L"..\\engines\\engine1.exe";
	CChessEngine Engine2(1000, false);

	if ((Status = Engine2.Init(EnginePath2)) != ERR_NOERROR)
		return Status;

	while (Engine1.GetMove() < 1000)
	{
		// white
		std::string move;
		if ((Status = Engine1.GetMove(move)) != ERR_NOERROR)
		{
			if (Status == ERR_MATE)
			{
				std::cout << "\nResign. Mate in " << Engine1.GetEstimate() << " moves" << std::endl;
				return ERR_NOERROR;
			}
			return Status;
		}
		Engine2.AddMove(move);
		std::cout << std::to_string(Engine1.GetMove()) << ". " << move << " (" << Engine1.GetEstimate() * 0.01 << ")";

		// black
		if ((Status = Engine2.GetMove(move)) != ERR_NOERROR)
		{
			if (Status == ERR_MATE)
			{
				std::cout << "\nResign. Mate in " << Engine2.GetEstimate() << " moves" << std::endl;
				return ERR_NOERROR;
			}
			return Status;
		}
		Engine1.AddMove(move);
		std::cout << " " << move << " (" << Engine2.GetEstimate() * 0.01 << ")" << std::endl;
	}

	std::cout << "Game is too long. Draw!" << std::endl;

	/*while (MainWindow.isOpen())
	{
		sf::Event e{};
		while (MainWindow.pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
				MainWindow.close();

			std::string move;
			if ((Status = Engine1.GetMove(move)) != ERR_NOERROR)
				return Status;
			Engine2.AddMove(move);

			if ((Status = Engine2.GetMove(move)) != ERR_NOERROR)
				return Status;
			Engine1.AddMove(move);

			MainWindow.clear();
			MainWindow.draw(sBoard);

			for (auto& f : sFigures)
			{
				sf::Vector2f V(28, 28);
				f.move(V);
				MainWindow.draw(f);
				f.move(-V);
			}

			MainWindow.display();
		}
	}*/

	return 0;
}