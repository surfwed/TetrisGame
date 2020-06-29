#include<iostream>
#include<Windows.h>
#include<thread>
#include<vector>




std::wstring tetromino[7];
int nFieldWidth = 8;
int nFieldHeight = 18;
int nScreenWidth = 80;
int nScreenHeight = 30;
unsigned char *pField;


int Rotate(int px, int py, int rt)
{
	switch (rt % 4)
	{
	case 0: return py * 4 + px;
	case 1: return 12 + py - (4 * px); 
	case 2: return rt = 15 - 4 * py - px; 
	case 3: return rt = 3 - py + 4 * px;
	}
	return 0;
}
bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
	for (int px = 0; px < 4; px++)
		for (int py = 0; py < 4; py++)
		{
			int index = Rotate(px, py, nRotation);
			int target = (nPosY + py)* nFieldWidth + (nPosX + px);
			if ( (nPosX + px >=0) && (nPosX + px < nFieldWidth) && (nPosY + py >=0) && (nPosY + py < nFieldHeight))
				if (tetromino[nTetromino][index] == L'X' && pField[target] != 0)
					return false;
		}
	return true;
}

int main()
{
	wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
	for (int i = 0; i < nScreenWidth*nScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	tetromino[0] = (L"..X...X...X...X."); // Tetronimos 4x4;
	tetromino[1] = (L"..X..XX...X.....");
	tetromino[2] = (L".....XX..XX.....");
	tetromino[3] = (L"..X..XX..X......");
	tetromino[4] = (L".X...XX...X.....");
	tetromino[5] = (L".X...X...XX.....");
	tetromino[6] = (L"..X...X..XX.....");
	pField = new unsigned char[nFieldWidth*nFieldHeight];
	// create field 
	for (int x = 0; x < nFieldWidth; x++)
		for (int y = 0; y < nFieldHeight; y++)
			pField[y*nFieldWidth + x] = ((x == 0) || (x == nFieldWidth - 1) || (y == nFieldHeight - 1)) ? 9 : 0;
	
	bool bGameOver = false;
	int nCurrentPiece = 3;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;
	bool bRotateHold = false;
	bool bKey[4];
	int nSpeed = 20;
	int nSpeedCounter = 0;
	bool bForceDown = false;
	std::vector<int> vLines;
	while (!bGameOver)
	{
		// Game Timing 		
		std::this_thread::sleep_for(std::chrono::milliseconds(70));
		nSpeedCounter++;
		bForceDown = nSpeedCounter == nSpeed;

		// Input
		for (int k = 0; k < 4; k++)
		{																		 //R   L   D	Z		
			bKey[k] = 0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]));
		}

		// Handle player movement
		nCurrentX += (bKey[0]) && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX+1, nCurrentY) ? 1 : 0;
		nCurrentX -= (bKey[1]) && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX-1, nCurrentY) ? 1 : 0;
		nCurrentY += (bKey[2]) && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY+1) ? 1 : 0;
		

		// Handle rotation
		if (bKey[3])
		{
			nCurrentRotation += (!bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation+1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHold = true;
		}
		else
		{
			bRotateHold = false;
		}
		// Game Logic
		if (bForceDown)
		{
			nSpeedCounter = 0;
			// Go down
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
			{
				nCurrentY++;
			}
			else
			{
				// Lock the current piece in the field
				for (int px = 0; px < 4; px++)
					for (int py = 0; py < 4; py++)
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
							pField[(nCurrentY + py)*nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
				// Check have we get any lines
				for (int py = 0; py < 4; py++)
				{
					if (nCurrentY + py < nFieldHeight - 1)
					{
						bool bLine = true;
						for (int px = 1; px < nFieldWidth - 1; px++)
							bLine &= pField[(nCurrentY + py)*nFieldWidth + px] !=0 ;

						if (bLine)
						{
							// Remove line, set equal 8 '='
							for (int px = 1; px < nFieldWidth - 1; px++)
								pField[(nCurrentY + py)*nFieldWidth + px] = 8;
							vLines.push_back(nCurrentY + py);
						}
					}
				}
				// chose next piece
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentPiece = rand() % 7;
				nCurrentRotation = 0;
				// if piece does not fit game over
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}
		}

		// Render Output

		// Draw field
		for (int px = 0; px < nFieldWidth; px++)
			for (int py = 0; py < nFieldHeight; py++)
			{
				screen[(2 + py)*nScreenWidth + (2 + px)] = L" ABCDEFG=#"[pField[py*nFieldWidth + px]];
			}

		// draw current piece
		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
					screen[(2 + py + nCurrentY) * nScreenWidth + (2 + px + nCurrentX)] = nCurrentPiece + 65;
		if (!vLines.empty())
		{
			//WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
			std::this_thread::sleep_for(std::chrono::milliseconds(400));
			for (auto v : vLines)
				for (int py = v; py > 0; py--)
					for (int px = 1; px < nFieldWidth - 1; px++)
					{
						pField[py*nFieldWidth + px] = pField[(py - 1)*nFieldWidth + px];						
						pField[px] = 0;
					}
			vLines.clear();
		}
		// Display Frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}
	
	CloseHandle(hConsole);
	system("pause");	
	return 0;
}