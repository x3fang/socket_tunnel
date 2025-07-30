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
#define EDIT_MAX_HEIGHT 10
class ConsoleChatUI
{
private:
      std::jthread inputThread;
      std::string prompt;
      std::vector<std::pair<bool /*true means user sent*/, std::vector<std::string>>> sentMessages;
      std::vector<std::vector<std::string>> finishedInput;
      std::vector<std::string> OutputWords;
      std::vector<std::string> editVector;
      std::mutex screenLock;
      std::mutex finishedInputVectorLock;
      int editHeight = 0, showHeight = 0;
      int termRows = 0, termCols = 0;

      int showY = 0;

      int editBeginX = 0, editBeginY = 0;
      int editX = 0, editY = 0;
      int editEndX = 0, editEndY = 0;
      int editVectorBeginNum = 0, editVectorEndNum = 0;

      int showVectorBeginNum = 0;

      bool stopSign = false;

public:
      ConsoleChatUI() = default;
      ~ConsoleChatUI() = default;
      void start();
      void stop();
      void outputline(std::string outputMessage, bool saveHistory = true, bool screenLocked = false);
      std::string getUserInputByString()
      {
            std::string ret;
            finishedInputVectorLock.lock();
            if (!finishedInput.empty())
            {
                  if (finishedInput.front().empty())
                        ret = "";
                  else
                  {
                        ret = finishedInput.front().front();
                        finishedInput.front().erase(finishedInput.front().begin());
                  }
            }
            finishedInputVectorLock.unlock();
            return ret;
      }
      int setPrompt(std::string prompt)
      {
            if (prompt.length() > termCols)
            {
                  this->prompt = prompt.substr(0, termCols);
            }
            this->prompt = prompt;
            return this->prompt.length();
      }

