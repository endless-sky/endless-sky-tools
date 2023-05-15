/* color-converter.cpp
Copyright (c) 2023 by warp-core

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

// A program for converting between the Endless Sky rgba color code format
// and 24-bit hexadecimal HTML colors.
// $ g++ --std=c++11 -o color-converter color-converter.cpp
// $ ./color-converter <r#> <g#> <b#>
// Takes the given ES color and prints to STDOUT the HTML representation.
// $ ./color-converter #RRGGBB
// Takes the given HTML color and prints to STDOUT the ES color representation.

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace {
	double ParseString(const string &input)
	{
		// Digits before the decimal point.
		int64_t value = 0;

		auto it = input.begin();
		while(*it >= '0' && *it <= '9')
			value = (value * 10) + (*it++ - '0');

		// Digits after the decimal point (if any).
		int64_t power = 0;
		if(*it == '.')
		{
			++it;
			while(*it >= '0' && *it <= '9')
			{
				value = (value * 10) + (*it++ - '0');
				--power;
			}
		}

		if(*it)
			cerr << "Encountered invalid character in numerical value." << endl;

		double result = value;
		result *= pow(10., power);
		return result;
	}

	int HexToDec(const char hex)
	{
		if(hex >= '0' && hex <= '9')
			return hex - '0';
		if(hex >= 'A' && hex <= 'F')
			return hex + 10 - 'A';
		if(hex >= 'a' && hex <= 'f')
			return hex + 10 - 'a';
		cerr << "Invalid character in hexadecimal sequence: " << hex << endl;
		return 0;
	}

	string DecToHex(const int input)
	{
		static const vector<string> conversion = {
			"0",
			"1",
			"2",
			"3",
			"4",
			"5",
			"6",
			"7",
			"8",
			"9",
			"A",
			"B",
			"C",
			"D",
			"E",
			"F",
		};

		if(input >= 16)
			return DecToHex(input / 16) + conversion[input % 16];
		return conversion[input];
	}

	void PrintHelp()
	{
		cerr << "<r#> <g#> <b#>: pass three numeric values between 0 and 1, representing a color in the format used"
				" in Endless Sky data, and the corresponding HTML color code will be printed to STDOUT." << endl;
		cerr << "#RRGGBB: pass a six character hexadecimal representation of a 24-bit color (HTML format), beginning"
				" with a '#' symbol and the corresponding Endless Sky color code will be printed to STDOUT." << endl;
		cerr << endl;
		cerr << "Return values:" << endl;
		cerr << "    1: incorrect argument count. Expected 1 or 3." << endl;
		cerr << "    2: too few arguments for Endless Sky color code, but HTML code does not begin with '#'." << endl;
		cerr << "    3: too few characters in HTML color code." << endl;
	}
}



int main(int argc, char *argv[])
{
	if(argc == 2)
	{
		string html = argv[1];

		if(html[0] != '#')
		{
			PrintHelp();
			return 2;
		}
		if(html.size() < 7)
		{
			PrintHelp();
			return 3;
		}

		double rgb[3];
		for(int i = 1; i < 6; ++i)
			rgb[i] = (16 * HexToDec(html[i]) + HexToDec(html[++i])) / 255.;

		for(int i = 0; i < 3; ++i)
		{
			if(i)
				cout << ' ';
			cout << rgb[i];
		}

		return 0;
	}

	if(argc == 4)
	{
		int rgb[3];
		for(int i = 0; i < 3; ++i)
			rgb[i] = ParseString(argv[i + 1]) * 255;

		string result = "#";
		for(int i = 0; i < 3; ++i)
		{
			rgb[i] = max(rgb[i], 0);
			rgb[i] = min(rgb[i], 255);
			string val = DecToHex(rgb[i]);
			if(val.size() < 2)
				result += "0";
			result += val;
		}

		cout << result;

		return 0;
	}

	PrintHelp();
	return 1;
}
