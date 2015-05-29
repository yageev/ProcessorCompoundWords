#include "stdafx.h"
#include "Words.h"
#include <mutex>
#include <algorithm>
#include <concurrent_vector.h>

using namespace std;
using namespace concurrency;
//
// Requirements for the processor of Compound Words Search:
//
//1. 1st longest word in the file that can be constructed by concatenating copies of shorter
//   words also found in the file. 
//2. The program should then go on to report the 2nd longest word found 
//3. Total count of how many of the words in the list can be constructed of other words 
//   in the list.

// Multi-threading, Multi-threading every where Multi-threading

// Original input benchmark file contains 232,000 words; one word - one line
// The file located in
string WORDS_FILE = "..\\wordsforproblem.txt";

//Test short list of words contains 10 words. It uses for test the algorithm. 
//The longest word is 'ratcatdogcat' - at 12 letters
//string WORDS_FILE = "..\\_wordsforproblem.txt"; //short list

// Constructor
Words::Words(const char *filename) : fileName(filename), foundCompoundWords(0)
{
	for (int i = 0; i < MAX_BITS; ++i)
	{
		bitsVector.push_back(bitset<MAX_BITS>(i));
	}
}

bool Words::fileExists(const char *fileName)
{
	ifstream infile(fileName);
	return infile.good();
}

bool Words::isWordExist(string s)
{
	return (wordSet.find(s) != wordSet.end());
}

void Words::LoadChank(int n)
{
	string s;
	while (getline(_chanks[n], s))
	{
		if (!s.empty() && s[0] != ' ')
		{
			words.push_back(s);
			wordSet.insert(s);
		}
	}
}
//
// Optimization of loading data is based on multi-threading execution
// The loading processes data from a single file
//
bool Words::LoadFile()
{
	string line;
	if (!fileExists(fileName.c_str()))
	{
		cerr << "File " + fileName + " not found";
		return false;
	}

	auto start = chrono::high_resolution_clock::now();

	ifstream _instream;
	vector<size_t> _ChankPoint(COUNT_THREADS);
	vector<size_t> _ChankSize(COUNT_THREADS);

	_instream.open(fileName, ios_base::in);
	_instream.seekg(0, ios::end);
	unsigned size = (unsigned)_instream.tellg();
	string buffer(size, ' ');
	_instream.seekg(0, ios::beg);
	_instream.read(&buffer[0], size);
	_instream.seekg(0, ios::beg);
	size_t chankSize = size / COUNT_THREADS;
	_ChankPoint[0] = 0;
	string _buf;
	size_t pos;
	for (int i = 1; i < COUNT_THREADS; i++)
	{
			pos = _ChankPoint[i - 1] + chankSize;
			for (; buffer[pos] != '\n' && buffer[pos] != NULL; pos++)
				;
			pos++;
		_ChankPoint[i] = pos;
		_ChankSize[i - 1] = _ChankPoint[i] - _ChankPoint[i - 1];
	}
	_ChankSize[COUNT_THREADS - 1] = size - _ChankPoint[COUNT_THREADS - 1];
	_instream.close();

	for (int i = 0; i < COUNT_THREADS; i++)
	{
		_chanks[i].write(&buffer[_ChankPoint[i]], _ChankSize[i]);
	}

	vector<thread> threads;
	for (int i = 0; i < COUNT_THREADS; i++)
	{
		threads.push_back(thread(&Words::LoadChank, this, i));
	}

	for (auto& th : threads)
	{
		th.join();
	}

	Chronometr(start, "LoadFile");
	start = std::chrono::high_resolution_clock::now();

	countWords = words.size();

	sort(words.begin(), words.end(),
		[](const string& a, const string& b)
	{
		return a.length() > b.length();
	});

	maxLength = words[0].size();

	Chronometr(start, "Sort vector");
	return true;
}

void Words::Chronometr(chrono::time_point<chrono::system_clock, chrono::system_clock::duration> start, string message)
{
	auto end = std::chrono::high_resolution_clock::now();
	chrono::duration<double, std::milli> elapsed = end - start;
	typedef std::chrono::milliseconds ms;
	ms d = chrono::duration_cast<ms>(elapsed);
	cout << message << ": " << d.count() << " ms\n";
}

