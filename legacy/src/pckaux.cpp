#include "pckcore.hpp"
#include <sstream>
#include <fstream>
#include <array>
#include <windows.h>
#include <cctype>
#include <stdarg.h>

namespace pck {
	// ===============================================================================
	//                          Bare auxiliary functions
	// ===============================================================================

	// speshul fx. hurr durr. me fancy.
	void spinner(int duration, WINDOW* scr) {
		int y, x;
		getyx(scr, y, x);

		for (int i = 1; i <= duration * 5; ++i) {
			mvwprintw(scr, y, x, "\\"); wrefresh(scr);
			sleep(50);
			mvwprintw(scr, y, x, "|"); wrefresh(scr);
			sleep(50);
			mvwprintw(scr, y, x, "/"); wrefresh(scr);
			sleep(50);
			mvwprintw(scr, y, x, "-"); wrefresh(scr);
			sleep(50);
		}
	}

	// Prints an error-colored message to the specified window.
	void wprinterr(WINDOW* win, const char* message, ...)
	{
		va_list args;
		va_start(args, message);
		wattron(win, COLOR_PAIR(ERRCOLOR));
		wprintw(win, message, args);
		wattroff(win, COLOR_PAIR(ERRCOLOR));
		va_end(args);
	}

	// Prints an ok-colored message to the specified window.
	void wprintok(WINDOW* win, const char* message, ...)
	{
		va_list args;
		va_start(args, message);
		wattron(win, COLOR_PAIR(OKCOLOR));
		wprintw(win, message, args);
		wattroff(win, COLOR_PAIR(OKCOLOR));
		va_end(args);
	}

	// Same as above but for stdscr
	void printerr(const char* message, ...)
	{
		va_list args;
		va_start(args, message);
		wprinterr(stdscr, message, args);
		va_end(args);
	}
	void printok(const char* message, ...)
	{
		va_list args;
		va_start(args, message);
		wprintok(stdscr, message, args);
		va_end(args);
	}

	// Slices a string n' splices it together into a vector of substrings
	std::vector<std::string> sliceNsplice(const std::string& str, char delim = ' ')
	{
		std::vector<std::string> res;
		std::stringstream sstr(str);
		std::string temp;

		// if there is no delimiter or string
		if (str.empty())
			return res;

		if (str.find_first_of(delim) == std::string::npos) {
			getline(sstr, temp, delim);
			res.push_back(temp);
			return res;
		}

		// remove leading delims
		if (str[0] == delim)
			sstr.ignore(INT_MAX, delim);

		while (getline(sstr, temp, delim)) {
			if (!temp.empty())
				res.push_back(temp);
		}

		return res;
	}

	// Initialize some custom color schemes.
	void initcolors() {
		init_pair(0, COLOR_BLACK, COLOR_WHITE);
		init_pair(1, COLOR_RED, COLOR_BLACK);
		init_pair(2, COLOR_GREEN, COLOR_BLACK);
		init_pair(3, COLOR_YELLOW, COLOR_BLACK);
		init_pair(4, COLOR_BLUE, COLOR_BLACK);
		init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(6, COLOR_CYAN, COLOR_BLACK);
		init_pair(7, COLOR_WHITE, COLOR_BLACK);

		//rainbow!
		init_color(RAINBOW1, 1000, 0, 0); //red
		init_color(RAINBOW2, 1000, 500, 0); //orange
		init_color(RAINBOW3, 1000, 1000, 0); //yellow
		init_color(RAINBOW4, 0, 1000, 0); //green
		init_color(RAINBOW5, 0, 750, 1000); //blue
		init_color(RAINBOW6, 0, 250, 1000); //indigo
		init_color(RAINBOW7, 700, 0, 1000); //violet

		init_pair(ERRCOLOR, RAINBOW1, COLOR_BLACK);
		init_pair(OKCOLOR, RAINBOW4, COLOR_BLACK);

		std::array<short, 7> rb = { RAINBOW1, RAINBOW2, RAINBOW3, RAINBOW4, RAINBOW5, RAINBOW6, RAINBOW7 };
		for (int i = 0; i < 7; ++i) {
			init_pair(i + 111, rb.at(i), COLOR_BLACK);
		}
	}

	// Auxiliary to check if something is in a given vector.
	template<class T>
	bool vector_isin(const std::vector<T>& target, const T& value) 
	{
		for (auto& val : target) {
			if (val == value)
				return true;
		}
		return false;
	}

