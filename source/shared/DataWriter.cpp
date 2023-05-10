/* DataWriter.cpp
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

#include "DataWriter.h"

#include "DataNode.h"

#if defined _WIN32
#include <windows.h>
#endif

#include <cstdlib>

using namespace std;



const string DataWriter::space = " ";



DataWriter::DataWriter()
	: before(&indent)
{
	out.precision(8);
}



string DataWriter::ToString() const
{
	return out.str();
}



void DataWriter::Write(const DataNode &node)
{
	for(int i = 0; i < node.Size(); ++i)
		WriteToken(node.Token(i).c_str());
	Write();

	if(node.begin() != node.end())
	{
		BeginChild();
		{
			for(const DataNode &child : node)
				Write(child);
		}
		EndChild();
	}
}



void DataWriter::Write()
{
	out << '\n';
	before = &indent;
}



void DataWriter::BeginChild()
{
	indent += '\t';
}



void DataWriter::EndChild()
{
	indent.erase(indent.length() - 1);
}



void DataWriter::WriteComment(const string &str)
{
	out << indent << "# " << str << '\n';
}



void DataWriter::AddLineBreak()
{
	out << '\n';
}



void DataWriter::WriteToken(const char *a)
{
	// Figure out what kind of quotation marks need to be used for this string.
	bool hasSpace = any_of(a.begin(), a.end(), [](char c) { return isspace(c); });
	bool hasQuote = any_of(a.begin(), a.end(), [](char c) { return (c == '"'); });
	// Write the token, enclosed in quotes if necessary.
	out << *before;
	if(hasQuote)
		out << '`' << a << '`';
	else if(hasSpace)
		out << '"' << a << '"';
	else
		out << a;
	before = &space;
}



void DataWriter::WriteToken(const string &a)
{
	WriteToken(a.c_str());
}
