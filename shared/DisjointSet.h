/* DisjointSet.h
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

#ifndef DISJOINT_SET_H_
#define DISJOINT_SET_H_

#include <map>
#include <string>
#include <vector>



// Class for tracking connected components. This is a simplified implementation,
// not a fully optimized one, not intended for huge sets.
class DisjointSet {
public:
	void Join(const std::string &first, const std::string &second);
	bool IsJoined(const std::string &first, const std::string &second) const;


private:
	void Add(const std::string &token);
	const std::string &Root(const std::string &token) const;


private:
	std::map<std::string, std::vector<std::string>> entries;
};



#endif
