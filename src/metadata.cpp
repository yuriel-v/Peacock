#include "matrixinator.hpp"

namespace mtx {

    // ===============================================================================
    //                                    Metadata                                   =
    // ===============================================================================

    // Constructors
    Metadata::Metadata() 
    {
        std::vector<double> temp(8, -2);
        octagon = temp;
    }

    // Setters

    // Copies one index's octagon to the caller
    void Metadata::copyOct(const Metadata& origin)
    {
        octagon = origin.octagon;
    }

    // Sets data values (20 wide-string fields)
    void Metadata::setData(const std::vector<std::wstring> datafield) 
    {
        if (datafield.size() < 20) return;
        data.resize(20);
        std::vector<std::wstring>::const_iterator it = datafield.begin();
        std::vector<std::wstring>::iterator jt = data.begin();

        for (int i = 0; i < 20; ++i) {
            *jt = *it;
            ++it; ++jt;
        }
    }

    // Sets octagon values
    void Metadata::setOctagon(const std::vector<double> origin) 
    {
        //this.octagon is guaranteed to be size 8 by the constructor
        if (origin.size() != 8)
            return;
        else {
            std::vector<double>::const_iterator it = origin.begin();
            for (auto& value : octagon) {
                value = *it;
                ++it;
            }
        }
    }
    void Metadata::setOctagon(const double *origin)
    {
        if (sizeof(origin) / sizeof(double) != 8)
            return;
        else {
            for (int i = 0; i < 8; ++i)
                octagon.push_back(origin[i]);
        }
    }
    void Metadata::setOctagon(std::array<double, 8> origin)
    {
        octagon.clear();
        for (auto& value : origin)
            octagon.push_back(value);
    }

    // Appends a match to the matches list
    void Metadata::appendMatch(std::wstring id, double sim) 
    {
        usaMatches.push_back(std::make_pair(id, sim));
    }
    void Metadata::appendMatch(std::pair<std::wstring, double> origin)
    {
        usaMatches.push_back(origin);
    }

    // Associates a sample with a specific node in the dendrogram
    void Metadata::associate(int node)
    {
        nodeNumber = node;
    }

    // Getters

    // Returns data/octagon values
    std::vector<std::wstring> Metadata::getData()
    {
        return data;
    }
    std::vector<double> Metadata::getOctagon() 
    {
        return octagon;
    }

    // Returns this sample's node number
    int Metadata::getNode() 
    {
        return nodeNumber;
    }

    // Returns this sample's match list
    std::vector<std::pair<std::wstring, double>> Metadata::getMatches()
    {
        return usaMatches;
    }

    // If all octagon values are -1, returns true; false otherwise
    bool Metadata::nullOct()
    {
        std::vector<double> nullOct = { -2, -2, -2, -2, -2, -2, -2, -2 };
        std::vector<double>::const_iterator it = nullOct.begin();
        int numMatches = 0;

        for (auto& val : octagon) {
            if (val == *it)
                ++numMatches;
            ++it;
        }

        return numMatches == 8;
    }
}