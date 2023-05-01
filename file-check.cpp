// Program to check data files for non-ASCII characters and non-Unix line endings.
// $ g++ --std=c++11 -o file-check file-check.cpp
// $ ./file-check path/to/data/*.txt

#include <iostream>
#include <fstream>

using namespace std;



int main(int argc, char *argv[])
{
	for(char **it = argv + 1; *it; ++it)
	{
		ifstream in(*it, ios::binary);
		
		in.seekg(0, ios::end);
		size_t bytes = in.tellg();
		in.seekg(0, ios::beg);
		
		char *data = new char[bytes];
		in.read(data, bytes);
		char *end = data + bytes;
		
		int line = 1;
		int pos = 1;
		for(char *cit = data; cit < end; ++cit, ++pos)
		{
			if(*cit == '\n')
			{
				++line;
				pos = 0;
			}
			else if(*cit == '\t')
				continue;
			else if(*cit < ' ' || *cit == 127)
			{
				cerr << *it << ":" << line << ":" << pos << ": Invalid character (" << int(*cit) << ")." << endl;
				break;
			}
		}
		if(bytes && end[-1] != '\n')
			cerr << *it << ": File does not end with a newline." << endl;
		
		delete [] data;
	}
	
	return 0;
}

