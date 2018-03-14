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

//InnerNode::InnerNode(vector<TreeNode*> childs, vector<Key> keyer, InnerNode *parent)
//: TreeNode{parent}, keys {keyer}, children {childs} {
//    
//    for(auto &kid : childs){
//        assert(kid != nullptr);
//        kid -> updateParent(this);
//    }
//}

void InnerNode::setVectors(InnerNode* innerNodeIn, vector<TreeNode*>childrenIn, vector<Key>keysIn){
    for(int i = 0; i < childrenIn.size(); ++i){
        this->children.push_back(childrenIn[i]);
    }
    for(int i = 0; i < keysIn.size(); ++i){
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
}

void InnerNode::deleteEntry(const DataEntry& entryToRemove) {
    // TO DO: implement this function
}

void InnerNode::insertChild(TreeNode* newChild, const Key& key) {
    // TO DO: implement this function
    assert((newChild != nullptr) && newChild->minKey() >= key);
    
    //update key in inner node (look at lecture slides)
    auto lower_bound = std::lower_bound(keys.begin(),keys.end(),key);
    keys.insert(lower_bound,key);
    
    auto i = children.begin() + (lower_bound - keys.begin()) + 1;
    newChild->updateParent(this);
    children.insert(i,newChild);
    
    //check if we need to split inner node
    if(keys.size() > 2*kInnerOrder+1){
        
        unsigned long middle = (children.size()-1)/2;
        Key newParentValue = this->keys[middle];
        
        vector<Key> newKeys;
        vector<TreeNode *> newChildren;
        
        //push correct children into newly created innerNode
        for(unsigned long i = middle+1; i < children.size(); ++i){
            newChildren.push_back(children[i]);
        }
        
        //remove the recently pushed children from current innerNode
        for(unsigned long i = middle+1; i < children.size(); ++i){
            this->children.pop_back();
        }
        
        for(unsigned long i = middle+1; i < keys.size(); ++i){
            newKeys.push_back(keys[i]);
        }
        
        for(unsigned long i = middle+1; i < keys.size(); ++i){
            this->keys.pop_back();
        }
        
        
        //how would you construct this
        InnerNode *newInnerNode = new InnerNode(newChildren[0],newParentValue,newChild,this->getParent());
        setVectors(newInnerNode,newChildren,newKeys);
        
        if (!getParent()) {
            InnerNode* createParent = new InnerNode(this,newParentValue,newInnerNode);
            updateParent(createParent);
            newInnerNode->updateParent(createParent);
        }
        
        else {
            getParent()->insertChild(newInnerNode,newParentValue);
        }
    }

}

void InnerNode::deleteChild(TreeNode* childToRemove) {
    // TO DO: implement this function
}
