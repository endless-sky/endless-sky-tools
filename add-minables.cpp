/* add-minables.cpp
Copyright (c) 2016 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

// add-minables: program to add "minables" to a map file.
// $ g++ --std=c++11 -o add-minables add-minables.cpp
// $ ./add-minables < <file> > <out>

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <random>
#include <sstream>

using namespace std;

bool StartsWith(const string &line, const string &str)
{
	return line.length() >= str.length() && !line.compare(0, str.length(), str);
}

string Token(const string &line, int index)
{
	size_t pos = 0;
	
	while(pos < line.length())
	{
		while(line[pos] <= ' ')
			++pos;
		
		char quote = 0;
		if(line[pos] == '"' || line[pos] == '`')
			quote = line[pos++];
		size_t start = pos;
		while(pos < line.length() && (quote ? (line[pos] != quote) : (line[pos] > ' ')))
			++pos;
		
		if(!index--)
			return line.substr(start, pos - start);
		pos = pos + !!quote;
	}
	return "";
}

double Value(const string &line, int index)
{
	double value = 0.;
	istringstream(Token(line, index)) >> value;
	return value;
}



int main(int argc, char *argv[])
{
	srand(time(nullptr));
	
	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<> real(0., 1.);
	
	map<string, double> probability = {
		{"aluminum",  0.12},
		{"copper",  0.08},
		{"gold",  0.02},
		{"iron",  0.13},
		{"lead",  0.15},
		{"neodymium",  0.03},
		{"platinum",  0.01},
		{"silicon",  0.2},
		{"silver",  0.05},
		{"titanium",  0.11},
		{"tungsten",  0.06},
		{"uranium",  0.04}
	};
	
	bool skip = true;
	bool previousWasHabitable = false;
	bool previousWasAsteroids = false;
	int totalCount = 0;
	double totalEnergy = 0.;
	
	string line;
	while(getline(cin, line))
	{
		if(StartsWith(line, "\thabitable"))
			previousWasHabitable = true;
		else if(previousWasHabitable)
		{
			previousWasHabitable = false;
			skip = StartsWith(line, "\tbelt");
			totalCount = 0;
			totalEnergy = 0.;
			if(!skip)
				cout << "\tbelt " << static_cast<int>(1000. + 1000. * real(gen)) << '\n';
		}
		
		if(!skip && StartsWith(line, "\tasteroids"))
		{
			previousWasAsteroids = true;
			int count = Value(line, 2);
			double energy = Value(line, 3);
			totalCount += count;
			totalEnergy += energy * count;
		}
		else if(previousWasAsteroids)
		{
			previousWasAsteroids = false;
			double meanEnergy = totalCount ? (totalEnergy / totalCount) : 0.;
			
			map<string, double> choices;
			
			// Minables should be much less prevalent than ordinary asteroids.
			totalCount /= 4;
			for(int i = 0; i < 3; ++i)
			{
				totalCount = rand() % (totalCount + 1);
				if(!totalCount)
					break;
				
				double choice = real(gen);
				for(const auto &it : probability)
				{
					choice -= it.second;
					if(choice < 0.)
					{
						choices[it.first] += totalCount;
						break;
					}
				}
			}
			for(const auto &it : choices)
			{
				double energy = (real(gen) + 1.) * meanEnergy;
				cout << "\tminables " << it.first << " " << it.second << " " << energy << "\n";
			}
		}
		cout << line << '\n';
	}
	return 0;
}
