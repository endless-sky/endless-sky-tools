/* map-merge.cpp
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

// map-merge: program to merge map data from two or more files.
// $ g++ --std=c++11 -o map-merge map-merge.cpp
// $ ./map-merge <file>... > <out>

#include "shared/DataFile.cpp"
#include "shared/DataNode.cpp"
#include "shared/DataWriter.cpp"

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

typedef map<string, vector<DataNode>> Object;

void PrintHelp();
void Write(DataWriter &out, const string &root, const map<string, Object> &data, const vector<string> &order);



int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		PrintHelp();
		return 1;
	}

	map<string, Object> galaxies;
	map<string, Object> systems;
	map<string, Object> planets;
	vector<DataNode> others;

	string line;
	for(char **it = argv + 1; *it; ++it)
	{
		DataFile file(*it);
		for(const DataNode &node : file)
		{
			Object *current = nullptr;
			if(node.Token(0) == "galaxy")
				current = &galaxies[node.Token(1)];
			else if(node.Token(0) == "system")
				current = &systems[node.Token(1)];
			else if(node.Token(0) == "planet")
				current = &planets[node.Token(1)];
			else
			{
				others.push_back(node);
				continue;
			}

			set<string> active;
			for(const DataNode &child : node)
			{
				if(!active.count(child.Token(0)))
				{
					(*current)[child.Token(0)].clear();
					active.insert(child.Token(0));
				}
				(*current)[child.Token(0)].push_back(child);
			}
		}
	}

	static const vector<string> GALAXY = {
		"pos", "sprite"};
	static const vector<string> SYSTEM = {
		"pos", "government", "music", "habitable", "belt", "link", "asteroids", "minables",
		"trade", "fleet", "object"};
	static const vector<string> PLANET = {
		"attributes", "landscape", "music", "description", "spaceport", "shipyard", "outfitter",
		"required reputation", "bribe", "security", "tribute"};

	DataWriter out;
	Write(out, "galaxy", galaxies, GALAXY);
	Write(out, "system", systems, SYSTEM);
	Write(out, "planet", planets, PLANET);

	string output = out.ToString();
	// Handle the fact that the map editor always uses backticks for
	// descriptions and spaceports even when not required.
	static const string FIX[] = {"\n\tdescription \"", "\n\tspaceport \""};
	for(const string &fix : FIX)
	{
		size_t start = 0;
		while(true)
		{
			size_t pos = output.find(fix, start);
			if(pos == string::npos)
				break;

			pos += fix.length();
			output[pos - 1] = '`';
			while(pos < output.length() && output[pos] != '\n')
				++pos;
			output[pos - 1] = '`';
			start = pos;
		}
	}
	cout << output;

	return 0;
}



void PrintHelp()
{
	cerr << endl;
	cerr << "Usage: $ map-merge <file>..." << endl;
	cerr << endl;
}



void Write(DataWriter &out, const string &root, const map<string, Object> &data, const vector<string> &order)
{
	for(const auto &it : data)
	{
		out.Write(root, it.first);
		out.BeginChild();

		const Object &object = it.second;
		set<string> used;
		for(const string &tag : order)
		{
			used.insert(tag);

			auto oit = object.find(tag);
			if(oit == object.end())
				continue;

			for(const DataNode &node : oit->second)
				out.Write(node);
		}

		for(const auto &oit : object)
			if(!used.count(oit.first))
			{
				for(const DataNode &node : oit.second)
					out.Write(node);
			}

		out.EndChild();
		out.AddLineBreak();
	}
}
