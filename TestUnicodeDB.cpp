#include "UnicodeDB.h"
#include <Windows.h>

int main() {
	WCHAR *indexPath = L"D:\\cygwin\\home\\Daniel\\ibus-unicode-db\\data\\index";
	WCHAR *dataPath = L"D:\\cygwin\\home\\Daniel\\ibus-unicode-db\\data\\UnicodeData.txt";
	UnicodeDB udb(indexPath, dataPath);

	vector <char *> wordlist;
	set<UNICODE_T> list, fuzzy_list;

	wordlist.push_back("greek");
	wordlist.push_back("capital");

	udb.findCandidates(wordlist, list, fuzzy_list);

	for (auto i = list.begin();
		i != list.end();
		i++) {

		printf("Exact match: %s\n", (*i).data );
	}
	for (auto i = list.begin();
		i != list.end();
		i++) {

		printf("Fuzzy match: %s\n", (*i).data );
	}
	return 0;
}