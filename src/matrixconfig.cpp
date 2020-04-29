#include "matrixinator.hpp"
#include <fstream>
#include <cstring>

namespace mtx {
    // ===============================================================================
    //                                  MatrixConfig                                 =
    // ===============================================================================

    // Statics
    bool MatrixConfig::detailed = false;
    bool MatrixConfig::overwrite = false;
    DisplayPaths MatrixConfig::paths = DisplayPaths::DisplayPaths();

    // Constructors
    MatrixConfig::MatrixConfig() 
    {
        folder = false;
        ioDefined = checkFile(true) && checkFile(false);
        init_color(gray, 500, 500, 500);
        init_pair(10, gray, COLOR_BLACK);
    }

    // TODO: .ini file configuration load from this (or another?) constructor
    MatrixConfig::MatrixConfig(std::string tf, std::string mf, std::string rf, bool ow, bool dt) 
    {
        paths.setFolder(rf);
        paths.setFile(mf);
        paths.setFile(tf);
        overwrite = ow;
        detailed = dt;
        ioDefined = checkFile(true) && checkFile(false);
        init_color(gray, 500, 500, 500);
        init_pair(10, gray, COLOR_BLACK);
    }

    // Toggle overwrite
    void MatrixConfig::toggleOverwrite() 
    {
        overwrite = !overwrite;
    }

    // Toggle detailed mode
    void MatrixConfig::toggleDetailed() 
    {
        detailed = !detailed;
    }

    // Checks if file exist at readFolder
    bool MatrixConfig::checkFile(bool metafile) 
    {

        std::fstream chk(paths.getFullFilepath(metafile), std::fstream::in);
        bool check = chk.is_open();
        chk.close();

        return check;
    }

    // Option parser
    void MatrixConfig::parseopt(int& option)
    {
        if (!folder) {
            if (option == 0) { //init program
                Matrixinator* mtx = new Matrixinator();
                mtx->mainSequence();
                delete mtx;
            }
            else if (option == 1) { //config folders
                setFolders();
                printheader();
                option = 0;
            }
            else if (option == 2) { //toggle overwrite
                toggleOverwrite();
                if (overwrite == false)
                    mvchgat(2, 9, 9, COLOR_PAIR(pck::ERRCOLOR), 120, NULL);
                else
                    mvchgat(2, 9, 9, COLOR_PAIR(pck::OKCOLOR), 121, NULL);
            }
            else if (option == 3) { //toggle detailed
                toggleDetailed();
                if (detailed == false)
                    mvchgat(2, 21, 8, COLOR_PAIR(pck::ERRCOLOR), 120, NULL);
                else
                    mvchgat(2, 21, 8, COLOR_PAIR(pck::OKCOLOR), 121, NULL);
            }
        }
        else {
            if (option == 0) { //current folder
                curs_set(1); noraw(); echo();

                char in[800]; in[0] = '\0';
                mvprintw(8, 0, "Please type a new folder. Leave the field empty to keep the current one, or type 'reset' to reset the folder.\n-> ");
                getnstr(in, 790);

                if (strcmp(in, "reset") == 0)
                    paths.setFolder("");

                else if (in[0] != '\0') {
                    paths.setFolder(in);
                }

                for (int i = 8; i < getmaxy(stdscr); ++i) {
                    move(i, 0); clrtoeol();
                }

                move(3, 30); clrtoeol();
                attron(A_BOLD);
                mvprintw(3, 30, " %s", paths.getVFolder().c_str());
                attroff(A_BOLD);
                refresh();

                curs_set(0); raw(); noecho();
            }
            else if (option == 1 || option == 2) { // 1 = dendrogram .xml | 2 = metadata .csv
                pck::FileSniffer dognose;
                dognose.path = paths.getRFolder();
                (option == 1) ?
                    dognose.extension = "xml" :
                    dognose.extension = "csv";
                dognose.togglePrintMode(); //default filename, we want file + path from root
                std::pair<bool, std::vector<std::string>> results = dognose.find();

                if (results.first == false) {
                    char type[10];
                    (option == 1) ?
                        strcpy_s(type, ".xml\0") :
                        strcpy_s(type, ".csv\0");

                    move(8, 0);
                    pck::printerr("<no %s files found in the specified folder - please try a different path. press any key.>", type);

                    getch();
                    move(8, 0); clrtoeol();
                }
                else {
                    //trim visuals
                    std::vector<std::string> visuals;
                    for (auto& item : results.second) {
                        if (item.length() > paths.getCharLim())
                            visuals.push_back(paths.pathTrimmer(item));
                        else
                            visuals.push_back(item);
                    }

                    attron(COLOR_PAIR(pck::OKCOLOR));
                    mvprintw(8, 0, "%d result(s) found. ", results.second.size());
                    attroff(COLOR_PAIR(pck::OKCOLOR));
                    printw("Please select a file, or press F1 to go back.");

                    pck::DisplayQueue displ(visuals, 10);
                    int choice = displ.qmenu();
                    if (choice >= 0) {
                        paths.setFile(results.second.at(choice));
                        attron(A_BOLD);
                        if (option == 1) {
                            move(3, 30); clrtoeol();
                            mvprintw(3, 30, " %s", paths.getVFolder().c_str());
                            move(4, 30); clrtoeol();
                            mvprintw(4, 30, " %s", paths.getTree().c_str());
                        }
                        else {
                            move(3, 30); clrtoeol();
                            mvprintw(3, 30, " %s", paths.getVFolder().c_str());
                            move(5, 30); clrtoeol();
                            mvprintw(5, 30, " %s", paths.getMeta().c_str());
                        }
                        attroff(A_BOLD);
                    }

                    for (int i = 0; i < 11; ++i) {
                        move(8 + i, 0); clrtoeol();
                    }
                }
            }
            ioDefined = checkFile(true) && checkFile(false);
            move(2, 14); clrtoeol(); move(2, 14);
            (ioDefined) ?
                pck::printok("set.\n") :
                pck::printerr("not set.\n");
        }
    }

