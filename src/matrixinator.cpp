#include "matrixinator.hpp"
#include <fstream>
#include <sstream>
#include <exception>

namespace mtx {
	// ===============================================================================
	//                                  Matrixinator                                 =
	// ===============================================================================

	// Constructor
	Matrixinator::Matrixinator()
	{
		numNodes = 0;
		numSamples = 0;
	}

	// Wide-string slice n' splice, matrixinator-exclusive
	std::vector<std::wstring> Matrixinator::w_sliceNsplice(const std::wstring& wstr, char delim)
	{
		std::vector<std::wstring> res;
		std::wstringstream sstr(wstr);
		std::wstring temp;

		// if there is no delimiter or string
		if (wstr.empty())
			return res;

		if (wstr.find_first_of(delim) == std::string::npos) {
			std::getline<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>(sstr, temp, delim);
			res.push_back(temp);
			return res;
		}

		// remove leading delims
		if (wstr[0] == delim)
			sstr.ignore(INT_MAX, delim);

		while (std::getline<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>(sstr, temp, delim)) {
			//if (!temp.empty())
				res.push_back(temp);
		}

		return res;
	}

	// True if the given ID is an USA sample, false otherwise
	bool Matrixinator::isUS(int id)
	{
		for (auto& sample : USAsamples)
			if (sample == id) return true;

		return false;
	}

	// Carry all the node IDs from sample to root, adding them to the nodes' lists along the way
	void Matrixinator::bulldozer(int node)
	{
		if (!acacia.at(node).isSample()) return;

		std::vector<int> changeList;
		int parent = acacia.at(node).getParentID();

		while (acacia.at(parent).getID() >= 1) {
			changeList.push_back(acacia.at(node).getID());
			node = parent;
			parent = acacia.at(parent).getParentID();

			//add change list into parent's child list
			for (auto& item : changeList)
				acacia.at(node).addChild(item);
		}

		//node 1, parent 0
		for (auto& item : changeList)
			acacia.at(node).addChild(item);
	}

	// Compare a "node"'s similarity to "origin". Returns 0 if it's below 80%.
	double Matrixinator::bullSim(int node, int origin)
	{
		if (!acacia.at(node).isSample()) return 0;

		int parent = acacia.at(node).getParentID();
		while (acacia.at(parent).getID() >= 1) {
			//the almighty time saver returns
			if (acacia.at(node).getSim() < 80)
				return 0;

			for (auto& it : acacia.at(parent).getChildren()) {
				if (it == origin)
					return acacia.at(parent).getSim();
			}

			node = parent;
			parent = acacia.at(parent).getParentID();
		}

		//just in case
		return 0;
	}

	// Init phase: read files to memory
	void Matrixinator::init()
	{
		//read metadata file
		std::wfstream* wfptr = new std::wfstream(paths.getFullFilepath(true), std::wfstream::in);
		wfptr->ignore(INT_MAX, '\n');
		int cnt = 0; std::wstring* in = new std::wstring; std::vector<std::wstring>* pieces = new std::vector<std::wstring>;

		while (!wfptr->eof()) {
			Metadata* temp = new Metadata();

			in->clear(); pieces->clear();
			std::getline<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>(*wfptr, *in, '\n');

			*pieces = w_sliceNsplice(*in, ',');
			if (pieces->empty()) break;
			//because trailing commas are also returned by wide slicing (intended)
			else {
				while (pieces->size() > 28)
					pieces->pop_back();
			}

			if (pieces->at(1) == L"US" || pieces->at(1) == L"USA") {
				USAsamples.push_back(cnt);

				//separate octagon
				std::array<double, 8> oct;
				for (int i = 7; i >= 0; --i) {
					double duo;
					std::wstringstream(pieces->back()) >> duo;
					oct[i] = duo;
					pieces->pop_back();
				}

				temp->setOctagon(oct);
			}

			temp->setData(*pieces);
			SS.push_back(*temp);
			delete temp;

			++cnt;
		}

		numSamples = (int)SS.size();
		wfptr->close();
		
		//because fuck wide stuff, honestly
		delete wfptr; delete in; delete pieces;

		//let's work with thin streams this time, shall we? (read tree file)
		std::fstream file(paths.getFullFilepath(false), std::fstream::in);
		Tree* starter = new Tree();
		starter->setIDs(0, 0);
		starter->setSim(0.0);
		acacia.push_back(*starter);
		delete starter;

		std::string buffer;
		int id, parentid;
		double sim;
		file.ignore(INT_MAX, '\"');


		while (!file.eof()) {
			Tree* temp = new Tree();

			std::getline(file, buffer, '\"');
			std::stringstream(buffer) >> id;

			file.ignore(INT_MAX, '\"');
			buffer.clear();
			std::getline(file, buffer, '\"');
			std::stringstream(buffer) >> parentid;

			file.ignore(INT_MAX, '\"');
			buffer.clear();
			std::getline(file, buffer, '\"');
			std::stringstream(buffer) >> sim;

			if (file.peek() != '/')
				temp->makeSample();

			temp->setIDs(id, parentid);
			temp->setSim(sim);
			acacia.push_back(*temp);

			delete temp;
			file.ignore(INT_MAX, '\"');
		}

		numNodes = (int)acacia.size() - 1; //node 0 is a fictional node
		file.close();

		//integrity check
		for (std::vector<Metadata>::iterator it = SS.begin(); it != SS.end(); ++it) {
			if (it->getData().at(0).empty()) { //no key, no sample
				it = SS.erase(it);
				--numSamples;
				if (it == SS.end())
					break;
			}
		}
	}

