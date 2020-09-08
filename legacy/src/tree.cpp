#include "matrixinator.hpp"

namespace mtx {

    // ===============================================================================
    //                                      Tree                                     =
    // ===============================================================================

	// Constructors
    Tree::Tree() 
    {
        IDs = { 0, 0 };
        similarity = 0;
        sample = false;
    }
    Tree::Tree(std::pair<int, int> IDs, double sim, bool sample) 
    {
        this->IDs = IDs;
        similarity = sim;
        this->sample = sample;
    }

    // Setters

    // Sets the ID and parentID, respectively
    void Tree::setIDs(int ID, int parentID)
    {
        IDs.first = ID;
        IDs.second = parentID;
    }
    void Tree::setIDs(std::pair<int, int> origin)
    {
        IDs = origin;
    }

    // Sets the similarity value of this particular node
    void Tree::setSim(double sim)
    {
        similarity = sim;
    }

    // Adds an ID to this node's child list
    void Tree::addChild(int childID) 
    {
        childList.insert(childID);
    }

    // Toggles whether or not this is a sample node
    void Tree::makeSample()
    {
        sample = !sample;
    }

    // Getters

    // Returns the node's ID
    int Tree::getID()
    {
        return IDs.first;
    }

    // Returns the node's parent's ID
    int Tree::getParentID() 
    {
        return IDs.second;
    }

    // Returns the node's similarity value
    double Tree::getSim() 
    {
        return similarity;
    }

    // Returns the list of children for this node
    std::set<int> Tree::getChildren() 
    {
        return childList;
    }

    // Returns true if the current node is a sample, false otherwise
    bool Tree::isSample() 
    {
        return sample;
    }


}