	// Sleep using STL functions. Relying on Windows API too much makes me uncomfortable for some reason.
	void sleep(const unsigned ms_duration) 
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(ms_duration));
	}

	// yes or no? (stdscr)
	/*bool YN() {
		curs_set(1); echo(); noraw();
		int y, x;
		getyx(stdscr, y, x); ++y;

		//will be asked in the line immediately after its call
		mvprintw(y, 0, ">> ");
		int input = getch();

		input = toupper(input);
		//assures input is either Y or N
		while (input != 'Y' && input != 'N') {
			move(y, 0); clrtoeol();
			mvprintw(y, 0, ">> ");
			input = toupper(getch());
		}
		return (input == 'Y') ? true : false;
	}*/

	// ===============================================================================
	//                                      Menu                                     =
	// ===============================================================================

	// Updates the position of our selection cursor. startY delimits the options menu itself from the header. Pass curpos as -1 to do an initial print.
	void Menu::updatepos(int& curpos, const int& newpos, const int& startY, const std::vector<std::string>& opts, int jumps) 
	{
		if (curpos == newpos) return;
		else if (curpos == -1) {
			mvprintw(startY, 0, "-> ");
			chgat((int)opts.at(0).length(), A_REVERSE, 0, NULL);
			curpos = 0;
		}

		mvprintw(startY + curpos, 0, "   ");
		chgat((int)opts.at((size_t)curpos + jumps).length(), A_NORMAL, 0, NULL);
		mvprintw(startY + newpos, 0, "-> ");
		chgat((int)opts.at((size_t)newpos + jumps).length(), A_REVERSE, 0, NULL);

		curpos = newpos;
	}

	// Prints the collection received to stdscr.
	//template<class T>
	void Menu::printopts(const std::vector<std::string>& collection)
	{
		try {
			for (auto& option : collection) {
				printw("   %s\n", option.c_str());
			}
		}
		catch (...) {
			return;
		}
	}

	// Loops the menu with the specified items list
	void Menu::menu(const int startY, const std::vector<std::string>& options, const bool isMain)
	{
		int curpos = 0;
		const int numitems = (int)options.size();
		bool looping = true;

		while (looping) {
			int input = getch();
			//mvprintw(getmaxy(stdscr) - 1, 0, "Key: %d", input);

			if (input == KEY_RESIZE) {
				sleep(350);

				if (getmaxx(stdscr) >= minX && getmaxy(stdscr) >= minY)
					resize_term(0, 0);

				else if (getmaxx(stdscr) < minX)
					resize_term(getmaxy(stdscr), minX);

				else if (getmaxy(stdscr) < minY)
					resize_term(minY, getmaxx(stdscr));

				else resize_term(minY, minX);
			}

			else if (input == KEY_F(1) || input == KEY_LEFT) {
				if (isMain && input == KEY_LEFT) continue;
				looping = false;
			}

			else if (input == KEY_UP) {
				(curpos == 0) ?
					updatepos(curpos, numitems - 1, startY, options) : //loop to last
					updatepos(curpos, curpos - 1, startY, options);
			}

			else if (input == KEY_DOWN) {
				(curpos >= numitems - 1) ?
					updatepos(curpos, 0, startY, options) : //loop to first
					updatepos(curpos, curpos + 1, startY, options);
			}

			//select
			else if (vector_isin<int>(selectkeys, input)) {
				// last option will ALWAYS be "back" or "exit".
				if (options.at(curpos) == options.back())
					looping = false;
				else
					parseopt(curpos);
			}

			//numeric input
			else if (input >= 49 && input <= 57) {
				// 49 = key 1 (numpad or otherwise); 57 = key 9
				input -= 48;

				if (input > numitems || input == 0)
					Beep(1500, 100);
				else {
					//because the index starts at 0, but to the user's visuals, there's no 0th option (hence why 0 fails the above if)
					--input;

					updatepos(curpos, input, startY, options);
					refresh(); sleep(200);
					parseopt(curpos);
				}
			}
			//the hell you say?
			else
				Beep(1500, 100);
			refresh();
		} //end-while
	}

	// ===============================================================================
	//                                  DisplayQueue                                 =
	// ===============================================================================

	// Constructor
	DisplayQueue::DisplayQueue(std::vector<std::string> display, int startY) 
	{
		hidden = 0;
		this->startY = startY;
		this->display = display;
	}

	// Scroll function
	void DisplayQueue::scroll(bool up) 
	{
		if (display.size() <= 9) return; //can't scroll unscrollable screens

		hidden += (up) ? -1 : +1;
		for (int i = 0; i < 9; ++i) {
			move(startY + i, 0); clrtoeol();
			mvprintw(startY + i, 0, "   %s", display.at((size_t)hidden + i).c_str());
		}

		if (up) {
			mvprintw(startY, 0, "-> ");
			chgat((int)display.at((size_t)hidden).length(), A_REVERSE, 0, NULL);
		}
		else {
			mvprintw(startY + 8, 0, "-> ");
			chgat((int)display.at((size_t)hidden + 8).length(), A_REVERSE, 0, NULL);
		}

		return;
	}


	// Queue menu (single-choice lifespan)
	int DisplayQueue::qmenu() 
	{
		if (display.size() == 0) return -1; //object is useless without something to display
		//that being said, try not to use the default constructor.

		raw(); noecho(); keypad(stdscr, TRUE);
		const int maxitems = 9, numitems = (int)display.size();
		int curpos = -1;

		for (int i = 0; i < maxitems && i < numitems; ++i) {
			mvprintw(startY + i, 0, "   %s", display.at(i).c_str());
		}
		updatepos(curpos, 0, startY, display);
		refresh();

		bool looping = true;
		while (looping) {
			int input = getch();

			if (input == KEY_RESIZE) {
				sleep(350);

				if (getmaxx(stdscr) < minX && getmaxy(stdscr) < minY)
					resize_term(minY, minX);

				else if (getmaxx(stdscr) < minX)
					resize_term(getmaxy(stdscr), minX);

				else if (getmaxy(stdscr) < minY)
					resize_term(minY, getmaxx(stdscr));

				else resize_term(0, 0);
			}

			else if (input == KEY_F(1) || input == KEY_LEFT) {
				curpos = -1; hidden = 0;
				looping = false;
			}

			else if (input == KEY_UP) {
				if (numitems == 1) continue;

				if (curpos != 0)
					updatepos(curpos, curpos - 1, startY, display, hidden);

				else {
					if (numitems <= maxitems) {
						updatepos(curpos, numitems - 1, startY, display);
					}
					else if (hidden == 0) { //loop to last
						for (int j = 0; j < numitems - 9; ++j)
							scroll(false);
						updatepos(curpos, 8, startY, display, hidden);
					}
					else {
						scroll(true);
					}
				}
			}

			else if (input == KEY_DOWN) {
				//can't move in a single-item menu, can we
				if (numitems == 1) continue;

				if (numitems <= maxitems) {
					if (curpos == numitems - 1) //loop to first
						updatepos(curpos, 0, startY, display);

					else
						updatepos(curpos, curpos + 1, startY, display);
				}

				else if (curpos != 8)
					updatepos(curpos, curpos + 1, startY, display, hidden);

				else {
					if (hidden + curpos == numitems - 1) { //loop to first
						for (int j = hidden; j > 0; --j)
							scroll(true);
						updatepos(curpos, 0, startY, display);
					}
					else {
						scroll(false);
					}
				}
			}

			//select
			else if (vector_isin<int>(selectkeys, input))
				looping = false;

			else //literally what
				Beep(1500, 100);

			refresh();
		}

		return curpos + hidden;
	}

	// ===============================================================================
	//                                  FileSniffer                                  =
	// ===============================================================================

	// Checks if the given file exists in the given path
	bool FileSniffer::exists(std::string filepath)
	{
		std::fstream chk(filepath, std::fstream::in);
		bool result = chk.is_open();
		chk.close();
		return result;
	}

	// Sets max output length for folder strings - minimum 30 chars, max 100 chars
	void FileSniffer::setMaxLen(size_t newlen) 
	{
		if (newlen >= 30 && newlen <= 100)
			maxPathLen = newlen;
	}

	// Returns the current max length for folder paths
	size_t FileSniffer::getMaxLen() 
	{
		return maxPathLen;
	}

	// Convert a path object to string
	std::string FileSniffer::path2string(std::filesystem::directory_entry p)
	{
		std::filesystem::path buffer = p.path();
		return buffer.string();
	}

	// Toggles print mode
	void FileSniffer::togglePrintMode() 
	{
		//1 = file name
		//2 = file name with path from given root

		(printmode == 1) ?
			++printmode :
			--printmode;
	}

	// File/Extension separator from a full path object
	std::string FileSniffer::fileName(std::string str, unsigned mode, const std::string pathref)
	{
		//modes:
		//0 = extension
		//1 = file name
		//2 = file name with path from given root
		//3 = path with no file (from root)
		//4 = path with no file (full)

		for (auto& ch : str) {
			if (ch == '/')
				ch = '\\';
		}

		std::size_t found = 0;
		if (mode == 0)
			found = str.find_last_of('.');

		else if (mode == 1)
			found = str.find_last_of("\\");

		else if (mode == 4) {
			found = str.find_last_of("\\");
		}

		else {
			//modes 2 and 3 depend on the root path to determine where it starts
			if (pathref.empty())
				throw - 1;
			else
				found = pathref.length() - 1;
		}


		if (mode == 4)
			str = str.substr(0, found);

		else if (mode != 3)
			str = str.substr(found + 1);

		else {
			size_t temp = str.find_last_of("\\");
			str = str.substr(found + 1, temp - found);
		}

		if (mode == 0) std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		return str;
	}

	// Find all files with a given extension
	std::pair<bool, std::vector<std::string>> FileSniffer::find() 
	{
		namespace fs = std::filesystem;
		std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
		std::vector<std::string> results; bool success;
		std::string buffer;

		//quick integrity check on path given
		if (path.back() != '/' && path.back() != '\\')
			path.push_back('/');

		try {
			for (auto& file : fs::recursive_directory_iterator(path)) {
				buffer = path2string(file);

				if (fileName(buffer, 0) == extension)
					results.push_back(fileName(buffer, printmode, path));
			}
			success = results.size() > 0;
		}
		catch (...) {
			success = false;
			results.clear();
		}

		return std::pair<bool, std::vector<std::string>>(success, results);
	}
}