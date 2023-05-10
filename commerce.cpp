/* commerce.cpp
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

// commerce: program for populating the map with commodity values.
// $ g++ --std=c++11 -o commerce commerce.cpp
// $ ./commerce <map> <settings>
// The settings file should contain key-value pairs:
// name <commodity>
// base <minimum>
// bins <weight>...

#include "shared/DataFile.cpp"
#include "shared/DataNode.cpp"
#include "shared/DataWriter.cpp"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

class System {
public:
	void Load(const DataNode &node);

	const vector<string> &Links() const;

	void SetTrade(const string &commodity, double value);

	void Write(DataWriter &out) const;


private:
	string name;
	double x;
	double y;
	vector<string> links;

	map<string, double> trade;
};

class Value {
public:
	int minBin;
	int maxBin;
	int bin;
};



int main(int argc, char *argv[])
{
	if(argc < 3)
		return 1;

	srand(time(NULL));

	// Load the "settings."
	string commodity;
	int base = 0;
	vector<double> binWeight;
	{
		DataFile file(argv[2]);
		double total = 0.;
		for(const DataNode &node : file)
		{
			if(node.Token(0) == "name" && node.Size() >= 2)
				commodity = node.Token(1);
			else if(node.Token(0) == "base" && node.Size() >= 2)
				base = node.Value(1);
			else if(node.Token(0) == "bins" && node.Size() >= 2)
				for(int i = 1; i < node.Size(); ++i)
				{
					binWeight.push_back(node.Value(i));
					total += binWeight.back();
				}
		}
		if(!base || commodity.empty() || binWeight.empty() || !total)
			return 1;
		for(double &value : binWeight)
			value /= total;
	}

	// Load the map file.
	map<string, System> systems;
	vector<string> names;
	{
		DataFile file(argv[1]);
		for(const DataNode &node : file)
			if(node.Size() >= 2 && node.Token(0) == "system")
			{
				names.push_back(node.Token(1));
				systems[node.Token(1)].Load(node);
			}
	}
	// Generate the quotas from the weights.
	vector<int> binQuota;
	for(double weight : binWeight)
		binQuota.push_back(ceil(weight * names.size()) + 1);

	// Look for an arrangement that works.
	int highBin = binQuota.size();
	map<string, Value> values;
	while(true)
	{
		// We have not assigned any values yet. So, we have our full quota
		// remaining, and each star can be assigned to any bin.
		vector<int> bin = binQuota;
		for(const string &name : names)
		{
			values[name].minBin = 0;
			values[name].maxBin = highBin;
		}

		// Keep track of which stars haven't been assigned values yet.
		vector<string> unassigned = names;
		while(unassigned.size())
		{
			// Pick a random star to assign a value to.
			int i = rand() % unassigned.size();
			string name = unassigned[i];
			unassigned[i] = unassigned.back();
			unassigned.pop_back();

			// Find out how many items left in our quota could be assigned to
			// this particular star.
			int possibilities = 0;
			for(int i = values[name].minBin; i < values[name].maxBin; ++i)
				possibilities += bin[i];
			if(!possibilities)
				break;

			// Pick a random one of those items to assign to it.
			int index = rand() % possibilities;
			int choice = values[name].minBin;
			while(true)
			{
				index -= bin[choice];
				if(index < 0)
					break;
				++choice;
			}
			--bin[choice];

			// Record our choice.
			values[name].bin = choice;
			int minBin = choice;
			int maxBin = choice + 1;

			// Starting from this star, trace outwards system by system. Each
			// neighboring system must be within 1 of this star's level; each
			// system neighboring those, within 2, and so on.
			vector<string> source = {name};
			set<string> done = {name};
			while(minBin > 0 || maxBin < highBin)
			{
				// Widen the min and max unless they are at their widest.
				if(minBin > 0)
					--minBin;
				if(maxBin < highBin)
					++maxBin;

				vector<string> next;

				// Update the min and max for each unvisited neighbor.
				for(const string &sourceName : source)
					for(const string &name : systems[sourceName].Links())
					{
						if(done.find(name) != done.end())
							continue;
						done.insert(name);

						values[name].minBin = max(values[name].minBin, minBin);
						values[name].maxBin = min(values[name].maxBin, maxBin);
						next.push_back(name);
					}

				// Now, visit neighbors of those neighbors.
				next.swap(source);
			}
		}
		// If there were not any stars that we could not assign values to, we
		// are done. Especially if the constraints are rather tight, it may
		// take quite a few iterations to find an acceptable solution. Or, it
		// may be outright impossible, in qhich case the program hangs.
		if(!unassigned.size())
			break;
	}

	// Assign each star system a value based on its bin.
	map<string, int> rough;
	for(auto &it : values)
		rough[it.first] = base + (rand() % 100) + 100 * it.second.bin;

	// Smooth out the values by averaging each system with the average of all
	// its neighbors.
	for(auto &it : systems)
	{
		int count = 0;
		int sum = 0;
		for(const string &link : it.second.Links())
		{
			sum += rough[link];
			++count;
		}
		if(!count)
			sum = rough[it.first];
		else
		{
			sum += count * rough[it.first];
			sum = (sum + count) / (2 * count);
		}
		it.second.SetTrade(commodity, sum);
	}

	// Write the result. This is not a full map; it needs to be merged into the
	// map using the map-merge tool.
	DataWriter out;
	for(const auto &it : systems)
		it.second.Write(out);
	string output = out.ToString();

	{
		ofstream file(argv[1]);
		file.write(output.data(), output.length());
	}

	return 0;
}



void System::Load(const DataNode &node)
{
	links.clear();
	trade.clear();
	name = node.Token(1);

	for(const DataNode &child : node)
	{
		if(child.Token(0) == "pos" && child.Size() >= 3)
		{
			x = child.Value(1);
			y = child.Value(2);
		}
		else if(child.Token(0) == "link" && child.Size() >= 2)
			links.push_back(child.Token(1));
		else if(child.Token(0) == "trade" && child.Size() >= 3)
			trade[child.Token(1)] = child.Value(2);
	}
}

const vector<string> &System::Links() const
{
	return links;
}

void System::SetTrade(const string &commodity, double value)
{
	trade[commodity] = value;
}

void System::Write(DataWriter &out) const
{
	out.Write("system", name);
	out.BeginChild();

	out.Write("pos", x, y);
	for(const string &link : links)
		out.Write("link", link);
	for(const auto &it : trade)
		out.Write("trade", it.first, it.second);

	out.EndChild();
	out.AddLineBreak();
}
