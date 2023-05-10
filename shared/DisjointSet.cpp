/* DisjointSet.cpp
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

#include "DisjointSet.h"

using namespace std;



void DisjointSet::Join(const string &first, const string &second)
{
	Add(first);
	Add(second);

	string firstRoot = Root(first);
	string secondRoot = Root(second);
	if(firstRoot == secondRoot)
		return;

	vector<string> &firstEntry = entries[firstRoot];
	vector<string> &secondEntry = entries[secondRoot];

	bool firstIsSmaller = (firstEntry.size() < secondEntry.size());
	vector<string> &smaller = firstIsSmaller ? firstEntry : secondEntry;
	vector<string> &larger = firstIsSmaller ? secondEntry : firstEntry;

	const string &newRoot = firstIsSmaller ? secondRoot : firstRoot;
	larger.insert(larger.end(), smaller.begin(), smaller.end());
	for(const string &token : smaller)
	{
		vector<string> &entry = entries[token];
		if(&entry != &smaller)
			entry = vector<string>(1, newRoot);
	}
	smaller = vector<string>(1, newRoot);
}



bool DisjointSet::IsJoined(const string &first, const string &second) const
{
	if(first == second)
		return true;

	string firstRoot = Root(first);
	string secondRoot = Root(second);
	if(firstRoot.empty() || secondRoot.empty())
		return false;

	return firstRoot == secondRoot;
}



void DisjointSet::Add(const string &token)
{
	vector<string> &entry = entries[token];
	if(entry.empty())
		entry.emplace_back(token);
}



const string &DisjointSet::Root(const string &token) const
{
	static const string EMPTY;
	auto it = entries.find(token);
	return (it == entries.end() || it->second.empty()) ? EMPTY : it->second.front();
}
