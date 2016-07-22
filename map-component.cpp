/* map-component.cpp
Copyright (c) 2016 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

// map-component: program to extract one connected component (e.g. the territory
// of one species) from the map. You can then edit it and map-merge it back in.
// $ g++ --std=c++11 -o map-component map-component.cpp
// $ ./map-component <file> <system>... > <out>

#include "shared/DisjointSet.cpp"

#include <fstream>
#include <iostream>
#include <map>
#include <string>

using namespace std;

void PrintHelp();
bool IsEmpty(const string &line);
string Token(const string &line, int index = 0);



int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		PrintHelp();
		return 1;
	}
	
	map<string, string> systems;
	DisjointSet links;
	
	ifstream in(argv[1]);
	string line;
	string current;
	while(getline(in, line))
	{
		// Skip blank lines.
		if(IsEmpty(line))
			continue;
		// If this line is not indented, it starts a new root object.
		if(line[0] > ' ')
			current.clear();
		
		if(Token(line, 0) == "system")
			current = Token(line, 1);
		else if(!current.empty() && Token(line, 0) == "link")
			links.Join(current, Token(line, 1));
		
		if(!current.empty())
		{
			systems[current] += line;
			systems[current] += '\n';
		}
	}
	
	vector<string> components;
	for(char **it = argv + 2; *it; ++it)
		components.push_back(*it);
	for(const pair<string, string> &it : systems)
	{
		bool match = false;
		for(const string &component : components)
			match |= links.IsJoined(it.first, component);
		
		if(match)
			cout << it.second << endl;
	}
	return 0;
}



void PrintHelp()
{
	cerr << endl;
	cerr << "Usage: $ map-component <map> <system>..." << endl;
	cerr << "   where <map> is the map file to extract a component from," << endl;
	cerr << "   and <system> is any system in that component." << endl;
	cerr << endl;
}



bool IsEmpty(const string &line)
{
	for(char c : line)
		if(c > ' ')
			return false;
	return true;
}



string Token(const string &line, int index)
{
	string::const_iterator it = line.begin();
	string::const_iterator end = line.end();
	
	string token;
	for( ; it != end && index >= 0; --index)
	{
		for( ; it != end && *it <= ' '; ++it) {}
		
		if(it != end)
		{
			char quote = '\0';
			if(*it == '"' || *it == '`')
				quote = *it;
			
			if(quote)
			{
				++it;
				for( ; it != end && *it != quote; ++it)
					if(!index)
						token += *it;
				if(it != end)
					++it;
			}
			else
			{
				for( ; it != end && *it > ' '; ++it)
					if(!index)
						token += *it;
			}
		}
		bool quote = false;
		for( ; it != end && *it > ' '; ++it)
			if(!index)
				token += *it;
	}
	return token;
}
