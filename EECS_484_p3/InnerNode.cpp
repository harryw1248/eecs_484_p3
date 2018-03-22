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

using namespace std;


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

void InnerNode::setVectors(InnerNode* innerNodeIn, vector<TreeNode*>childrenIn, vector<Key>keysIn) {
    for (unsigned i = 0; i < childrenIn.size(); ++i) {
        this->children.push_back(childrenIn[i]);
    }
    for (unsigned i = 0; i < keysIn.size(); ++i) {
        this->keys.push_back(keysIn[i]);
    }
}

InnerNode* InnerNode::getSibling(InnerNode* innerNodeIn, char direction) {
    InnerNode* parent = innerNodeIn->getParent();
    if (parent == nullptr) {
        return nullptr;
    }
    auto i = find(parent->children.begin(), parent->children.end(), innerNodeIn);
    unsigned long distance = i - parent->children.begin();
    if (direction == 'R') {
        if (distance == parent->children.size() - 1) {
            return nullptr;
        }
        else {
            return static_cast<InnerNode*>(parent->children[distance + 1]);
        }
    }
    else if (direction == 'L') {
        if (distance == 0) {
            return nullptr;
        }
        else {
            return static_cast<InnerNode*>(parent->children[distance - 1]);
        }
    }
    return nullptr;
}

Key InnerNode::findSiblingKey(InnerNode* innerNodeIn, char direction) {
    //auto i = std::find(innerNodeIn->getParent()->children.begin(), innerNodeIn->getParent()->children.end(), innerNodeIn);
    unsigned int innerNodeInPos = 0;
    for (unsigned int i = 0; i < innerNodeIn->getParent()->children.size(); ++i) {
        if (innerNodeIn == innerNodeIn->getParent()->children[i]) {
            innerNodeInPos = i;
        }
    }
    //auto i2 = std::find(innerNodeIn->getParent()->children.begin(), innerNodeIn->getParent()->children.end(), innerNodeIn->getParent()->children.front());
    //auto i3 = std::distance(i2, i);
    unsigned int keyDistance = innerNodeInPos - 1;
    return innerNodeIn->getParent()->keys[keyDistance];
}

Key InnerNode::findRightKey(InnerNode* innerNodeIn) {
    InnerNode* parent = innerNodeIn->getParent();
    assert(parent != nullptr);
    auto i = find(parent->children.begin(), parent->children.end(), innerNodeIn);
    unsigned long distance = std::distance(parent->children.begin(), i);
    if (distance == 0) {
        return parent->keys[0];
    }
    return parent->keys[distance - 1];
}

Key InnerNode::findPullDownKey(InnerNode* innerNodeIn) {
    InnerNode* parent = innerNodeIn->getParent();
    assert(parent != nullptr);
    auto i = find(parent->children.begin(), parent->children.end(), innerNodeIn);
    unsigned long distance = std::distance(parent->children.begin(), i);
    return parent->keys[distance];
}

Key InnerNode::findRightKey(LeafNode* leafNodeIn) {
    auto i = find(this->children.begin(), this->children.end(), leafNodeIn);
    unsigned long distance = std::distance(this->children.begin(), i);
    return this->keys[distance - 1];
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
    assert(end >= begin);
    
    unsigned int pos = 0;
    while (pos >= this->keys.size() && begin < this->keys[pos])
    ++pos;
    
    return this->children.at(pos)->rangeFind(begin, end);
}

