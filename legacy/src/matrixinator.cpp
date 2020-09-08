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
		for (int& sample : USAsamples)
			if (sample == id) return true;

		return false;
	}

	// Carry all the node IDs from sample to root, adding them to the nodes' lists along the way
	void Matrixinator::bulldozer(int node)
	{
		//direct-access code
		if (!acacia[node].sample) return;
		std::vector<int> changeList;
		int parent = acacia[node].IDs.second;

		while (acacia[parent].IDs.first >= 1) {
			changeList.push_back(acacia[node].IDs.first);
			node = parent;
			parent = acacia[parent].IDs.second;

			for (int& item : changeList)
				acacia[node].childList.insert(item);
		}

		//node 1, parent 0
		for (int& item : changeList)
			acacia[node].childList.insert(item);


		//method-access code
		//if (!acacia.at(node).isSample()) return;

		//std::vector<int> changeList;
		//int parent = acacia.at(node).getParentID();

		//while (acacia.at(parent).getID() >= 1) {
		//	changeList.push_back(acacia.at(node).getID());
		//	node = parent;
		//	parent = acacia.at(parent).getParentID();

		//	//add change list into parent's child list
		//	for (auto& item : changeList)
		//		acacia.at(node).addChild(item);
		//}

		////node 1, parent 0
		//for (auto& item : changeList)
		//	acacia.at(node).addChild(item);
	}

	// Compare a "node"'s similarity to "origin". Returns 0 if it's below 80%.
	double Matrixinator::bullSim(int node, int origin)
	{
		if (!acacia[node].sample) return 0;

		int parent = acacia[node].IDs.second;

		while (acacia[parent].IDs.first >= 1) {
			// the almighty time saver
			if (acacia[node].similarity < 80)
				return 0;

			for (const int& it : acacia[parent].childList) {
				if (it == origin)
					return acacia[parent].similarity;
			}

			node = parent;
			parent = acacia[parent].IDs.second;
		}
		// deprecated fail-safe
		return acacia[node].similarity;
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
			if (acacia[i].sample)
				bulldozer(i);
		}

		//node-sample association
		int sCount = 0;
		for (int i = 1; i <= numNodes; ++i) {
			if (acacia[i].sample) {
				SS[sCount].nodeNumber = i;
				++sCount;
			}
		}

	}

	// Sweep phase: process data in memory
	void Matrixinator::sweep()
	{
		std::vector<double> similarities;
		std::vector<int> matches;

		for (int foreign = 0; foreign < numSamples; ++foreign) {

			if (isUS(foreign)) {
				continue;
			}
			similarities.clear();
			matches.clear();

			int caseCount = 0;
			for (int& item : USAsamples) {
				double sim = bullSim(SS[foreign].nodeNumber, SS[item].nodeNumber);

				if (sim >= 80) {
					++caseCount;
					matches.push_back(item);
					similarities.push_back(sim);

					if (detailed) {
						//std::pair<std::wstring, double> pr(SS[item].data.front(), sim);
						SS[foreign].usaMatches.push_back(std::make_pair(SS[item].data.front(), sim));
					}
				}
			}

			switch (caseCount) {
			case 0:
				// because if it gets here, then no matches have been made up above
				if (detailed)
					SS[foreign].usaMatches.push_back(std::make_pair(std::to_wstring(0), 0));
				break;

			case 1:
				SS[foreign].octagon = SS[matches[0]].octagon;
				//SS.at(foreign).copyOct(SS.at(matches.at(0)));
				break;

			default:
				//multiply origins' octagon values by the similarity of origin samples to the foreign sample
				std::vector<std::array<double, 8>> tempOct(matches.size());
				std::array<double, 8> foreignOctagon;
				double tempSim = 0;

				int i = 0;
				for (int& it : matches) {
					for (int j = 0; j < 8; ++j) //octagon value #j
						//tempOct[i][j] = SS.at(it).getOctagon().at(j) * similarities.at(i);
						tempOct[i][j] = SS[it].octagon[j] * similarities[i];
					++i;
				}

				//add up all the similarities between origins and foreign
				for (double& it : similarities)
					tempSim += it;

				//add up all octagon values into a single octagon set
				foreignOctagon.fill(0);
				for (int i = 0; i < (int)matches.size(); ++i) {
					for (int j = 0; j < 8; ++j)
						foreignOctagon[j] += tempOct[i][j];
				}

				//then divide these values by the added similarity "tempSim" and send it back
				SS[foreign].octagon.clear();
				for (int i = 0; i < 8; ++i) {
					//foreignOctagon[i] /= tempSim;
					SS[foreign].octagon.push_back(foreignOctagon[i] / tempSim);
				}

				//now send it back
				//SS.at(foreign).setOctagon(foreignOctagon);
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

		for (Metadata& entry : SS) {
			//data
			for (std::wstring& field : entry.getData())
				output << field << L",";

			if (!entry.nullOct()) {
				//octagon
				for (double& value : entry.getOctagon())
					output << value << L",";

				//matches
				if (detailed) {
					for (std::pair<std::wstring, double>& pair : entry.getMatches())
						output << pair.first << L"=" << pair.second << L"; ";
					output << L",";
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

		//benchmarking
		beg = std::chrono::high_resolution_clock::now();

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

			mvwprintw(mtxcon, getcury(mtxcon) + 1, 1, "Outputting results to file: \"%s\"... ",
				(paths.getMeta().substr(0, paths.getMeta().find_last_of('.')) + "-out.csv").c_str()
			);

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
		end = std::chrono::high_resolution_clock::now();
		double total = (double)std::chrono::duration_cast<std::chrono::milliseconds>(end - beg).count() / 1000;
		mvwprintw(mtxcon, (getmaxy(mtxcon) / 4) * 3, 1, "Execution terminated. Duration: %.2fs | Code: %d | Status: ", total, code);
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