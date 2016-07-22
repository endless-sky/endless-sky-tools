// Program for generating a map of the galaxy, colored by government.
// $ g++ --std=c++11 -o mapper mapper.cpp
// $ ./mapper path/to/map.txt path/to/governments.txt > map.svg

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
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

void Color(double &r, double &g, double &b, double value)
{
	value = value * 2. - 1.;
	if(value < 0.)
	{
		r = 20. + 20. * value;
		g = 80. + 60. * value;
		b = 80. - 20. * value;
	}
	else
	{
		r = 20. + 80. * value;
		g = 80.;
		b = 80. - 80. * value;
	}
}



int main(int argc, char *argv[])
{
	if(argc <= 1)
		return 1;
	
	map<string, double> posX;
	map<string, double> posY;
	map<string, string> gov;
	map<string, double> trade;
	map<string, set<string>> links;
	
	map<string, double> govR;
	map<string, double> govG;
	map<string, double> govB;
	
	double minX = 0.;
	double maxX = 0.;
	double minY = 0.;
	double maxY = 0.;
	
	string line;
	string name;
	
	// First, read the government file, if any.
	string commodity;
	double tradeMin = 0.;
	double tradeMax = 2000.;
	if(argv[2] && argv[3])
	{
		commodity = argv[3];
		ifstream in(argv[2]);
		
		while(getline(in, line))
			if(StartsWith(line, "\tcommodity") && Token(line, 1) == commodity)
			{
				tradeMin = Value(line, 2);
				tradeMax = Value(line, 3);
			}
	}
	else if(argv[2])
	{
		ifstream in(argv[2]);
		
		while(getline(in, line))
		{
			if(StartsWith(line, "government"))
				name = Token(line, 1);
			else if(StartsWith(line, "\tcolor"))
			{
				govR[name] = Value(line, 1);
				govG[name] = Value(line, 2);
				govB[name] = Value(line, 3);
			}
		}
		name.clear();
	}
	
	// Now, parse the map.
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
		else if(StartsWith(line, "\tgovernment") && !name.empty())
			gov[name] = Token(line, 1);
		else if(StartsWith(line, "\tlink") && !name.empty())
			links[name].insert(Token(line, 1));
		else if(StartsWith(line, "\ttrade") && !name.empty() && Token(line, 1) == commodity)
			trade[name] = max(0., min(1., (Value(line, 2) - tradeMin) / (tradeMax - tradeMin)));
		else if(!StartsWith(line, "\t"))
			name.clear();
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
	const double MAX_DIMENSION = 600.;
	double scale = MAX_DIMENSION / max(maxX - minX, maxY - minY);
	int width = scale * (maxX - minX);
	int height = scale * (maxY - minY);
	cout << "<svg width=\"" << width << "\" height=\"" << height << "\">" << endl;
	cout << "<rect width=\"" << width << "\" height=\"" << height << "\" fill=\"black\" />" << endl;
	
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
			
			cout << "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2
				<< "\" style=\"stroke:#666666\" />" << endl;
		}
	}
	
	// Draw circles for the systems.
	const double RADIUS = 2.;
	for(const auto &it : posX)
	{
		string system = it.first;
		double x = (posX[system] - minX) * scale;
		double y = (posY[system] - minY) * scale;
		double r = 100.;
		double g = 100.;
		double b = 100.;
		auto cit = trade.find(system);
		auto git = gov.find(system);
		if(cit != trade.end())
			Color(r, g, b, cit->second);
		else if(git != gov.end() && govR.find(git->second) != govR.end())
		{
			r = 100. * govR[git->second];
			g = 100. * govG[git->second];
			b = 100. * govB[git->second];
		}
		else
			cerr << system << endl;
		
		cout << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"" << RADIUS
			<< "\" fill=\"rgb(" << r << "%, " << g << "%, " << b << "%)\" />" << endl;
	}
	
	cout << "</svg>" << endl;
	return 0;
}