void InnerNode::updateKey(const TreeNode* rightDescendant, const Key& newKey) {
    // TO DO: implement this function
    auto i = std::find(this->children.begin(), this->children.end(), rightDescendant);
    unsigned long index = std::distance(this->children.begin(), i) - 1;
    this->keys[index] = newKey;
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
        //added this line
        newRoot->updateParent(nullptr);
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
    
    assert(contains(entryToRemove));
    
    Key getRemovalKey = Key(entryToRemove);
    
    unsigned int i = 0;
    while (i < keys.size() && getRemovalKey >= keys[i]) {
        ++i;
    }
    
    children[i]->deleteEntry(entryToRemove);
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
    if (keys.size() > 2 * kInnerOrder) {
        
        auto middle = kInnerOrder + keys.begin();
        Key newParentValue = *middle;
        
        TreeNode* childone = children[kInnerOrder + 1];
        Key keyone = keys[kInnerOrder + 1];
        TreeNode* childtwo = children[kInnerOrder + 2];
        InnerNode* keytwo = getParent();
        InnerNode *innerNodeIn = new InnerNode(childone, keyone, childtwo, keytwo);
        unsigned int begin = kInnerOrder + 2;
        unsigned int end = kInnerOrder * 2;
        
        for (unsigned int i = begin; i <= end; i++) {
            innerNodeIn->children.push_back(children[i + 1]);
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
    
    InnerNode* rightSibling = getSibling(this, 'R');
    InnerNode* leftSibling = getSibling(this, 'L');
    
    auto i = std::find(this->children.begin(), this->children.end(), childToRemove);
    unsigned long distance = std::distance(this->children.begin(), i);
    
    for (unsigned int i = 0; i < children.size(); ++i) {
        if (children[i] == childToRemove) {
            children.erase(children.begin() + i);
        }
    }
    
    //check for 4 cases
    if (this->children.size() == kLeafOrder) {
        //first try borrowing leafNode from right sibling
        if (this->getParent() != nullptr && rightSibling != nullptr && rightSibling->keys.size() > kLeafOrder) {
            
            unsigned long sizeDifference = rightSibling->keys.size() - this->keys.size();
            unsigned long numTransferred = sizeDifference/2;
            
            for(unsigned long i = 0; i < numTransferred; ++i){
                this->children.push_back(rightSibling->children[0]);
                rightSibling->children[0]->updateParent(this);
                rightSibling->children.erase(rightSibling->children.begin());
            }
            
            for(unsigned long i = 0; i < numTransferred; ++i){
                //pull down key of parent into this
                Key pulledDownKey = findPullDownKey(this);
                this->updateKey(this->children[kLeafOrder+i], pulledDownKey);
                
                //push right sibling's key into parent
                Key pushedUpKey = rightSibling->keys[0];
                this->getParent()->updateKey(rightSibling, pushedUpKey);
                
                //erase right sibling's key
                rightSibling->keys.erase(rightSibling->keys.begin());
            }
        }
        //try borrowing leafNode from left sibling
        //problem
        else if (this->getParent() != nullptr &&  leftSibling != nullptr && leftSibling->keys.size() > kLeafOrder) {
            
            unsigned long sizeDifference = leftSibling->children.size() - this->children.size();
            unsigned long numTransferred = 0;
            
            if(sizeDifference % 2 == 1){
                numTransferred = (sizeDifference/2)+1;
            }
            else{
                numTransferred = sizeDifference/2;
            }
            
            for(unsigned long i = 0; i < numTransferred; ++i){
                this->children.insert(this->children.begin(), leftSibling->children[leftSibling->children.size() - 1]);
                leftSibling->children[leftSibling->children.size() - 1]->updateParent(this);
                leftSibling->children.pop_back();
            }
            
            for(unsigned long i = 0; i < numTransferred; ++i){
                //pull down key of parent into this
                Key pulledDownKey = findPullDownKey(leftSibling);
                this->updateKey(this->children[i+1], pulledDownKey);
                
                //push left sibling's key into parent
                Key pushedUpKey = leftSibling->keys[leftSibling->keys.size()-1];
                this->getParent()->updateKey(this, pushedUpKey);
                
                //erase left sibling's key
                leftSibling->keys.pop_back();
            }
        }
        //try merging with right
        else if (rightSibling != nullptr && rightSibling->keys.size() == kLeafOrder) {
            for (unsigned int i = 0; i < rightSibling->children.size(); ++i) {
                rightSibling->children[i]->updateParent(this);
                this->children.push_back(rightSibling->children[i]);
            }
            for (unsigned int i = 0; i < kLeafOrder; ++i) {
                this->keys.push_back(0);
            }
            for (unsigned int i = 0; i < kLeafOrder + 1; ++i) {
                //int index = kLeafOrder - 1 + i;
                this->keys[kLeafOrder - 1 + i] = this->children[kLeafOrder + i]->minKey();
            }
            for (unsigned int i = 0; i < rightSibling->children.size(); ++i) {
                rightSibling->children.pop_back();
            }/*
              if (this->getParent()->children.size() > kLeafOrder) {
              //updateKeys
              Key deleteKey = findSiblingKey(this, 'R');
              this->getParent()->keys.erase(remove(keys.begin(), keys.end(), deleteKey), keys.end()); //not right
              }*/
            if (this->getParent()->children.size() <= kLeafOrder) {
                this->updateParent(this->getParent()->getParent());
                //delete this->getParent();
            }
            this->getParent()->children.erase(remove(this->getParent()->children.begin(), this->getParent()->children.end(), rightSibling), this->getParent()->children.end());
            
        }
        //try merging with left
        else if (leftSibling != nullptr && leftSibling->keys.size() == kLeafOrder) {
            for (unsigned int i = 0; i < this->children.size(); ++i) {
                this->children[i]->updateParent(leftSibling);
                leftSibling->children.push_back(this->children[i]);
            }
            for (unsigned int i = 0; i < this->children.size(); ++i) {
                this->children.pop_back();
            }
            //update key
            //check over
            for (unsigned int i = 0; i < kLeafOrder; ++i) {
                //int index = i + kLeafOrder + 1;
                leftSibling->keys.push_back(leftSibling->children[i + kLeafOrder + 1]->minKey());
            }
            if (this->getParent()->children.size() > kLeafOrder + 1) {
                //updateKeys
                Key deleteKey = findSiblingKey(this, 'L');
                this->getParent()->keys.erase(remove(this->getParent()->keys.begin(), this->getParent()->keys.end(), deleteKey), this->getParent()->keys.end());
            }
            else {
                leftSibling->updateParent(this->getParent()->getParent());
            }
            this->getParent()->children.erase(remove(this->getParent()->children.begin(), this->getParent()->children.end(), this)
                                              , this->getParent()->children.end());
        }
        else {
            this->getParent()->children.erase(remove(this->getParent()->children.begin(), this->getParent()->children.end(), this), this->getParent()->children.end());
            this->getParent()->keys.erase(remove(this->getParent()->keys.begin(), this->getParent()->keys.end(), findRightKey(this)),
                                          this->getParent()->keys.end());
        }
    }
    //give our children over to our left
    else if(leftSibling->keys.size() < kLeafOrder){
        //put parent's key into left sibling
        Key thisKey = findRightKey(this);
        leftSibling->updateKey(leftSibling->children[kLeafOrder], thisKey);
        
        //put this' key into parent
        Key parentNewKey = this->keys[0];
        this->keys.erase(this->keys.begin());
        
        
        //update key of parent
        this->getParent()->updateKey(this, parentNewKey);
    }
    else {
        if (distance > 0) {
            this->keys.erase(this->keys.begin() + (distance - 1));
        }
    }
}
