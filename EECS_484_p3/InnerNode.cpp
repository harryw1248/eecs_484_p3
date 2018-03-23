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
    
    unsigned long distance = 0;
    
    for (unsigned int i = 0; i < children.size(); ++i) {
        if (children[i] == childToRemove) {
            distance = i;
            children.erase(children.begin() + i);
        }
    }
    
    //check for 4 cases
    if (this->children.size() == kLeafOrder) {
        //first try borrowing leafNode from right sibling
        if (this->getParent() != nullptr && rightSibling != nullptr && rightSibling->keys.size() > kLeafOrder) {
            
            if (distance > 0) {
                unsigned long deletePos = distance - 1;
                this->keys.erase(this->keys.begin() + deletePos);
            }
            else {
                this->keys.erase(this->keys.begin());
            }
            
            unsigned long sizeDifference = rightSibling->keys.size() - this->keys.size();
            unsigned long numTransferred = sizeDifference / 2;
            
            for (unsigned long i = 0; i < numTransferred; ++i) {
                
                this->children.push_back(rightSibling->children[0]);
                rightSibling->children[0]->updateParent(this);
                rightSibling->children.erase(rightSibling->children.begin());
                
                //pull down key of parent into this
                Key pulledDownKey = findPullDownKey(this);
                this->keys.push_back(pulledDownKey);
                
                this->updateKey(this->children[this->children.size()-1], pulledDownKey);
                //check if pulled down key needs to be updated
                if(this->children[this->children.size()-2]->minKey() < this->keys[this->keys.size()-1]){
                    this->updateKey(this->children[this->children.size()-1], this->children[this->children.size()-1]->minKey());
                }
                
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
            
            if (distance > 0) {
                unsigned long deletePos = distance - 1;
                this->keys.erase(this->keys.begin() + deletePos);
            }
            else {
                this->keys.erase(this->keys.begin());
            }
            
            
            unsigned long sizeDifference = leftSibling->children.size() - this->children.size();
            unsigned long numTransferred = 0;
            
            if (sizeDifference % 2 == 1) {
                numTransferred = (sizeDifference / 2) + 1;
            }
            else {
                numTransferred = sizeDifference / 2;
            }
            
            for (unsigned long i = 0; i < numTransferred; ++i) {
                
                this->children.insert(this->children.begin(), leftSibling->children[leftSibling->children.size() - 1]);
                leftSibling->children[leftSibling->children.size() - 1]->updateParent(this);
                leftSibling->children.pop_back();
                
                //pull down key of parent into this
                Key pulledDownKey = findPullDownKey(leftSibling);
                this->keys.insert(keys.begin(),pulledDownKey);
                
                //check if pulled down key needs to be updated
                if(this->children[0]->minKey() < this->keys[0]){
                    this->updateKey(this->children[1], this->children[1]->minKey());
                }
                
                //push left sibling's key into parent
                Key pushedUpKey = leftSibling->keys[leftSibling->keys.size() - 1];
                this->getParent()->updateKey(this, pushedUpKey);
                
                //erase left sibling's key
                leftSibling->keys.pop_back();
            }
        }
        //try merging with right
        else if (rightSibling != nullptr && rightSibling->keys.size() == kLeafOrder) {
            if (distance > 0) {
                unsigned long deletePos = distance - 1;
                this->keys.erase(this->keys.begin() + deletePos);
            }
            else {
                this->keys.erase(this->keys.begin());
            }
            this->merger();
        }
        //try merging with left
        else if (leftSibling != nullptr && leftSibling->keys.size() == kLeafOrder) {
            if (distance > 0) {
                unsigned long deletePos = distance - 1;
                this->keys.erase(this->keys.begin() + deletePos);
            }
            else {
                this->keys.erase(this->keys.begin());
            }
            leftSibling->merger();
        }
    }
    else {
        if (distance > 0) {
            unsigned long deletePos = distance - 1;
            this->keys.erase(this->keys.begin() + deletePos);
        }
        else {
            this->keys.erase(this->keys.begin());
        }
    }
}

Key InnerNode::getKey() {
    auto getParent = this->getParent();
    
    unsigned int i = 0;
    while (getParent->children.at(i) != this)
    i++;
    
    return getParent->keys[--i];
}

void InnerNode::merger() {
    
    auto sibling = this->getSibling(this, 'R');
    for(unsigned long i = 0; i < kLeafOrder+1; ++i){
        
        //transfer child
        this->children.push_back(sibling->children[0]);
        sibling->children[0]->updateParent(this);
        sibling->children.erase(sibling->children.begin());
        
        //pull key from parent
        this->keys.push_back(sibling->getKey());
        
        //find this position in parent
        unsigned long posInParent = 0;
        for(unsigned long i = 0; i < this->getParent()->children.size(); ++i){
            if(this->getParent()->children[i] == this){
                posInParent = i;
            }
        }
        
        unsigned long keyInParentPos = posInParent;
        
        //update parent's key
        if(sibling->keys.size() != 0){
            this->getParent()->keys[keyInParentPos] = sibling->keys[0];
            sibling->keys.erase(sibling->keys.begin());
        }
        else{
            delete sibling;
            this->getParent()->keys.erase(this->getParent()->keys.begin()+keyInParentPos);
            this->getParent()->children.erase(this->getParent()->children.begin()+keyInParentPos+1);
            return;
        }
        
        //check if key of this has to be updated
        if(this->children[this->keys.size()-1]->minKey() < this->keys[keys.size()-1] ){
        }
        else{
            this->keys[keys.size()-1] = this->children[keys.size()]->minKey();
        }
    }
    delete sibling;

}
