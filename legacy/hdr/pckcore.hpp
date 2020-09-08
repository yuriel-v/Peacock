/* Peacock Framework v0.1 (Beta)
 * (c) Leonardo Valim & Joshua Walker, all rights reserved.
 *
 * This is an interactive terminal for configuring and executing any utilities I (Leo)
 * made and will make for my assistant Joshu in his lab endeavors.
 *
 * It includes its own namespace, "pck", along with some nifty auxiliary functions for
 * it and any of the other projects linked to it.
 *
 * The projects are linked, preferrably by means of a configuration class, which serves
 * as a bridge between the actual utility and the framework, which is, at its core, an
 * interactive terminal and nothing more. Said configuration classes preferrably derive
 * from the auxiliary superclass Menu for easier and quicker implementation.
 *
 * A word on includes: This header ONLY includes headers relating to the function of the
 * auxiliary classes and the framework core itself. Other headers containing configuration
 * classes for utilities should be included in the .cpp file itself, so they're visible
 * to the framework's option parser. The configuration classes, should they inherit from
 * the Menu superclass, should include this header in theirs.
 *
 *
 * Known bugs: Maximizing the terminal window may cause some weird behavior, like the cursor
 * showing up, or other windows suddenly resizing into big windows again.
 *
 * Leo, 25-Apr-2020.
 */
#ifndef PCKCORE_HPP
#define PCKCORE_HPP

#include <vector>
#include <string>
#include <curses.h>
#include <thread>
#include <chrono>
#include <filesystem>

namespace pck {
	constexpr char PCKRELEASE[] = "Beta\0";
	constexpr char PCKVER[] = "0.1\0";


	//rainbow!

	constexpr short RAINBOW1 = 111;
	constexpr short RAINBOW2 = 112;
	constexpr short RAINBOW3 = 113;
	constexpr short RAINBOW4 = 114;
	constexpr short RAINBOW5 = 115;
	constexpr short RAINBOW6 = 116;
	constexpr short RAINBOW7 = 117;
	constexpr short ERRCOLOR = 120;
	constexpr short OKCOLOR = 121;


	// functions

	template<class T>
	bool vector_isin(const std::vector<T>&, const T&);

	void sleep(const unsigned);
	void initcolors();
	std::vector<std::string> sliceNsplice(const std::string& str, char delim);

	void wprinterr(WINDOW* win, const char* message, ...);
	void wprintok(WINDOW* win, const char* message, ...);
	void printerr(const char* message, ...);
	void printok(const char* message, ...);
	void spinner(int duration, WINDOW* scr = stdscr); //ME FANCY HURR DURR


	/* ============================================================================== *
	 * Menu class                                                                     *
	 *                                                                                *
	 * Auxiliary superclass for other classes' integrated menus.                      *
	 * ============================================================================== */
	class Menu {
	public:
		static const int minX = 120;
		static const int minY = 30;
		const std::vector<int> selectkeys = { KEY_RIGHT, KEY_ENTER, '\n', '\r', PADENTER };

		void menu(const int startY, const std::vector<std::string>& options, const bool isMain = false);
		void updatepos(int& curpos, const int& newpos, const int& startY, const std::vector<std::string>& opts, int jumps = 0);
		virtual void parseopt(int& option) = 0;

		//template<class T>
		void printopts(const std::vector<std::string>&);
	};

	/* ============================================================================== *
	 * DisplayQueue class                                                             *
	 *                                                                                *
	 * Auxiliary class for displaying its given display items in a queue-styled menu. *
	 * Will display up to 9 items at a time.                                          *
	 *                                                                                *
	 * menu() will return the index of the selected item in display.                  *
	 * ============================================================================== */

	class DisplayQueue : private Menu {
		std::vector<std::string> display;
		int startY;
		int hidden;

		void scroll(bool);
		void parseopt(int& option) { return; }
	public:
		DisplayQueue() { startY = 0; hidden = 0; }
		DisplayQueue(std::vector<std::string> display, int startY);
		int qmenu();
	};

	/* ============================================================================== *
	 * FileSniffer class                                                              *
	 *                                                                                *
	 * Auxiliary class for finding files of a specific extension in a given folder.   *
	 * If possible, instance objects of this class should be called "dognose" or      *
	 * similar variants. (^:                                                          *
	 * ============================================================================== */

	class FileSniffer {
		size_t maxPathLen = 50;
		unsigned printmode = 1;
		//1 = file name
		//2 = file name with path from given root

		std::string path2string(std::filesystem::directory_entry);
	public:
		static std::string fileName(std::string str, unsigned mode, const std::string pathref = "");
		static bool exists(std::string filepath);
		std::string extension;
		std::string path;

		std::pair<bool, std::vector<std::string>> find();
		void setMaxLen(size_t newlen);
		size_t getMaxLen();
		void togglePrintMode();
	};

	/* ============================================================================== *
	 * Peacock class                                                                  *
	 *                                                                                *
	 * Core class of the Peacock Framwork. Hosts the main menu and handles calls to   *
	 * other classes for other projects.                                              *
	 * ============================================================================== */

	class Peacock : public Menu {

	private:
		std::vector<std::string> projects;
		int startY;     //tells us where header ends
	
		void parseopt(int& option);
		void printheader();

	public:
		bool stdscr_initialized = false;
		Peacock();
		void redflash(int, int, int);
		void main_menu();
	};
}

#endif //PCKCORE_HPP