#include "../common/utils.hpp"

#include <include/iruntime.hpp>
#include <include/runtimefactory.hpp>

#include <cassert>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <thread>
#include <set>
#include <string>
#include <vector>


// ----------------------------------------------------------------------------


namespace boggle_game_test
{
  void GenerateBoard(const uint32_t width, const uint32_t height, std::vector<char>& out)
  {
    out.resize(width * height);

    for (auto& c : out)
    {
      c = (rand() % ('z' - 'a')) + 'a';
    }
  }


  void LoadBoard(const std::string& rootFolder, uint32_t& widthOut, uint32_t& heightOut, std::vector<char>& boardArrayOut)
  {
    ///
    /// Load a potential predefined board
    ///
    const std::string boardFilePath = rootFolder + "/board.txt";
    std::ifstream boardFileStream(boardFilePath, std::ifstream::binary);
    if (boardFileStream)
    {
      std::string currentLine;
      while (std::getline(boardFileStream, currentLine))
      {
        widthOut = 0;
        for (const char c : currentLine)
        {
          if (isalpha(c))
          {
            boardArrayOut.push_back(c);
            widthOut++;
          }
        }

        heightOut++;
      }
    }
    ///
    /// if there's no file named "board.txt", then generate the board instead ...
    ///
    else
    {
      std::cout << "Couldn't open board file: '"
        << boardFilePath
        << "'. Will generate a random one instead, and write it to disk." << std::endl;

      widthOut =  10000;
      heightOut = 10000;
      GenerateBoard(widthOut, heightOut, boardArrayOut);

      ///
      /// Dump the new board to disk to ensure we have consistent test data
      ///
      std::ofstream boardFileStreamOut(rootFolder + "/board.txt", std::ofstream::binary);
      if (boardFileStreamOut)
      {
        uint32_t currentX = 0;
        uint32_t currentY = 0;
        for (const char c : boardArrayOut)
        {
          boardFileStreamOut << c;

          currentX++;
          if (currentX == widthOut)
          {
            currentX = 0;
            currentY++;

            boardFileStreamOut << '\n';
          }
        }

        assert(currentX == 0);
        assert(currentY == heightOut);
      }
    }
  }


  void CheckResultAgainstExpectedFile(const std::string& rootFolder, const boggle_game::SBoggleResults& result)
  {
    const std::string filePath = rootFolder + "/expected_word_list.txt";
    std::ifstream fileStream(filePath, std::ifstream::binary);
    if (!fileStream)
    {
      std::cout << "Couldn't open expected result set: '"
        << filePath
        << "'. Will skip test verification and dump the found words to the expected file." << std::endl;


      std::ofstream expectedWordsFileStreamOut(rootFolder + "/expected_word_list.txt", std::ofstream::binary);
      if (expectedWordsFileStreamOut)
      {
        for (size_t i = 0; i < result.words.size(); i++)
        {
          expectedWordsFileStreamOut << result.words[i] << std::endl;
        }
      }

      return;
    }


    std::set<std::string> expectedWords;

    std::string currentWord;
    while (std::getline(fileStream, currentWord))
    {
      expectedWords.insert(currentWord);
    }

    ROTA_ASSERT(result.words.size() == expectedWords.size()  &&  "Expected word count failed");

    for (uint32_t i = 0; i < result.words.size(); i++)
    {
      const char* pCurrentWord = result.words[i];

      const bool foundWord = expectedWords.find(pCurrentWord) != expectedWords.cend();

      ROTA_ASSERT(foundWord  &&  "Didn't find word in the result set.");
    }

    std::cout << "Test passed!" << std::endl;
  }


  void RunTestFromDir(const std::string& testFolder)
  {
    std::cout << "Will execute test in: " << testFolder << std::endl;

    const auto timeStart = std::chrono::steady_clock::now();

    const std::string rootFolder
      = "bin/boggle_gamelib.test/testdata/" + testFolder;

    auto pRuntime = boggle_game::CreateRuntimeSolver(boggle_game::EBoggleSolver::TrieThreaded);

    ///
    /// Use the exposed function <LoadDictionary> to load our dictionary file ...
    ///
    pRuntime->LoadDictionary( std::string(rootFolder + "/dictionary.txt").c_str() );

    const auto timeAfterLoadingDictionary = std::chrono::steady_clock::now();

    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<char> boardArray;
    LoadBoard(rootFolder, width, height, boardArray);

    const auto timeAfterLoadingBoard = std::chrono::steady_clock::now();

    const auto result = pRuntime->FindWords(boardArray.data(), width, height);

    const auto timeEnd = std::chrono::steady_clock::now();

    std::cout
      << "Found " << result.words.size() << " words for a score of " << result.score << std::endl
      << "  Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeStart).count() << " ms" << std::endl
      << "  LoadDictionary : " << std::chrono::duration_cast<std::chrono::milliseconds>(timeAfterLoadingDictionary - timeStart).count() << " ms" << std::endl
      << "  LoadingBoard : " << std::chrono::duration_cast<std::chrono::milliseconds>(timeAfterLoadingBoard - timeAfterLoadingDictionary).count() << " ms" << std::endl
      << "  FindWords : " << std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeAfterLoadingBoard).count() << " ms" << std::endl
      ;

    CheckResultAgainstExpectedFile(rootFolder, result);

    std::this_thread::sleep_for( std::chrono::seconds(2) );
  }
}


// ----------------------------------------------------------------------------


int main()
{
  /// Regression tests
  boggle_game_test::RunTestFromDir("regression_qu1");
  boggle_game_test::RunTestFromDir("regression_qu2");
  boggle_game_test::RunTestFromDir("regression_ensure-non-duplicates");

  /// Performance test
  boggle_game_test::RunTestFromDir("performance_huge");
  boggle_game_test::RunTestFromDir("performance_monster");
  boggle_game_test::RunTestFromDir("performance_titan");
  boggle_game_test::RunTestFromDir("performance_titans-creator");


  return 0;
}
