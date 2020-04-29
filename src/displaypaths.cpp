#include "matrixinator.hpp"

namespace mtx {
	// ===============================================================================
	//                                  DisplayPaths                                 =
	// ===============================================================================

	// Constructors
	DisplayPaths::DisplayPaths() 
	{
		treeFile = "Undefined";
		metaFile = "Undefined";
		readFolder = "Undefined";
		treeRoot = "Undefined";
		metaRoot = "Undefined";
		charLim = 75;
	}
	DisplayPaths::DisplayPaths(std::string mf, std::string tf, std::string rf) 
	{
		setFolder(rf);
		setFile(mf);
		setFile(tf);
		charLim = 75;
	}

	// Path string trimmer
	std::string DisplayPaths::pathTrimmer(std::string path, unsigned charlimit) 
	{
		if (charlimit == 0)
			charlimit = charLim;

		for (auto& ch : path)
			if (ch == '/')
				ch = '\\';

		if (path.find_first_of("\\") == std::string::npos || path.length() <= charlimit)
			return path;

		std::vector<std::string> pieces = pck::sliceNsplice(path, '\\');
		std::string result;

		if (pieces.size() == 1)
			result = pieces.at(0);

		else if (pieces.size() == 2)
			result = pieces.at(0) + "\\" + pieces.at(1);

		else
			result = pieces.at(0) + "\\...\\" + pieces.at(pieces.size() - 2) + "\\" + pieces.back();


		if (result.length() > charlimit) {
			pieces = the_abbrevi8r_9001(pieces, charlimit / pieces.size());
			if (pieces.size() > 2)
				result = pieces.at(0) + "\\...\\" + pieces.at(pieces.size() - 2) + "\\" + pieces.back();
			else if (pieces.size() == 2)
				result = pieces.at(0) + "\\" + pieces.at(1);
			else
				result = pieces.at(0);
		}

		return result;
		
	}

	// Path string abbreviator (for really long folder/file names)
	std::vector<std::string> mtx::DisplayPaths::the_abbrevi8r_9001(std::vector<std::string> longstring, unsigned charlimit) 
	{
		for (auto& sub : longstring) {
			if (sub.length() <= charlimit || (sub.length() <= charlimit + 5 && sub == longstring.back()))
				continue;

			else if (sub == longstring.back()) { //file
				sub = sub.substr(0, charlimit / 2) + "..." + sub.substr(sub.length() - charlimit / 2 - 1);
			}
			else {
				sub = sub.substr(0, charlimit / 2) + "..." + sub.substr(sub.length() - charlimit / 2 - 1);
			}
		}

		return longstring;
	}

	// Sets char limit for virtual paths. Min 50 chars, max 100 chars. 0 resets to default (75 chars).
	void DisplayPaths::setCharLim(unsigned limit = 0)
	{
		if (limit == 0)
			charLim = 75;
		else if (limit >= 50 && limit <= 100)
			charLim = limit;
		else if (limit < 50)
			charLim = 50;
		else //>100
			charLim = 100;
	}

	// Returns true if the string in question has "Undefined" as content. False otherwise
	bool DisplayPaths::isUndefined(const std::string& str) 
	{
		if (str == "Undefined")
			return true;
		else
			return false;
	}

	// Getters: Return either the virtual (trimmed) or real (full) root folder

	// Char limit
	unsigned DisplayPaths::getCharLim()
	{
		return charLim;
	}

	// Virtual folder
	std::string DisplayPaths::getVFolder()
	{
		return pathTrimmer(readFolder);
	}

	// Real folder
	std::string DisplayPaths::getRFolder()
	{
		return readFolder;
	}

	// Metadata .csv file
	std::string DisplayPaths::getMeta()
	{
		return metaFile;
	}

	// Dendrogram .xml file
	std::string DisplayPaths::getTree()
	{
		return treeFile;
	}

	// Full path to any file: true = metadata file, false = tree file
	std::string DisplayPaths::getFullFilepath(bool metafile) 
	{
		std::string* fptr = (metafile) ? &metaFile : &treeFile;
		std::string* rptr = (metafile) ? &metaRoot : &treeRoot;

		if (isUndefined(*fptr) || isUndefined(readFolder))
			return std::string("Undefined");

		std::string output = readFolder;
		if (!isUndefined(*rptr))
			output.append(*rptr);

		output.append(*fptr);
		return output;
	}

	// Leading path to any file: true = metadata file, false = tree file
	std::string DisplayPaths::getLeadingPath(bool metafile)
	{
		std::string path = getFullFilepath(metafile);
		size_t found = 0;
		found = path.find_last_of('\\');
		path = path.substr(0, found);
		return path;
	}

	// Setters

	// Root folder: Indicated by a slash, forward or back, anywhere in the string. Else, resets.
	void DisplayPaths::setFolder(std::string rf) 
	{
		if (rf.empty() || rf.find_first_of("/\\") == std::string::npos) {
			readFolder = "Undefined";
			return;
		}

		for (auto& ch : rf) {
			if (ch == '/')
				ch = '\\';
		}
		if (rf.back() != '\\')
			rf.append("\\");

		readFolder = rf;
	}

	// File setter: type true = metadata file, false = tree file
	void DisplayPaths::setFile(std::string file) 
	{
		//can't append a path to an empty path, go set folder first
		if (readFolder == "Undefined")
			return;

		for (auto& ch : file) {
			if (ch == '/')
				ch = '\\';
		}

		std::vector<std::string> filePieces = pck::sliceNsplice(file, '\\');

		//this is a specialized matrixinator class, any other file types are irrelevant
		std::string extension = filePieces.back().substr(filePieces.back().length() - 3, 3);
		std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
		if (extension != "csv" && extension != "xml")
			return;

		std::string* ptr;
		if (extension == "csv") {
			metaRoot.clear();
			ptr = &metaRoot;
		}
		else {
			treeRoot.clear();
			ptr = &treeRoot;
		}

		for (std::vector<std::string>::const_iterator it = filePieces.begin(); it != filePieces.end(); ++it) {
			if (it == filePieces.end() - 1) {
				if (extension == "csv")
					metaFile = *it;
				else
					treeFile = *it;
			}
			else {
				ptr->append(*it + "\\");
			}
		}
	}
}