#ifndef DIRECTORY_H_
#define DIRECTORY_H_

#include <string>
#include <vector>

namespace io {

/**
 * A file directory
 */
class Directory {
public:
	Directory(std::string);
	virtual ~Directory();

	/**
	 * read all files with a given extension
	 */
	std::vector<std::string> fileList(std::string);

private:
	std::string loc;
};

} /* namespace io */

#endif /* DIRECTORY_H_ */
