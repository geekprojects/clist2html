clist2html
==

clist2html is a tool to convert [Xchecklist](https://forums.x-plane.org/files/file/20785-xchecklist-linwinmac/)
checklists to HTML for easy printing.

It's purely a personal preference, but I find it easier to have checklists on paper in front of me rather
than faffing around with the mouse when flying with X-Plane. There are also lots of really good
checklists created for Xchecklist, so I created this to help me make printable versions of them!

As I am not the creator of those checklists, I won't distribute the HTML output. But if you're the creator
of an Xchecklist checklist, feel free to distribute your own HTML version.


# Usage
Build using CMake. It has no other dependencies.

Run the tool with the path clist.txt file:

```aiignore
./clist2html [options] <clist.txt>

Options:
  --columns <n>       Number of columns to format lists in to (Optional, defaults to 2)
  --title <title>     Title to display at the top (Optional)
  --output <filename> Filename of the output file (Optional)
```

Unless specified with --output, the tool will generate an HTML file with the original filename but the extention
changed to .html.


# License
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
