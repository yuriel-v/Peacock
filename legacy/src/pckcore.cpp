/* Peacock Framework v0.1 (Alpha)
 * (c) Leonardo Valim & Joshua Walker, all rights reserved.
 *
 * This is the implementation file for the framework core. Include any utility headers
 * below, should they contain classes for the execution and/or configuration of other
 * utilities to be called by the framework.
 */
#include "pckcore.hpp"
#include "matrixinator.hpp"
#include <iostream>
#include <sstream>

int main()
{
	pck::Peacock* pck = new pck::Peacock();
	pck->main_menu();

	endwin();
	delete pck;

	return 0;
}

namespace pck {
	// ===============================================================================
	//                                  Peacock Core                                 =
	// ===============================================================================

	//constructor
	Peacock::Peacock()
	{
		if (!stdscr_initialized) {
			initscr();
			keypad(stdscr, TRUE);
			curs_set(0);
			resize_term(minY, minX);

			start_color();
			initcolors();
			stdscr_initialized = true;
		}
		startY = 0;
	}

	// private:

	// Selects an option
	void Peacock::parseopt(int& option)
	{
		if (option == 0) { //matrixinator

			mtx::MatrixConfig* mtxconfig = new mtx::MatrixConfig;
			mtxconfig->mtxMenu();
			printheader();
			option = 0;
			delete mtxconfig;
		}
		else if (option == 1) { //documentator
			redflash(startY + option, 3, (int)projects.at(option).length());
		}
	}

	// Leading header
	void Peacock::printheader() 
	{
		clear();
		raw(); noecho(); nonl(); curs_set(0);

		int any;
		attrset(COLOR_PAIR(114));
		printw("Peacock Framework v%s (%s)", PCKVER, PCKRELEASE);					  //0
		if (has_colors()) {
			printw(" - Now with ");
			addch('c' | COLOR_PAIR(RAINBOW1)); //red
			addch('o' | COLOR_PAIR(RAINBOW2)); //orange
			addch('l' | COLOR_PAIR(RAINBOW3)); //yellow
			addch('o' | COLOR_PAIR(RAINBOW4)); //green
			addch('r' | COLOR_PAIR(RAINBOW5)); //blue
			addch('s' | COLOR_PAIR(RAINBOW6)); //indigo
			addch('!' | COLOR_PAIR(RAINBOW7)); //purple
		}
		
		printw("\n(c) Leonardo Valim & Joshua Walker, all rights reserved.\n"		  //1
			     "--------------------------------------------------------\n");	      //2
			
		printw("Press right-arrow or enter to select. Press F1 to quit.\n\n");        //3
		getyx(stdscr, startY, any);	any = -1;										  //5
		attrset(COLOR_PAIR(COLOR_BLACK));

		projects.clear();
		projects = {
			"The Matrixinator",
			"The Documentator",
			"Exit program (F1)"
		};
		
		printopts(projects);
		updatepos(any, 0, startY, projects);
		refresh();
	}

	// Flashes a particular option red. Signals "not yet implemented"
	void Peacock::redflash(int y, int x, int chars) 
	{
		mvchgat(y, x, chars, A_REVERSE | COLOR_PAIR(111), 1, NULL); refresh();
		sleep(100);
		mvchgat(y, x, chars, A_REVERSE, 0, NULL); refresh();
		sleep(100);
		mvchgat(y, x, chars, A_REVERSE | COLOR_PAIR(111), 1, NULL); refresh();
		sleep(100);
		mvchgat(y, x, chars, A_REVERSE, 0, NULL); refresh();
	}

	// public:

	// Main menu, core of the framework
	void Peacock::main_menu() 
	{
		printheader();
		menu(startY, projects, true);

		//exit routine
		int endX, endY;
		getmaxyx(stdscr, endY, endX);
		endY /= 2; endX = (endX - 27) / 2;
		init_pair(120, COLOR_WHITE, COLOR_RED); attron(COLOR_PAIR(120));
		curs_set(0);
		mvprintw(endY, endX, "Terminating program in ");
		for (int i = 3; i > 0; --i) {
			mvprintw(endY, endX + 23, "%d...", i); refresh();
			sleep(1000);
		}
		return;
	}
}