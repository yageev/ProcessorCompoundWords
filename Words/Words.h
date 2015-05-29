#include "stdafx.h"
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <thread>
#include <atomic>
#include <cstdlib>
#include <algorithm> 
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <bitset>
#include <ppl.h>
#include <unordered_set>
#include <concurrent_vector.h>

using namespace std;
using namespace concurrency;

const int MAX_BITS = 20;
const int COUNT_THREADS = 8;
const int MIN_WORD_LENGTH = 2;

class Words
{
	int maxLength;
	int countWords;
	string fileName;
	volatile int foundCompoundWords;
	concurrent_vector<string> words;

	set<string> wordSet;
	vector<set<string>> foundWords;
	vector <bitset<MAX_BITS>> bitsVector;
	map<int, vector<string>> traceWords;
	stringstream _chanks[COUNT_THREADS];

	bool fileExists(const char *filename);
	void AddTrace(int indexTrace, string subword);

  public:

	Words(const char *filename);
	bool LoadFile();
	void LoadChank(int pos);

	bool isWordExist(string s);
	void PrintLongesWords();
	void FindWord();
	bool CheckWords(string w1, string w2, int indexCurrent);
	bool CheckOut(int idx1, int idx_start, int idx_finish);

	void Chronometr(chrono::time_point<chrono::system_clock, chrono::system_clock::duration> start, string message);

};