    // Print header
    void MatrixConfig::printheader() 
    {
        clear(); move(0, 0); //raw(); curs_set(0); pck::initcolors(); nonl();
        resize_term(0, 0);
        folder = false;
        attron(COLOR_PAIR(115));
        printw("The Matrixinator v%s (%s)\n", MTXVER, MTXREL);      //0
        printw("---------------------------------------------\n");  //1
        attroff(COLOR_PAIR(115));
        statistiks(); //2

        startY = 4;
        opts = {
            "Initialize program",
            "Configure folders and files",
            "Toggle Overwrite",
            "Toggle Detailed mode",
            "Back to Peacock Framework (F1)"
        };
        printopts(opts);
        int any = -1;
        updatepos(any, 0, startY, opts);
        refresh();
    }

    // Main config UI
    void MatrixConfig::mtxMenu() 
    {
        printheader();
        menu(startY, opts);
    }

    // UI for setting folders - does NOT work without curses mode on!
    void MatrixConfig::setFolders() 
    {
        folder = true;
        clear(); move(0, 0);
        attron(COLOR_PAIR(pck::RAINBOW5));
        printw("The Matrixinator - Folder Configuration\n" //0
            "---------------------------------------------\n\n"); //1
        attroff(COLOR_PAIR(pck::RAINBOW5));
        const std::vector<std::string> folderOptions = { //3
            "Current root folder:",
            "Current dendrogram file:",
            "Current metadata file:",
            "Back"
        };
        int any = -1;

        mvprintw(2, 0, "I/O files are ");
        (ioDefined) ?
            pck::printok("set.\n") :
            pck::printerr("not set.\n");

        printopts(folderOptions);
        updatepos(any, 0, 3, folderOptions);
        attron(A_BOLD);
        mvprintw(3, 30, " %s", paths.getVFolder().c_str());
        mvprintw(4, 30, " %s", paths.getTree().c_str());
        mvprintw(5, 30, " %s", paths.getMeta().c_str());
        attroff(A_BOLD);

        menu(3, folderOptions);
    }

    // Returns the state of I/O files existence
    bool MatrixConfig::isIOdefined()
    {
        return ioDefined;
    }

    // Print flags
    void MatrixConfig::statistiks() 
    {
        printw("Flags: | "); //start-end (chars)
        (overwrite) ?
            pck::printok("Overwrite") : //9-18 (9)
            pck::printerr("Overwrite");
        printw(" | ");

        (detailed) ?
            pck::printok("Detailed") : //21-29 (8)
            pck::printerr("Detailed");
        printw(" | ");

        (ioDefined) ?
            pck::printok("I/O Files") : //32-41 (9)
            pck::printerr("I/O Files");

        printw("\n\n");

    }
}