	// Post-init phase: prepare data structures
	void Matrixinator::postinit()
	{
		//branching
		for (int i = 1; i <= numNodes; ++i) {
			if (acacia.at(i).isSample())
				bulldozer(i);
		}

		//node-sample association
		int sCount = 0;
		for (int i = 1; i <= numNodes; ++i) {
			if (acacia.at(i).isSample()) {
				SS.at(sCount).associate(i);
				++sCount;
			}
		}

	}

	// Sweep phase: process data in memory
	void Matrixinator::sweep()
	{
		for (int foreign = 0; foreign < numSamples; ++foreign) {

			if (isUS(foreign)) {
				continue;
			}
			std::vector<double> similarities;
			std::vector<int> matches;
			similarities.clear(); //just in case
			matches.clear();

			int caseCount = 0;
			for (auto& item : USAsamples) {
				double sim = bullSim(SS.at(foreign).getNode(), SS.at(item).getNode());

				if (sim >= 80) {
					++caseCount;
					matches.push_back(item);
					similarities.push_back(sim);

					if (detailed)
						SS.at(foreign).appendMatch(SS.at(item).getData().at(0), sim);
				}
			}

			switch (caseCount) {
			case 0:
				// because if it gets here, then no matches have been made up above
				if (detailed)
					SS.at(foreign).appendMatch(std::make_pair(std::to_wstring(0), 0));

				//no need as constructor sets it as this by default
				//SS.at(foreign).setOctagon({ -1, -1, -1, -1, -1, -1, -1, -1 });
				break;

			case 1:
				//SS.at(foreign).setOctagon(SS.at(matches.at(0)).getOctagon());
				SS.at(foreign).copyOct(SS.at(matches.at(0)));
				break;

			default:
				//multiply origins' octagon values by the similarity of origin samples to the foreign sample
				std::vector<std::vector<double>> tempOct(matches.size());
				for (int i = 0; i < matches.size(); i++)
					tempOct.at(i).resize(8);

				//similarity between matches and foreign, lined up by index: match[0] => sim[0]
				/*std::vector<double> similarities;
				for (auto& it : matches)
					similarities.push_back(bullSim(SS.at(foreign).getNode(), SS.at(it).getNode()));*/

				int i = 0;
				for (auto& it : matches) {
					for (int j = 0; j < 8; ++j) //octagon value #j
						tempOct[i][j] = SS.at(it).getOctagon().at(j) * similarities.at(i);
					++i;
				}

				//add up all the similarities between origins and foreign
				double tempSim = 0;
				for (auto& it : similarities)
					tempSim += it;

				//add up all octagon values into a single octagon set
				double foreignOctagon[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
				for (int i = 0; i < (int)matches.size(); ++i) {
					for (int j = 0; j < 8; ++j)
						foreignOctagon[j] += tempOct[i][j];
				}

				//then divide these values by the added similarity "tempSim"
				for (int i = 0; i < 8; ++i) {
					foreignOctagon[i] /= tempSim;
				}

				//now send it back
				SS.at(foreign).setOctagon(foreignOctagon);
			}
		}
	}

	// Output phase: dumps memory content into a .csv file
	void Matrixinator::output()
	{
		//WE WIDE AGAIN FUCK MY LIFE WOO
		typedef pck::FileSniffer fsn;
		std::wfstream output;
		std::string path, noext = paths.getMeta().substr(0, paths.getMeta().find_last_of('.'));
		path = (overwrite) ? paths.getFullFilepath(true) : paths.getLeadingPath(true) + "\\" + noext;

		//check if the file already exists
		if (fsn::exists(path + "-out.csv") && !overwrite) {
			size_t count = 1;
			while (fsn::exists(path + "-out" + std::to_string(count) + ".csv")) {
				++count;
			}
			path.append("-out" + std::to_string(count) + ".csv");
		}
		else
			path.append("-out.csv");

		output.open(path, std::wfstream::out | std::wfstream::trunc);

		if (!output.is_open()) {
			output.close();
			output.open(noext + "-out.csv", std::wfstream::out);

			if (!output.is_open())
				throw 1;
		}

		output << std::setprecision(8);
		output << std::fixed;
		//name header
		output << L"Key,Location,CollectionDate,Company,FSGID,Farm,Age_days,SampleOrigin,SampleType,VMP,ibeA,traT,iutA,ompT,sitA,irp2,cvaC,tsh,iucC,iss"
			<< L",BS22,BS15,BS3,BS8,BS27,BS84,BS18,BS278,";

		if (detailed)
			output << L"MatchKey=Similarity,";

		output << L"\n";
		
		for (auto& entry : SS) {
			//data
			for (auto& field : entry.getData())
				output << field << L",";

			if (!entry.nullOct()) {
				//octagon
				for (auto& value : entry.getOctagon())
					output << value << L",";

				//matches
				if (detailed) {
					for (auto& pair : entry.getMatches())
						output << pair.first << L"=" << pair.second << L",";
				}
			}
			else if (detailed)
				output << L",,,,,,,,,";
			else
				output << L",,,,,,,,";
			output << L"\n";
		}
	}

	// Main sequence
	void Matrixinator::mainSequence() 
	{
		int maxY, maxX; getmaxyx(stdscr, maxY, maxX);
		maxY /= 2;

		WINDOW* mtxcon = newwin(maxY, maxX, maxY, 0);
		wborder(mtxcon, '|', '|', '-', '-', '#', '#', '#', '#');
		wrefresh(mtxcon);

		mvwprintw(mtxcon, 1, 1, "Initializing the Matrixinator v%s... ", MTXVER); wrefresh(mtxcon);
		pck::spinner(2, mtxcon); //because speshul fx, makes it look like it's actually loading

		//because aesthethiccs are important
		wmove(mtxcon, getcury(mtxcon), getcurx(mtxcon) - 1);
		wprintw(mtxcon, "   ");

		//fail-safe
		if (!isIOdefined()) {
			wmove(mtxcon, getcury(mtxcon) + 1, 1);
			pck::wprinterr(mtxcon, "Fatal error: ");
			wprintw(mtxcon, "I/O Files have not been defined!");
			wrefresh(mtxcon);
			closing(2, mtxcon); return;
		}

		mvwprintw(mtxcon, getcury(mtxcon) + 1, 1, "Reading files to memory... "); wrefresh(mtxcon);
		try {
			init();
			pck::wprintok(mtxcon, "done."); wrefresh(mtxcon);
		}
		catch (const std::exception & e) {
			pck::wprinterr(mtxcon, "Exception caught."); wmove(mtxcon, getcury(mtxcon) + 1, 1);
			pck::wprinterr(mtxcon, "-> %s", e.what()); wrefresh(mtxcon);
			closing(10, mtxcon); return;
		}
		catch (...) {
			pck::wprinterr(mtxcon, "Exception caught! We don't know which one though."); wrefresh(mtxcon);
			closing(10, mtxcon); return;
		}

		mvwprintw(mtxcon, getcury(mtxcon) + 1, 1, "Running post-initialization... "); wrefresh(mtxcon);
		try {
			postinit();
			pck::wprintok(mtxcon, "done.");
		}
		catch (const std::exception & e) {
			pck::wprinterr(mtxcon, "Exception caught."); wmove(mtxcon, getcury(mtxcon) + 1, 1);
			pck::wprinterr(mtxcon, "-> %s", e.what()); wrefresh(mtxcon);
			closing(10, mtxcon); return;
		}
		catch (...) {
			pck::wprinterr(mtxcon, "Exception caught! We don't know which one though."); wrefresh(mtxcon);
			closing(10, mtxcon); return;
		}

		mvwprintw(mtxcon, getcury(mtxcon) + 1, 1, "Performing memory data sweep... "); wrefresh(mtxcon);
		try {
			sweep();
			pck::wprintok(mtxcon, "done."); wrefresh(mtxcon);
		}
		catch (const std::exception & e) {
			pck::wprinterr(mtxcon, "Exception caught."); wmove(mtxcon, getcury(mtxcon) + 1, 1);
			pck::wprinterr(mtxcon, "-> %s", e.what()); wrefresh(mtxcon);
			closing(10, mtxcon); return;
		}
		catch (...) {
			pck::wprinterr(mtxcon, "Exception caught! We don't know which one though."); wrefresh(mtxcon);
			closing(10, mtxcon); return;
		}

		(overwrite) ?
			mvwprintw(mtxcon, getcury(mtxcon) + 1, 1, "Outputting results to file: \"%s\"... ", paths.getMeta().c_str()) :
			mvwprintw(mtxcon, getcury(mtxcon) + 1, 1, "Outputting results to file: \"output.csv\"... ");
		wrefresh(mtxcon);
		try {
			output();
			pck::wprintok(mtxcon, "done."); wrefresh(mtxcon);
			closing(0, mtxcon); return;
		}
		catch (const std::exception & e) {
			pck::wprinterr(mtxcon, "Exception caught."); wmove(mtxcon, getcury(mtxcon) + 1, 1);
			pck::wprinterr(mtxcon, "-> %s", e.what()); wrefresh(mtxcon);
			closing(10, mtxcon); return;
		}
		catch (const int ex) {
			pck::wprinterr(mtxcon, "Exception caught: Error code #%d", ex); wrefresh(mtxcon);
			closing(ex, mtxcon); return;
		}
		catch (...) {
			pck::wprinterr(mtxcon, "Exception caught! We don't know which one though."); wrefresh(mtxcon);
			closing(10, mtxcon); return;
		}
	}

	// Closing sequence
	void Matrixinator::closing(int code, WINDOW* mtxcon) 
	{
		mvwprintw(mtxcon, (getmaxy(mtxcon) / 4) * 3, 1, "Execution terminated. Code: %d | Status: ", code);
		if (code == 0)
			pck::wprintok(mtxcon, "OK");
		else {
			pck::wprinterr(mtxcon, "FAILURE");
			mvwprintw(mtxcon, getcury(mtxcon) + 1, 1, "-> ");
		}

		switch (code) {
		case 1:
			wprintw(mtxcon, "The program was not able to create the output file, neither in the input metadata file's folder, nor in its own folder.");
			mvwprintw(mtxcon, getcury(mtxcon) + 1, 1, "Please make sure the folders exist, and that they are not read-only.");
			break;

		case 2:
			wprintw(mtxcon, "The program could not find the I/O files to read from. Please set them before initializing the Matrixinator.");
			break;

		case 10:
			wprintw(mtxcon, "The program encountered an exception from the standard library.");
			break;
		}

		mvwprintw(mtxcon, getcury(mtxcon) + 1, 1, "Press any key to continue.");
		refresh();
		wrefresh(mtxcon);
		getch();

		wborder(mtxcon, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
		for (int i = 0; i < getmaxy(mtxcon); ++i) {
			wmove(mtxcon, i, 0); wclrtoeol(mtxcon);
		}
		refresh();
		wrefresh(mtxcon);
		delwin(mtxcon);
	}
}