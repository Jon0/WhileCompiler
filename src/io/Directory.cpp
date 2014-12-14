#include <iostream>
#include <dirent.h>

#include "Directory.h"

namespace io {

Directory::Directory(std::string l)
	:
	loc {l} {
}

Directory::~Directory() {}

std::vector<std::string> Directory::fileList(std::string type) {
	std::vector<std::string> names;

	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(loc.c_str())) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			std::string filename = ent->d_name;
			size_t ind = filename.find(".");

			//cout << "split " << filename.substr(0, ind) << " and " << filename.substr(ind+1) << endl;
			if (filename.substr(ind+1) == type) {
				names.push_back( filename.substr(0, ind) );
			}
		}
		closedir(dir);
	} else {
		/* could not open directory */
	}

	return names;
}

} /* namespace io */
