/* DataFile.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "DataFile.h"

#if defined _WIN32
#include <windows.h>
#endif

#include <cstdlib>
#include <stdexcept>

using namespace std;



DataFile::DataFile(const string &path)
{
	Load(path);
}



DataFile::DataFile(istream &in)
{
	Load(in);
}



void DataFile::Load(const string &path)
{
#if defined _WIN32
	FILE *file = _wfopen(ToUTF16(path).c_str(), L"rb");
#else
	FILE *file = fopen(path.c_str(), "rb");
#endif
	string data;
	if(!file)
		return;

	// Find the remaining number of bytes in the file.
	size_t start = ftell(file);
	fseek(file, 0, SEEK_END);
	size_t size = ftell(file) - start;
	// Reserve one extra byte in case there is no final '\n'.
	data.reserve(size + 1);
	data.resize(size);
	fseek(file, start, SEEK_SET);

	// Read the file data.
	size_t bytes = fread(&data[0], 1, data.size(), file);
	if(bytes != data.size())
		throw runtime_error("Error reading file!");

	// As a sentinel, make sure the file always ends in a newline.
	if(data.empty() || data.back() != '\n')
		data.push_back('\n');

	Load(&*data.begin(), &*data.end());
}



void DataFile::Load(istream &in)
{
	vector<char> data;

	static const size_t BLOCK = 4096;
	while(in)
	{
		size_t currentSize = data.size();
		data.resize(currentSize + BLOCK);
		in.read(&*data.begin() + currentSize, BLOCK);
		data.resize(currentSize + in.gcount());
	}
	// As a sentinel, make sure the file always ends in a newline.
	if(data.back() != '\n')
		data.push_back('\n');

	Load(&*data.begin(), &*data.end());
}



list<DataNode>::const_iterator DataFile::begin() const
{
	return root.begin();
}



list<DataNode>::const_iterator DataFile::end() const
{
	return root.end();
}



void DataFile::Load(const char *it, const char *end)
{
	vector<DataNode *> stack(1, &root);
	vector<int> whiteStack(1, -1);

	for( ; it != end; ++it)
	{
		// Find the first non-white character in this line.
		int white = 0;
		for( ; *it <= ' ' && *it != '\n'; ++it)
			++white;

		// If the line is a comment, skip to the end of the line.
		if(*it == '#')
		{
			while(*it != '\n')
				++it;
		}
		// Skip empty lines (including comment lines).
		if(*it == '\n')
			continue;

		// Determine where in the node tree we are inserting this node, based on
		// whether it has more indentation that the previous node, less, or the same.
		while(whiteStack.back() >= white)
		{
			whiteStack.pop_back();
			stack.pop_back();
		}

		// Add this node as a child of the proper node.
		list<DataNode> &children = stack.back()->children;
		children.emplace_back(stack.back());
		DataNode &node = children.back();

		// Remember where in the tree we are.
		stack.push_back(&node);
		whiteStack.push_back(white);

		// Tokenize the line. Skip comments and empty lines.
		while(*it != '\n')
		{
			char endQuote = *it;
			bool isQuoted = (endQuote == '"' || endQuote == '`');
			it += isQuoted;

			const char *start = it;

			// Find the end of this token.
			while(*it != '\n' && (isQuoted ? (*it != endQuote) : (*it > ' ')))
				++it;

			// It ought to be legal to construct a string from an empty iterator
			// range, but it appears that some libraries do not handle that case
			// correctly. So:
			if(start == it)
				node.tokens.emplace_back();
			else
				node.tokens.emplace_back(start, it);
			if(isQuoted && *it == '\n')
				node.PrintTrace("Closing quotation mark is missing:");

			if(*it != '\n')
			{
				it += isQuoted;
				while(*it != '\n' && *it <= ' ' && *it != '#')
					++it;

				// If a comment is encountered outside of a token, skip the rest
				// of this line of the file.
				if(*it == '#')
				{
					while(*it != '\n')
						++it;
				}
			}
		}
	}
}