      std::vector<std::string> getUserInputByVector()
      {
            if (finishedInput.empty())
                  return std::vector<std::string>();
            finishedInputVectorLock.lock();
            auto ret = finishedInput.front();
            finishedInput.erase(finishedInput.begin());
            finishedInputVectorLock.unlock();
            return ret;
      }

private:
      // std::pair<int ->rows  , int ->cols  >
      std::pair<int /*rows*/, int /*cols*/> get_terminal_size()
      {
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
            int term_rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
            int term_cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            return {term_rows, term_cols};
      }
      void inputline();
      void display(bool showDown = false, bool editDown = false, bool all = false, bool locked = false);
      inline void moveCursor(int x, int y)
      {
            COORD coord;
            coord.X = x;
            coord.Y = y;
            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
      }
};
void ConsoleChatUI::start()
{
      std::pair<int, int> term_size = get_terminal_size();
      termRows = term_size.first;
      termCols = term_size.second;

      editBeginX = 0;
      if (termRows % 2 != 0)
      {
            editBeginY = (termRows - 1) / 2;
            showHeight = (termRows + 1) / 2;
      }
      else
      {
            editBeginY = termRows / 2;
            showHeight = termRows / 2;
      }

      editEndX = termCols;
      editEndY = termRows;
      editX = editBeginX;
      editY = editBeginY + 1;

      display(false, false, true);
      inputThread = std::jthread(&ConsoleChatUI::inputline, this);
}
void ConsoleChatUI::stop()
{
      stopSign = true;
      system("pause");
      system("cls");
      return;
}
void ConsoleChatUI::outputline(std::string outputMessage, bool saveHistory, bool screenLocked)
{
      if (saveHistory)
            OutputWords.push_back(outputMessage);
      if (!screenLocked)
            screenLock.lock();
      moveCursor(0, showY);
      if (showY >= showHeight)
      {
            if (outputMessage.find_first_of("\n") != std::string::npos)
            {
                  while (outputMessage.find_first_of("\n") != std::string::npos)
                  {
                        outputline(outputMessage.substr(0, outputMessage.find_first_of("\n")), false, true);
                        outputMessage = outputMessage.substr(outputMessage.find_first_of("\n") + 1);
                  }
                  if (outputMessage.empty())
                  {
                        if (!screenLocked)
                              screenLock.unlock();
                        return;
                  }
            }
            if (outputMessage.length() > termCols)
            {
                  while (outputMessage.length() > termCols)
                  {
                        outputline(outputMessage.substr(0, termCols), false, true);
                        outputMessage = outputMessage.substr(termCols);
                  }
                  outputline(outputMessage, false, true);
            }
            else
            {
                  ++showVectorBeginNum;
                  display(true, false, false, true);
                  moveCursor(0, showY - 1);
                  std::cout << outputMessage << std::endl;
            }
            moveCursor(editX, editY);
      }
      else
      {
            if (outputMessage.find_first_of("\n") != std::string::npos)
            {
                  while (outputMessage.find_first_of("\n") != std::string::npos)
                  {
                        outputline(outputMessage.substr(0, outputMessage.find_first_of("\n")), false, true);
                        outputMessage = outputMessage.substr(outputMessage.find_first_of("\n") + 1);
                  }
                  if (outputMessage.empty())
                  {
                        if (!screenLocked)
                              screenLock.unlock();
                        return;
                  }
            }
            if (outputMessage.length() > termCols)
            {
                  while (outputMessage.length() > termCols)
                  {
                        outputline(outputMessage.substr(0, termCols), false, true);
                        outputMessage = outputMessage.substr(termCols);
                  }
                  outputline(outputMessage, false, true);
            }
            else
            {
                  moveCursor(0, showY);
                  std::cout << outputMessage << std::endl;
                  showY++;
            }
            moveCursor(editX, editY);
      }
      if (!screenLocked)
            screenLock.unlock();
      return;
}
void ConsoleChatUI::inputline()
{
      bool DownShift = false;
      char inputChar;
      while (!stopSign)
      {
            if (_kbhit() != 0)
            {
                  inputChar = getch();
                  screenLock.lock();
                  switch (inputChar)
                  {
                  case '\r':
                  case '\n':

                        if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) == 0)
                        {
                              finishedInputVectorLock.lock();
                              finishedInput.push_back(editVector);
                              finishedInputVectorLock.unlock();
                              editVector.clear();

                              display(false, true, false, true);
                              editX = editBeginX;
                              editY = editBeginY + 1;
                              editVectorEndNum = 0;
                              editVectorBeginNum = 0;

                              break;
                        }
                        editVector.push_back(std::string(""));
                        editX = 0;
                        editY++;
                        editVectorEndNum++;
                        if (editVectorEndNum - editVectorBeginNum >= editEndY - editBeginY - 2 || editY >= editEndY - 1)
                        {
                              editVectorBeginNum += 1;
                              editY--;
                              display(false, true, false, true);
                        }
                        else
                              std::cout << std::endl;
                        break;
                  case 8:
                  case 127:
                        if (editX == 0 && editY - 2 >= editBeginY)
                        {
                              editX = termCols;
                              editY--;
                              if (editVector.size() == 0)
                                    editVector.push_back(std::string(""));
                              if (editVector.back().empty())
                              {
                                    editVector.pop_back();
                                    editX = 0;
                                    moveCursor(editX, editY);
                                    editVectorEndNum--;
                              }
                              else
                              {
                                    editVector.back().pop_back();
                              }
                        }
                        else if (editX > 0)
                        {
                              if (editVector.size() == 0)
                                    editVector.push_back(std::string(""));
                              editVector.back().pop_back();
                              editX--;
                        }
                        else
                              break;
                        display(false, true, false, true);
                        break;
                  case -32: // arrow key
                        inputChar = getch();
                        switch (inputChar)
                        {
                        case 75: // left
                              if (editX > 0)
                              {
                                    editX--;
                              }
                              else if (editX == 0)
                              {
                                    if (editY > editBeginY + 1)
                                    {
                                          editY--;
                                          editX = editVector[editY - editBeginY - 1].size();
                                    }
                              }
                              break;
                        case 77: // right
                              if (editX < editVector[editY - editBeginY - 1].size())
                              {
                                    editX++;
                              }
                              else if (editX == editVector[editY - editBeginY - 1].size())
                              {
                                    if (editY - editBeginY - 1 < editVector.size())
                                    {
                                          editY++;
                                          editX = 0;
                                    }
                              }
                              break;
                        case 72: // up
                              if (editY > editBeginY + 1)
                              {
                                    editY--;
                              }
                              if (editX > editVector[editY - editBeginY - 1].size())
                              {
                                    editX = editVector[editY - editBeginY - 1].size();
                              }
                              break;
                        case 80: // down
                              if (editY < editEndY)
                              {
                                    editY++;
                              }
                              if (editX > editVector[editY - editBeginY - 1].size())
                              {
                                    editX = editVector[editY - editBeginY - 1].size();
                              }
                              break;
                        }
                        moveCursor(editX, editY);
                        display(false, true, false, true);
                        break;
                  default:
                        if (editX + 1 > termCols)
                        {
                              if (editVector.size() == 0 || editVector.size() < editY - editBeginY)
                                    editVector.push_back(std::string(""));
                              editX = 0;
                              editY++;
                              editVector[editY - editBeginY - 1].insert(editVector[editY - editBeginY - 1].begin() + editX - editBeginX, inputChar);
                              editX++;

                              if (editY >= editEndY - 1)
                              {
                                    editVectorBeginNum += 1;
                                    display(false, true, false, true);
                                    editY--;
                                    std::cout << inputChar;
                              }
                              else
                                    std::cout << std::endl
                                              << inputChar;
                        }
                        else if (inputChar != '\r')
                        {
                              if (editVector.size() == 0 || editVector.size() < editY - editBeginY - 1)
                                    editVector.push_back(std::string(""));
                              editVector[editY - editBeginY - 1].insert(editVector[editY - editBeginY - 1].begin() + editX - editBeginX, inputChar);
                              editX++;
                              std::cout << inputChar;
                        }
                  }
                  screenLock.unlock();
            }
            else
                  std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      std::cout << "input thread exit";
      return;
}
void ConsoleChatUI::display(bool showDown, bool editDown, bool all, bool locked)
{
      if (!locked)
            screenLock.lock();
      if (showDown)
      {
            moveCursor(0, showY);
            for (int i = 0; i < editEndY - editBeginY; i++)
                  std::cout << "\r\033[K\r\n";

            display(false, true, false, true);
            moveCursor(0, editBeginY - 1);
      }
      if (editDown)
      {
            moveCursor(0, editBeginY);
            std::cout << this->prompt;
            for (int i = 0; i < termCols - this->prompt.length(); i++)
                  std::cout << "-";
            std::cout << std::endl;
            for (int i = 0; i < editY - editBeginY; i++)
                  std::cout << "\r\033[K\r\n";
            moveCursor(0, editBeginY + 1);
            for (int i = editVectorBeginNum; i < editVectorEndNum && editVector.size() > 0; i++)
            {
                  std::cout << editVector[i] << std::endl;
            }
            if (editVector.size() > 0)
                  std::cout << editVector[editVectorEndNum];
      }
      if (all)
      {
            std::cout << "\033[2J";
            display(true, true, false, true);
      }
      if (!locked)
            screenLock.unlock();
}
#endif // CONSOLECHATUI_H