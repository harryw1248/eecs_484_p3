#include "DataEntry.h"                                  // for DataEntry
#include "InnerNode.h"                                  // for InnerNode
#include "LeafNode.h"                                   // file-specific header
#include "TreeNode.h"                                   // for TreeNode
#include "Utilities.h"                                  // for size constants, print prefix, Key alias
#include <algorithm>                                    // for find
#include <cassert>                                      // for assert
#include <iostream>                                     // for ostream
#include <numeric>                                      // for numeric_limits
#include <string>                                       // for string
#include <vector>                                       // for vector

using std::find;
using std::vector;
using std::numeric_limits;
using std::ostream;
using std::string;


// constructor
LeafNode::LeafNode(InnerNode* parent)
    : TreeNode{ parent }, entries{} {}

// print keys of data entries surrounded by curly braces, ending
// newline
void LeafNode::print(ostream& os, int indent) const {
    assert(indent >= 0);

    os << kPrintPrefix << string(indent, ' ') << "{ ";
    for (const auto& entry : entries) {
        if (entry != entries[0]) {
            os << " | ";
        }
        os << entry;
    }
    os << " }\n";

    assert(satisfiesInvariant());
}

// data entries are sorted; minimum is first entry's key
Key LeafNode::minKey() const {
    if (entries.empty()) {
        return numeric_limits<Key>::min();
    }
    return entries.front();
}

// data entries are sorted; maximum is last entry's key
Key LeafNode::maxKey() const {
    if (entries.empty()) {
        return numeric_limits<Key>::max();
    }
    return entries.back();
}

// TRUE if key is the key of any entry
bool LeafNode::contains(const Key& key) const {
    return (find(entries.cbegin(), entries.cend(), key) != entries.cend());
}

// TRUE if this node is the target
bool LeafNode::contains(const TreeNode* node) const {
    return (this == node);
}

// return the data entry with given key
const DataEntry& LeafNode::operator[](const Key& key) const {
    assert(contains(key));

    return *find(entries.cbegin(), entries.cend(), key);
}

vector<DataEntry> LeafNode::rangeFind(const Key& begin, const Key& end) const {
    // TO DO: implement this function
    return vector<DataEntry>{};
}

// use generic delete; height can't decrease
TreeNode* LeafNode::deleteFromRoot(const DataEntry& entryToRemove) {
    assert(contains(entryToRemove));
    assert(!getParent());

    deleteEntry(entryToRemove);
    assert(satisfiesInvariant());
    return this;
}

void LeafNode::insertEntry(const DataEntry& newEntry) {
    // TO DO: implement this function
    if(entries.size() == 2*kLeafOrder){
        
    }
    else{
        entries.push_back(newEntry);
    }
    
    
}

void LeafNode::deleteEntry(const DataEntry& entryToRemove) {
    // TO DO: implement this function
}