bool Words::CheckWords(string w1, string w2, int indexCurrent) //w1 substr
{
	vector<int> _indexes;
	vector<int> _posWords;
	vector<string> _words;

	int len1 = w1.length();
	int len2 = w2.length();
	int n = (len2 - len1) - MIN_WORD_LENGTH;
	if (n < 0) return false;

	string::size_type posStart = 0;
	int countWords = (maxLength / MIN_WORD_LENGTH) + 1;

	int count = 0;
	_posWords.clear();
	while ((posStart = w2.find(w1, posStart)) != string::npos)
	{
		if ((posStart >= MIN_WORD_LENGTH && (len2 - (posStart + len1) ) >= MIN_WORD_LENGTH) ||
			posStart == 0 || (len2 - (posStart + len1)) == 0)
		{
			_posWords.push_back(posStart);
			count++;
		}
		posStart = posStart + len1;
	}
	if (count == 0) 
		return false;

	bool found = true;
	int posSize = (int)_posWords.size();
	int m = (int)pow(posSize, 2);
	for (int j = 1; j <= m; ++j)
	{
		bitset<MAX_BITS> b = bitsVector[j];
		_words.clear();
		_indexes.clear();
		for (int i = 0; i < posSize; i++)
		{
			if (b[i])
				_indexes.push_back(_posWords[i]);
		}

		int sizeIndexes = _indexes.size();
		found = sizeIndexes > 0;
		for (int n = 0; n < sizeIndexes; n++)
		{
			int i1, lenSub1;

			int k = _indexes[n];
			if (n == 0 && k != 0)
			{
				i1 = 0;
				lenSub1 = k;
				string s = w2.substr(i1, lenSub1);
				if (isWordExist(s))
				{
					_words.push_back(s);
				}
				else
				{
					found = false;
					break;
				}
			}
			else if (len2 == (k + len1))
			{
				break;
			}
			i1 = k + len1;
			lenSub1 = (n < sizeIndexes - 1) ? _indexes[n + 1] - k - len1
				                            : len2 - i1;
			string s = w2.substr(i1, lenSub1);
			if (isWordExist(w2.substr(i1, lenSub1)))
			{
				_words.push_back(s);
			}
			else
			{
				found = false;
				break;
			}
		}

		if (found)
		{
			AddTrace(indexCurrent, w1);
			for (int i = 0; i < (int)_words.size(); i++)
			{
				AddTrace(indexCurrent, _words[i]);
			}
			break;
		}
	}
	return found;
}

void Words::AddTrace(int indexTrace, string subword)
{
	map<int, vector<string>>::iterator foundValue = traceWords.find(indexTrace);
	if (foundValue == traceWords.end())
	{
		vector<string> v = { subword };
		traceWords.insert({ indexTrace, v });
	}
	else
	{
		traceWords[indexTrace].push_back(subword);
	}
}

void Words::PrintLongesWords()
{
//	typedef pair<string, vector<int>> TypeTracePair;
	typedef map<int, vector<string>> TypeTraceMap;

	if (!traceWords.size())
	{
		cout << "No found." << endl;
		return;
	}
	int n = 1;
	for (TypeTraceMap::iterator p = traceWords.begin(); p != traceWords.end(); ++p)
	{
		int idx = p->first;
		vector<string> v = p->second;
		cout << "The longest compound word #" << n++ << " : " << words[idx] << " length=" << words[idx].length() << endl;
		for (int i = 0; i < (int)v.size(); i++)
		{
			cout << '\t' << v[i] << endl;
		}
	}
}

bool Words::CheckOut(int idx1, int idx_start, int idx_finish)
{
	int indexCurrent = idx1;
	int indexStart = idx_start; 
	int till = idx_finish; 
	string w = words[indexCurrent];
	while (indexStart < countWords && indexStart < till && foundCompoundWords < 2)
	{
		if (CheckWords(words[indexStart], w, indexCurrent))
		{
			foundCompoundWords++;
			indexCurrent++;
			indexStart = indexCurrent;
			w = words[indexCurrent];
			continue;
		}

		indexStart++;
	}
	return foundCompoundWords == 2;
}

//
// Optimization of searching compound words with the multi-threading approache
//
void Words::FindWord()
{
	int indexCurrent = 0;
	int indexStart = 1;

	int STEP = countWords / (COUNT_THREADS * 2);
	int indexFinish = STEP;

	bool init_tread = false;
	int move_to_next_word = 0;
	int next_step = 0;

	vector<thread> threads;

	while (indexCurrent < (countWords - 1) && foundCompoundWords < 2)
	{
		if (!init_tread)
		{
			init_tread = true;
			for (int i = 0; i < COUNT_THREADS; i++)
			{
				int idxfinish = (i + 1)*STEP;
				idxfinish = (idxfinish > countWords) ? countWords : idxfinish;
				threads.push_back(thread(&Words::CheckOut, this, indexCurrent, i*STEP, idxfinish));
			}
		}
		for (auto& th : threads)
		{
			th.join();

			if (foundCompoundWords >= 2)
				continue;

			if (move_to_next_word == 0)
			{
				indexCurrent++;
			}
			indexStart = indexCurrent + 1 + STEP * move_to_next_word;
			indexFinish = indexCurrent + STEP * (1+move_to_next_word);
			indexFinish = (indexFinish > countWords) ? countWords : indexFinish;

			if (indexFinish >= countWords)
			{
				move_to_next_word = 0;
			}
			else
			{
				move_to_next_word++;
				if (move_to_next_word == STEP)
					move_to_next_word = 0;
			}

			th = thread(&Words::CheckOut, this, indexCurrent, indexStart, indexFinish);
			break;
		}

	}
	if (foundCompoundWords == 0) //Is any compound word?
	{
		cout << "There are no compound words." << endl;
	}
}

int main(int argc, char* argv[])
{
	if (argc == 2)
	{
		WORDS_FILE = argv[1];
	}
	cout << "Start" << endl;
	auto start = std::chrono::high_resolution_clock::now();

	Words *_words = new Words(WORDS_FILE.c_str());

	if (_words->LoadFile())
	{
		auto _start = std::chrono::high_resolution_clock::now();

		_words->FindWord();
		_words->Chronometr(_start, "FindWord");
		_words->Chronometr(start, "Total time");
		_words->PrintLongesWords();
	}
	delete _words;

	cout << "End" << endl;
	cout << "Hit any key to exit" << endl;
	getchar();

	return 1;
}


