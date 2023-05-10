/* file-check.cpp
Copyright (c) 2016 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

// Program to check data files for non-ASCII characters and non-Unix line endings.
// $ g++ --std=c++11 -o file-check file-check.cpp
// $ ./file-check path/to/data/*.txt

#include <fstream>
#include <iostream>

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

