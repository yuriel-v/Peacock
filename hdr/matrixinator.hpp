/*
 * Plugin file: The Matrixinator v1.1b for the Peacock Framework
 * 
 * Author: (c) Leonardo Valim & Joshua Walker
 * 
 * This file contains the declaration of classes to be used in the Matrixinator.
 * That's it. More info in the class definition themselves.
*/
#ifndef MATRIXINATOR_HPP
#define MATRIXINATOR_HPP

#include <vector>
#include <string>
#include <array>
#include <set>
#include "pckcore.hpp"

namespace mtx {
    constexpr short gray = 10;
    constexpr char MTXVER[] = "1.1\0";
    constexpr char MTXREL[] = "Beta\0";

    /* ============================================================================== *
     * DisplayPaths class                                                             *
     *                                                                                *
     * Auxiliary class for displaying the root folder, metadata and spreadsheet in an *
     * abbreviated manner, for less console pollution.                                *
     *                                                                                *
     * Will not lose the real path values.                                            *
     * ============================================================================== */

    class DisplayPaths {
    private:
        std::string treeFile;
        std::string metaFile;
        std::string readFolder;
        std::string treeRoot;   // path leading from readFolder to treeFile
        std::string metaRoot;   // path leading from readFolder to metaFile
        unsigned charLim;

    public:
        DisplayPaths();
        DisplayPaths(std::string mf, std::string tf, std::string rf);

        unsigned getCharLim();
        std::string getTree();
        std::string getMeta();

        std::string getVFolder();
        std::string getRFolder();

        std::string getFullFilepath(bool metafile);
        std::string getLeadingPath(bool metafile);

        void setCharLim(unsigned limit);
        void setFile(std::string file);
        void setFolder(std::string rf);
        std::string pathTrimmer(std::string path, unsigned charlimit = 0);
        std::vector<std::string> the_abbrevi8r_9001(std::vector<std::string> longstring, unsigned charlimit = 0);
        bool isUndefined(const std::string& str);
    };

    /* ============================================================================== *
     * MatrixConfig class                                                             *
     *                                                                                *
     * This class is pretty self-explanatory: it contains configuration parameters    *
     * for the Matrixinator to function. Its intended use is through the Peacock      *
     * Framework, where it may be properly configured and visualized.                 *
     *                                                                                *
     * Here is the starting point for the execution of the Matrixinator.              *
     * ============================================================================== */

    class MatrixConfig : public pck::Menu {
    private:

        std::vector<std::string> opts;
        bool ioDefined;
        int startY = 0;
        bool folder; //distinguishes parsing options from the folder set or main menu

        void toggleOverwrite();
        void toggleDetailed();
        void setFolders();             //folder search menu
        bool checkFile(bool);
        void parseopt(int& option);
        void statistiks();
        void printheader();

    protected:
        static DisplayPaths paths;
        static bool overwrite;
        static bool detailed;

        bool isIOdefined();

    public:
        MatrixConfig();
        MatrixConfig(std::string tf, std::string mf, std::string rf, bool ow = false, bool dt = false);

        void mtxMenu();
    };

    /* ============================================================================== *
     * Metadata class                                                                 *
     *                                                                                *
     * This is a data structure class imply meant to hold data described in the .csv  *
     * spreadsheet passed to the main class Matrixinator in its config class.         *
     * It then becomes altered in memory by the program's processing routines and     *
     * eventually printed into another .csv file at the end of the run.               *
     *                                                                                *
     * For title purposes, the 20 data fields are, in index order:                    *
     * Key, Location, CollectionDate, Company, FSGID, Farm, Age_days, SampleOrigin,   *
     * SampleType, VMP, ibeA, traT, iutA, ompT, sitA, irp2, cvaC, tsh, iucC, iss.     *
     *                                                                                *
     * The 8 octagon fields are:                                                      *
     * BS22, BS15, BS3, BS8, BS27, BS84, BS18, BS278.                                 *
     * ============================================================================== */

    class Metadata {
    private:
        std::vector<double> octagon;
        std::vector<std::wstring> data;
        std::vector<std::pair<std::wstring, double>> usaMatches;
        int nodeNumber;

    public:
        Metadata();

        void setData(const std::vector<std::wstring> datafield);
        void setOctagon(const std::vector<double> origin);
        void setOctagon(const double *vals);
        void setOctagon(std::array<double, 8> origin);
        void appendMatch(std::wstring id, double sim);
        void appendMatch(std::pair<std::wstring, double> origin);
        void clearMatches() { usaMatches.clear(); }
        void associate(int node);

        std::vector<std::pair<std::wstring, double>> getMatches();
        std::vector<std::wstring> getData();
        std::vector<double> getOctagon();
        int getNode();
        bool nullOct();
        void copyOct(const Metadata& origin);

    };

    /* ============================================================================== *
     * Tree class                                                                     *
     *                                                                                *
     * This is another data structure class meant to hold nodes parsed from the .xml  *
     * dendrogram pointed to by the Matrixinator's config class.                      *
     *                                                                                *
     * It also holds info on whether or not the current node is a sample.             *
     *                                                                                *
     * Please note that despite the name, one instance of this class represents one   *
     * single dendrogram node!                                                        *
     * ============================================================================== */

    class Tree {
    private:
        std::pair<int, int> IDs; //ID and parentID
        double similarity;
        std::set<int> childList;
        bool sample;

    public:
        Tree();
        Tree(std::pair<int, int> IDs, double sim, bool sample);

        void setIDs(int, int);
        void setIDs(std::pair<int, int>);
        void setSim(double);
        void addChild(int);
        void makeSample();

        int getID();
        int getParentID();
        double getSim();
        std::set<int> getChildren();
        bool isSample();
    };

    /* ============================================================================== *
     * Matrixinator class                                                             *
     *                                                                                *
     * This is the program's main class. It inherits some stuff from its config class *
     * and uses that info to work. That's it.                                         *
     * ============================================================================== */

    class Matrixinator : private MatrixConfig {
    private:
        std::vector<Metadata> SS;
        std::vector<Tree> acacia;
        std::vector<int> USAsamples;
        int numNodes;
        int numSamples;

        void init();
        void postinit();
        void sweep();
        void output();
        void closing(int code, WINDOW* mtxcon);

        void bulldozer(int node);
        double bullSim(int node, int origin);
        bool isUS(int id);

    public:
        Matrixinator();

        static std::vector<std::wstring> w_sliceNsplice(const std::wstring& wstr, char delim = ' ');
        void mainSequence();
    };
}

#endif //MATRIXINATOR_HPP