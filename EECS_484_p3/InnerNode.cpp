#include "DataEntry.h"                                  // for DataEntry
#include "InnerNode.h"                                  // file-specific header
#include "TreeNode.h"                                   // for TreeNode
#include "Utilities.h"                                  // for size constants, print prefix, Key alias
#include <algorithm>                                    // for any_of
#include <cassert>                                      // for assert
#include <iostream>                                     // for ostream
#include <string>                                       // for string
#include <vector>                                       // for vector

using std::any_of;
using std::vector; using std::string;
using std::ostream;


// value constructor
InnerNode::InnerNode(TreeNode* child1, const Key& key, TreeNode* child2, InnerNode* parent)
: TreeNode{ parent }, keys{ key }, children{ child1, child2 } {
    
    assert(child1 && child2);
    assert(*child1 < key && *child2 >= key);
    
    child1->updateParent(this);
    child2->updateParent(this);
}

class sortInvariant {
public:
    bool operator()(const TreeNode* rhs, const TreeNode* lhs) {
        
        return rhs->minKey() < lhs->minKey();
    }
};

//InnerNode::InnerNode(vector<TreeNode*> childs, vector<Key> keyer, InnerNode *parent)
//: TreeNode{parent}, keys {keyer}, children {childs} {
//
//    for(auto &kid : childs){
//        assert(kid != nullptr);
//        kid -> updateParent(this);
//    }
//}

void InnerNode::setVectors(InnerNode* innerNodeIn, vector<TreeNode*>childrenIn, vector<Key>keysIn) {
    for (unsigned i = 0; i < childrenIn.size(); ++i) {
        this->children.push_back(childrenIn[i]);
    }
    for (unsigned i = 0; i < keysIn.size(); ++i) {
        this->keys.push_back(keysIn[i]);
    }
}

// deallocate all children
InnerNode::~InnerNode() {
    for (auto child : children) {
        delete child;
    }
}

// print keys, then each node on its own line
void InnerNode::print(ostream& os, int indent) const {
    assert(indent >= 0);
    
    os << kPrintPrefix << string(indent, ' ') << "[ ";
    for (const auto& key : keys) {
        if (key != keys[0]) {
            os << " | ";
        }
        os << key;
    }
    os << " ]\n";
    
    for (const auto child : children) {
        child->print(os, indent + kIndentIncr);
    }
    
    assert(satisfiesInvariant());
}

// ask first child, which must exist
Key InnerNode::minKey() const {
    return children.front()->minKey();
}

// ask last child, which must exist
Key InnerNode::maxKey() const {
    return children.back()->maxKey();
}

// ask the child where the data entry with that key would be
bool InnerNode::contains(const Key& key) const {
    return (
            any_of(children.cbegin(), children.cend(), [key](auto n)->bool { return n->contains(key); }));
}

// ask children if they contain
bool InnerNode::contains(const TreeNode* node) const {
    return ((this == node) ||
            any_of(children.cbegin(), children.cend(), [node](auto n)->bool { return n->contains(node); }));
}

// ask the child where the data entry with that key is
const DataEntry& InnerNode::operator[](const Key& key) const {
    assert(contains(key));
    
    for (const auto child : children) {
        if (child->contains(key)) {
            return child->operator[](key);
        }
    }
    assert(false);
}

vector<DataEntry> InnerNode::rangeFind(const Key& begin, const Key& end) const {
    // TO DO: implement this function
    return vector<DataEntry>{};
}

void InnerNode::updateKey(const TreeNode* rightDescendant, const Key& newKey) {
    // TO DO: implement this function
}

// use generic delete, then look at number of children to determine
// if height decreased
TreeNode* InnerNode::deleteFromRoot(const DataEntry& entryToRemove) {
    assert(!getParent());
    assert(contains(entryToRemove));
    
    deleteEntry(entryToRemove);
    if (children.size() == 1) {                 // one child means height has shrunk
        auto newRoot = children.front();
        children.clear();                       // clear children so not deallocated later
        assert(satisfiesInvariant());
        return newRoot;
    }
    assert(satisfiesInvariant());
    return this;
}

void InnerNode::insertEntry(const DataEntry& newEntry) {
    // TO DO: implement this function
    //keep tracing down using inner node keys
    assert(!contains(newEntry));
    for (unsigned i = 0; i < keys.size(); ++i) {
        if (newEntry < keys[i]) {
            this->children[i]->insertEntry(newEntry);
            assert((keys.size() <= 2 * kInnerOrder && (!getParent() || (keys.size() >= kInnerOrder))) || !getParent());
            return;
        }
    }
    children.back()->insertEntry(newEntry);
    assert((keys.size() <= 2 * kInnerOrder && (!getParent() || (keys.size() >= kInnerOrder))) || !getParent());
}

void InnerNode::deleteEntry(const DataEntry& entryToRemove) {
    // TO DO: implement this function
}

void InnerNode::insertChild(TreeNode* newChild, const Key& key) {
    // TO DO: implement this function
    assert((newChild != nullptr) && newChild->minKey() >= key);
    
    //update key in inner node (look at lecture slides)
    auto upper_bound = std::upper_bound(keys.begin(), keys.end(), key);
    auto i = children.begin() + (upper_bound - keys.begin()) + 1;
    keys.insert(upper_bound, key);
    children.insert(i, newChild);
    
    //check if we need to split inner node
    if (keys.size() > 2*kInnerOrder) {
        
        auto middle = kInnerOrder + keys.begin();
        Key newParentValue = *middle;
        
        TreeNode* childone = children[kInnerOrder+1];
        Key keyone = keys[kInnerOrder+1];
        TreeNode* childtwo = children[kInnerOrder+2];
        InnerNode* keytwo = getParent();
        InnerNode *innerNodeIn = new InnerNode(childone, keyone, childtwo, keytwo);
        unsigned int begin = kInnerOrder + 2;
        unsigned int end = kInnerOrder*2;
        
        for (unsigned int i = begin; i <= end; i++) {
            innerNodeIn->children.push_back(children[i+1]);
            innerNodeIn->keys.push_back(keys[i]);
            innerNodeIn->children.back()->updateParent(innerNodeIn);
        }
        for (unsigned int i = 0; i < kInnerOrder + 1; i++) {
            children.pop_back();
            keys.pop_back();
        }
        if (!getParent()) {
            InnerNode *createParent = new InnerNode(this, newParentValue, innerNodeIn);
            innerNodeIn->updateParent(createParent);
            updateParent(createParent);
        }
        else {
            getParent()->insertChild(innerNodeIn, newParentValue);
        }
    }
}

void InnerNode::deleteChild(TreeNode* childToRemove) {
    
    // TO DO: implement this function
    
    //case where tree height decreases
    if(this->keys.size() == 1){
        
    }
    else{
        auto i = std::find(this->children.begin(),this->children.end(),childToRemove);
        unsigned long distance = this->children.begin() - i;
        this->children.erase(i);
        this->keys.erase(this->keys.begin() + (distance-1));
    }
}
