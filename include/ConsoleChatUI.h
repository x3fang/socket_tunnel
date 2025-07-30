// ConsoleChatUI.h
#ifndef CONSOLECHATUI_H
#define CONSOLECHATUI_H

#include <iostream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <cstring>
#include <algorithm>
#include <windows.h>
#include <conio.h>
/*
													  showBegin     0
.                                                                             1
.                                                                            ....
.                                                                            ....
													  showEnd        n
prompt-----------------------editBegin       n+1
.                                                                             ..
.                                                                             ..
.                                                                             ..
													  editEnd           m
*/
class ConsoleChatUI
{
private:
	std::jthread inputThread;

	std::mutex screenLock;

	int terminalHeight = 0, terminalWidth = 0;

	std::vector<std::string> displayHistory;
	int displlayHistorylength = 0;
	int displayAreaBeginY = 0, displayAreaEndY = 0;
	int displayHistoryVectorInputIndex = 0;
	int displayAreaCursorY = 0;

	std::string prompt;
	std::vector<std::string> editingMessage;
	std::vector<std::vector<std::string>> completedEditMessage;
	int editBegin = 0, editEnd = 0;
	int editCursorX = 0, editCursorY = 0;
	int editingMessageNumY = 0; // Actual location in editingMessage
	int editEditingMessageVectorOutputBeginNum = 0, editEditingMessageVectorOutputBeginSection = 0;
	int editEditingMessageVectorOutputEndNum = 0, editEditingMessageVectorOutputEndSection = 0;

public:
	ConsoleChatUI() = default;
	~ConsoleChatUI() = default;
	void setPrompt(std::string prompt)
	{
		this->prompt = prompt;
		return;
	}
	void start()
	{
		auto terminalHeightWeight = get_terminal_size();
		terminalHeight = terminalHeightWeight.first;
		terminalWidth = terminalHeightWeight.second;

		if (terminalHeight % 2 != 0)
		{
			displayAreaEndY = (terminalHeight - 1) / 2 - 1;
			editBegin = (terminalHeight - 1) / 2;
		}
		else
		{
			displayAreaEndY = terminalHeight / 2 - 1;
			editBegin = terminalHeight / 2;
		}
		editEnd = terminalHeight;
		editCursorY = editBegin + 1;
		editingMessageNumY = 0;

		repaint(false, false, true, false, false);

		this->editingMessage.push_back("");
		inputThread = std::jthread(&ConsoleChatUI::inputFunc, this);
		return;
	}
	void outputFunc(std::string outputMessage, bool saveHistory = true, bool screenLocked = false);
	std::vector<std::string> getUserInputByOnce()
	{
		if (this->completedEditMessage.empty())
			return std::vector<std::string>();
		else
		{
			auto temp = this->completedEditMessage.front();
			this->completedEditMessage.erase(this->completedEditMessage.begin());
			return temp;
		}
	}
	std::vector<std::vector<std::string>> getUserInputByALL()
	{
		if (this->completedEditMessage.empty())
			return std::vector<std::vector<std::string>>();
		else
		{
			auto& temp = this->completedEditMessage;
			this->completedEditMessage.clear();
			return temp;
		}
	}
	void stop()
	{
		system("cls");
		inputThread.join();
		return;
	}

private:
	static std::pair<int /*rows*/, int /*cols*/> get_terminal_size()
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		int term_rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
		int term_cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		return { term_rows, term_cols };
	}
	static inline void moveCursor(int x, int y)
	{
		COORD coord = { 0 };
		coord.X = x;
		coord.Y = y - 1;
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
	}

	void repaint(bool displayArea = false, bool editArea = false, bool allArea = false, bool lock_screenLock = true, bool partialRepaint = true, bool onlyClear = false);
	void repaintDisplayArea(bool lock_screenLock = true, bool partialRepaint = true);
	void repaintEditArea(bool lock_screenLock = true, bool partialRepaint = true, bool onlyClear = false);
	void repaintAllArea(bool lock_screenLock = true, bool partialRepaint = true, bool onlyClear = false);

	void inputFunc();
	void inputFunc_switch1_case_r_case_n();
	void inputFunc_switch1_case_b();
	void inputFunc_switch1_case_default(char inputChar);
	void inputFunc_switch1_case_32(char inputChar);
};
void ConsoleChatUI::repaint(bool displayArea, bool editArea, bool allArea, bool lock_screenLock, bool partialRepaint, bool onlyClear)
{
	if (displayArea)
	{
		this->repaintDisplayArea(lock_screenLock, partialRepaint);
	}
	if (editArea)
	{
		this->repaintEditArea(lock_screenLock, partialRepaint, onlyClear);
	}
	if (allArea)
	{
		this->repaintAllArea(lock_screenLock, partialRepaint, onlyClear);
	}
	return;
}
// the partialRepaint hasn't meaning
void ConsoleChatUI::repaintDisplayArea(bool lock_screenLock, bool partialRepaint)
{
	if (!lock_screenLock)
		this->screenLock.lock();
	moveCursor(0, this->displayAreaCursorY + 1);
	if (this->displlayHistorylength - this->displayHistoryVectorInputIndex >= this->displayAreaEndY - this->displayAreaBeginY)
	{
		std::cout << "\033[K"; // clear the line
		moveCursor(0, this->terminalHeight);
		std::cout << std::endl;
		repaintEditArea(true, false, false);
	}
	if (!lock_screenLock)
		screenLock.unlock();
	return;
}
void ConsoleChatUI::repaintEditArea(bool lock_screenLock, bool partialRepaint, bool onlyClear)
{
	if (!lock_screenLock)
		screenLock.lock();
	moveCursor(0, this->editBegin);
	if (!onlyClear)
	{
		std::cout << this->prompt;
		for (int i = 0; i < terminalWidth - this->prompt.length(); i++)
		{
			std::cout << "-";
		}
	}

	moveCursor(0, this->editBegin + 1);
	for (int i = editEditingMessageVectorOutputBeginNum; i < this->editEditingMessageVectorOutputEndNum && this->editingMessage.size() > 0; i++)
	{
		std::cout << "\033[K" << std::endl;
	}
	std::cout << "\033[K";
	if (onlyClear)
	{
		if (!lock_screenLock)
			screenLock.unlock();
		return;
	}

	// paint edit Area
	moveCursor(0, this->editBegin + 1);
	for (int i = editEditingMessageVectorOutputBeginNum; i <= this->editEditingMessageVectorOutputEndNum && this->editingMessage.size() > 0; i++)
	{
		std::cout << this->editingMessage[i];
		if (i != this->editEditingMessageVectorOutputEndNum)
		{
			std::cout << std::endl;
		}
	}
	if (!lock_screenLock)
		screenLock.unlock();
	return;
}
void ConsoleChatUI::repaintAllArea(bool lock_screenLock, bool partialRepaint, bool onlyClear)
{
	this->repaintDisplayArea(lock_screenLock, partialRepaint);
	this->repaintEditArea(lock_screenLock, partialRepaint, onlyClear);
	return;
}
void ConsoleChatUI::inputFunc_switch1_case_r_case_n()
{
	if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) == 0 && !this->editingMessage.empty()) // No press shift ,send message
	{
		this->completedEditMessage.push_back(this->editingMessage);
		this->editingMessage.clear();
		this->editingMessage.push_back("");
		this->editCursorX = 0;
		this->editCursorY = this->editBegin + 1;
		this->editingMessageNumY = 0;
		this->repaintEditArea(true, false, true);
		this->editEditingMessageVectorOutputBeginNum = 0;
		this->editEditingMessageVectorOutputBeginSection = 0;
		this->editEditingMessageVectorOutputEndNum = 0;
		this->editEditingMessageVectorOutputEndSection = 0;
	}
	else // newline
	{
		if (this->editCursorX != 0)
		{
			std::string temp = this->editingMessage[this->editingMessageNumY].substr(this->editCursorX);
			this->editingMessage[this->editingMessageNumY] = this->editingMessage[this->editingMessageNumY].substr(0, this->editCursorX);
			this->editingMessage.insert(this->editingMessage.begin() + this->editingMessageNumY + 1, temp);
		}
		else
		{
			if (this->editingMessage.begin() + this->editingMessageNumY + 1 < this->editingMessage.end())
				this->editingMessage.insert(this->editingMessage.begin() + this->editingMessageNumY + 1, "");
			else
				this->editingMessage.push_back("");
		}
		if (this->editCursorY >= this->editEnd)
		{
			this->editEditingMessageVectorOutputBeginNum++;
		}
		if (this->editCursorY < this->editEnd)
		{
			this->editCursorY++;
		}
		this->editingMessageNumY++;
		this->editEditingMessageVectorOutputEndNum++;
		this->editCursorX = 0;
		this->repaintEditArea(true, true, false);
	}
	return;
}
void ConsoleChatUI::inputFunc_switch1_case_b()
{
	if (this->editCursorX != 0)
	{
		this->editingMessage[this->editingMessageNumY].erase(this->editingMessage[this->editingMessageNumY].begin() + this->editCursorX - 1);
		this->editCursorX--;
	}
	else if (this->editingMessageNumY != 0)
	{
		this->editingMessage.erase(this->editingMessage.begin() + this->editingMessageNumY);
		this->editCursorY--;
		this->editingMessageNumY--;
		this->editCursorX = static_cast<int>(this->editingMessage[this->editingMessageNumY].size());
		if (this->editEditingMessageVectorOutputBeginNum != 0)
			this->editEditingMessageVectorOutputBeginNum--;
		this->editEditingMessageVectorOutputEndNum--;
	}
	this->repaintEditArea(true, false, true);
	return;
}
void ConsoleChatUI::inputFunc_switch1_case_default(char inputChar)
{
	if (inputChar >= 32 && inputChar <= 126) // Printable characters
	{
		if (this->editingMessage[this->editingMessageNumY].size() + 1 > terminalWidth) // new line and add the last char
		{
			auto insertDatait = this->editingMessage.insert(this->editingMessage.begin() + this->editingMessageNumY + 1, "");
			insertDatait->push_back(this->editingMessage[this->editingMessageNumY].back());
			this->editingMessage[this->editingMessageNumY].pop_back();
			this->editingMessage[this->editingMessageNumY].insert(this->editingMessage[this->editingMessageNumY].begin() + this->editCursorX - 1, inputChar);
			if (this->editCursorX + 1 >= terminalWidth)
			{
				this->editCursorX = 1;
				if (this->editCursorY == this->editEnd)
				{
					this->editEditingMessageVectorOutputBeginNum++;
					editCursorY--;
				}
				this->editCursorY++;
				this->editingMessageNumY++;
			}
			this->editEditingMessageVectorOutputEndNum++;
		}
		else
		{
			this->editingMessage[this->editingMessageNumY].insert(this->editingMessage[this->editingMessageNumY].begin() + this->editCursorX, inputChar);
			this->editCursorX++;
		}
		this->repaintEditArea(true, true, false);
	}
	return;
}
void ConsoleChatUI::inputFunc_switch1_case_32(char inputChar)
{
	switch (inputChar)
	{
	case 72: // up
		if (this->editingMessageNumY > 0)
		{

			if (this->editCursorX > this->editingMessage[this->editingMessageNumY].size())
				this->editCursorX = static_cast<int>(this->editingMessage[this->editingMessageNumY].size());
			if (this->editingMessageNumY <= this->editEditingMessageVectorOutputBeginNum)
			{
				this->editEditingMessageVectorOutputBeginNum--;
				this->editEditingMessageVectorOutputEndNum--;
			}
			else
				this->editCursorY--;
			this->editingMessageNumY--;
		}
		break;
	case 80: // down
		if (this->editingMessageNumY < this->editingMessage.size() - 1)
		{
			this->editingMessageNumY++;
			if (this->editCursorX > this->editingMessage[this->editingMessageNumY].size())
				this->editCursorX = static_cast<int>(this->editingMessage[this->editingMessageNumY].size());
			if (this->editCursorY == this->editEnd)
			{
				this->editEditingMessageVectorOutputBeginNum++;
				this->editEditingMessageVectorOutputEndNum++;
			}
			else
				this->editCursorY++;
		}
		break;
	case 75: // left
		if (this->editCursorX > 0)
		{
			this->editCursorX--;
		}
		else if (this->editingMessageNumY > 0)
		{
			this->inputFunc_switch1_case_32(72);
			this->editCursorX = static_cast<int>(this->editingMessage[this->editingMessageNumY].size());
			return;
		}
		break;
	case 77: // right
		if (this->editCursorX < this->editingMessage[this->editingMessageNumY].size())
		{
			this->editCursorX++;
		}
		else if (this->editingMessageNumY < this->editingMessage.size() - 1)
		{
			this->inputFunc_switch1_case_32(80);
			this->editCursorX = 0;
			return;
		}
		break;
	}
	this->repaintEditArea(true, false, false);
	return;
}
void ConsoleChatUI::inputFunc()
{
	char inputChar;
	while (true)
	{
		if (_kbhit())
		{
			screenLock.lock();
			inputChar = _getch();
			switch (inputChar)
			{
			case '\r':
			case '\n':
				this->inputFunc_switch1_case_r_case_n();
				break;
			case '\b':
				this->inputFunc_switch1_case_b();
				break;
			case -32: // directional keys
				inputChar = _getch();
				this->inputFunc_switch1_case_32(inputChar);
				break;
			default:
				this->inputFunc_switch1_case_default(inputChar);
				break;
			}
			moveCursor(this->editCursorX, this->editCursorY);
			this->screenLock.unlock();
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
}
void ConsoleChatUI::outputFunc(std::string outputMessage, bool saveHistory, bool screenLocked)
{
	if (saveHistory)
		this->displayHistory.push_back(outputMessage);
	if (!screenLocked)
		this->screenLock.lock();
	this->displlayHistorylength++;
	if (outputMessage.find_first_of("\n") != std::string::npos)
	{
		while (outputMessage.find_first_of("\n") != std::string::npos)
		{
			this->outputFunc(outputMessage.substr(0, outputMessage.find_first_of("\n")), false, true);
			if (outputMessage.find_first_of("\n") == outputMessage.size() - 1)
			{
				this->outputFunc("", false, true);
				outputMessage.clear();
				break;
			}
			outputMessage = outputMessage.substr(outputMessage.find_first_of("\n") + 1);
		}
		if (outputMessage.empty())
		{
			if (!screenLocked)
				screenLock.unlock();
			moveCursor(this->editCursorX, this->editCursorY);
			return;
		}
	}
	if (outputMessage.length() > this->terminalWidth)
	{
		while (outputMessage.length() > this->terminalWidth)
		{
			this->outputFunc(outputMessage.substr(0, this->terminalWidth), false, true);
			outputMessage = outputMessage.substr(this->terminalWidth);
		}
		this->outputFunc(outputMessage, false, true);
	}
	else if (this->displayAreaCursorY >= this->displayAreaEndY)
	{
		this->repaintDisplayArea(true, true);
		moveCursor(0, this->displayAreaCursorY);
		std::cout << outputMessage << std::endl;
	}
	else
	{
		moveCursor(0, this->displayAreaCursorY + 1);
		std::cout << outputMessage << std::endl;
		this->displayAreaCursorY++;
	}
	if (!screenLocked)
		this->screenLock.unlock();
	moveCursor(this->editCursorX, this->editCursorY);
	return;
}
#endif // CONSOLECHATUI_H