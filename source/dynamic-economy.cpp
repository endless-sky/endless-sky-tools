/* dynamic-economy.cpp
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

// Simulator for the dynamic economy implementation. Every time you press <enter>,
// the simulation steps forward another 1000 days.
// $ g++ --std=c++11 -o dynamic-economy dynamic-economy.cpp
// $ ./dynamic-economy path/to/map.txt

#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <sstream>
#include <string>

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

double MapColor(double value, double *r, double *g, double *b)
{
	value = min(1., max(-1., value));
	if(value < 0.)
	{
		*r = 0.;
		*g = 50. * -value;
		*b = 100. * -value;
	}
	else
	{
		*r = 100. * value;
		*g = 50. * value;
		*b = 0.;
	}
}



int main(int argc, char *argv[])
{
	if(argc < 2)
		return 1;

	mt19937_64 gen;
	gen.seed(12345);
	normal_distribution<double> normal;

	map<string, double> posX;
	map<string, double> posY;
	map<string, set<string>> links;

	double minX = 0.;
	double maxX = 0.;
	double minY = 0.;
	double maxY = 0.;

	string line;
	string name;

	ifstream in(argv[1]);
	while(getline(in, line))
	{
		if(StartsWith(line, "system"))
			name = Token(line, 1);
		else if(StartsWith(line, "\tpos") && !name.empty())
		{
			double x = Value(line, 1);
			double y = Value(line, 2);
			minX = min(minX, x);
			maxX = max(maxX, x);
			minY = min(minY, y);
			maxY = max(maxY, y);
			posX[name] = x;
			posY[name] = y;
		}
		else if(StartsWith(line, "\tlink") && !name.empty())
			links[name].insert(Token(line, 1));
	}

	// Add a slight border around the edges.
	const double BORDER = .05;
	double xBorder = (maxX - minX) * BORDER;
	minX -= xBorder;
	maxX += xBorder;
	double yBorder = (maxY - minY) * BORDER;
	minY -= yBorder;
	maxY += yBorder;

	// Figure out the scale to apply to the map to keep dimensions below...
	const double MAX_DIMENSION = 900.;
	const double RADIUS = 4.;
	double scale = MAX_DIMENSION / max(maxX - minX, maxY - minY);
	int width = scale * (maxX - minX);
	int height = scale * (maxY - minY);

	// Run the simulation repeatedly.
	map<string, double> supply;
	map<string, double> trade;
	int DAYS = 1000;
	while(true)
	{
		static const double TRADE = .10;
		static const double KEEP = .89;
		static const double VOLUME = 10000.;
		static const double LIMIT = 100000.;
		for(int day = 0; day < DAYS; ++day)
		{
			for(const auto &it : links)
			{
				trade[it.first] = TRADE * supply[it.first];
				supply[it.first] *= KEEP;
				supply[it.first] += normal(gen) * VOLUME;
			}
			for(const auto &it : links)
				if(it.second.size())
				{
					double share = trade[it.first] / it.second.size();
					for(const string &link : it.second)
						supply[link] += share;
				}
		}
		// After the first day, step according to the given step size.
		if(argc > 2)
			DAYS = stoi(argv[2]);

		ofstream out("economy.svg");
		out << "<svg width=\"" << width << "\" height=\"" << height << "\">" << endl;
		out << "<rect width=\"" << width << "\" height=\"" << height << "\" fill=\"black\" />" << endl;

		// Draw the links.
		for(const auto &it : posX)
		{
			string system = it.first;
			double x1 = (posX[system] - minX) * scale;
			double y1 = (posY[system] - minY) * scale;
			for(const string &link : links[system])
			{
				// Only draw links in one direction.
				if(link <= system)
					continue;
				double x2 = (posX[link] - minX) * scale;
				double y2 = (posY[link] - minY) * scale;

				out << "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2
					<< "\" style=\"stroke:#444444;stroke-width:1.5\" />" << endl;
			}
		}

		// Draw circles for the systems.
		double lowest = 1.;
		double highest = -1.;
		for(const auto &it : posX)
		{
			string system = it.first;
			double x = (posX[system] - minX) * scale;
			double y = (posY[system] - minY) * scale;
			double value = erf(supply[it.first] / LIMIT);
			lowest = min(value, lowest);
			highest = max(value, highest);
			double r, g, b;
			MapColor(value, &r, &g, &b);

			out << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"" << RADIUS
				<< "\" fill=\"rgb(" << r << "%, " << g << "%, " << b << "%)\" />" << endl;
		}

		out << "</svg>" << endl;
		out.close();

		cout << "Adjustment range: " << lowest << " to " << highest;
		cin.get();
		if(!cin)
			break;
	}
	cout << endl;
	return 0;
}
