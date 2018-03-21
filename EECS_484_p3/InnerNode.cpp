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

InnerNode* InnerNode::getSibling(InnerNode* innerNodeIn, char direction){
    InnerNode* parent = innerNodeIn->getParent();
    if(parent == nullptr){
        return nullptr;
    }
    auto i = find(parent->children.begin(),parent->children.end(),innerNodeIn);
    unsigned long distance = i - parent->children.begin();
    if(direction == 'R'){
        if(distance == parent->children.size()-1){
            return nullptr;
        }
        else{
            return static_cast<InnerNode*>(parent->children[distance+1]);
        }
    }
    else if(direction == 'L'){
        if(distance == 0){
            return nullptr;
        }
        else{
            return static_cast<InnerNode*>(parent->children[distance-1]);
        }
    }
    return nullptr;
}

Key InnerNode::findSiblingKey(InnerNode* innerNodeIn, char direction){
    auto i = getSibling(innerNodeIn,direction);
    auto i2 = static_cast<InnerNode*>(innerNodeIn->getParent()->children.front());
    auto i3 = i - i2;
    unsigned long keyDistance = i3 - 1;
    return innerNodeIn->getParent()->keys[keyDistance];
}


Key InnerNode::findRightKey(InnerNode* innerNodeIn){
    InnerNode* parent = innerNodeIn->getParent();
    assert(parent != nullptr);
    auto i = find(parent->children.begin(),parent->children.end(),innerNodeIn);
    unsigned long distance = i - parent->children.begin();
    return parent->keys[distance-1];
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
    auto i = std::find(this->children.begin(),this->children.end(),rightDescendant);
    unsigned long index = (this->children.begin()-i) - 1;
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
    while(keys.size() > i && getRemovalKey >= keys[i]){
        i++;
    }
    
    children[i] -> deleteEntry(entryToRemove);
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
    
    InnerNode* rightSibling = getSibling(this,'R');
    InnerNode* leftSibling = getSibling(this,'L');

    if(this->getParent() != nullptr){
        if(this->keys.size() == kLeafOrder){
            //first try borrowing leafNode from right sibling
            if(this->getParent() != nullptr && getSibling(this,'R') != nullptr && getSibling(this,'R')->keys.size() > kLeafOrder){
                //Borrow one child over
                this->children.push_back(getSibling(this,'R')->children[0]);
                getSibling(this,'R')->children.erase(getSibling(this,'R')->children.begin());
                
                //update key of this
                Key thisKey = findRightKey(this);
                this->updateKey(this->children[kLeafOrder-1],thisKey);
                
                //update keys of right sibling
                getSibling(this,'R')->keys.erase(getSibling(this,'R')->keys.begin());
                
                //update key of parent
                this->getParent()->updateKey(getSibling(this,'R'),getSibling(this,'R')->keys[0]);
            }
            //try borrowing leafNode from left sibling
            //problem
            else if(this->getParent() != nullptr &&  getSibling(this,'L') != nullptr && getSibling(this,'L')->keys.size() > kLeafOrder){
                //Borrow one child over
                this->children.push_back(getSibling(this,'L')->children[getSibling(this,'L')->children.size()-1]);
                getSibling(this,'L')->children.pop_back();
                
                //update key of this
                //push key to front
                this->updateKey(this->children[1],this->children[1]->minKey());
                
                //update keys of left sibling
                getSibling(this,'L')->keys.pop_back();
                
                //update key of parent
                this->getParent()->updateKey(this,this->keys[0]);
            }
            
            //try merging with right
            else if(getSibling(this,'R') != nullptr && getSibling(this,'R')->keys.size() == kLeafOrder){
                for(unsigned int i = 0; i < getSibling(this,'R')->children.size(); ++i){
                    this->children.push_back(getSibling(this,'R')->children[i]);
                }
                delete getSibling(this,'R');
                if(this->getParent()->children.size() > kLeafOrder){
                    //updateKeys
                    Key deleteKey = findSiblingKey(this,'R');
                    this->getParent()->keys.erase(remove(keys.begin(),keys.end(),deleteKey),keys.end());
                    
                }
                else{
                    delete this->getParent();
                }
            }
            //try merging with left
            else if(getSibling(this,'L') != nullptr && getSibling(this,'L')->keys.size() == kLeafOrder){
                for(unsigned int i = 0; i < children.size(); ++i){
                    if(children[i] == childToRemove){
                        children.erase(children.begin() + i);
                    }
                }
                for(unsigned int i = 0; i < this->children.size(); ++i){
                    this->children[i]->updateParent(getSibling(this,'L'));
                    getSibling(this,'L')->children.push_back(this->children[i]);
                }
                for(unsigned int i = 0; i < this->children.size(); ++i){
                    this->children.pop_back();
                }
                //update key
                this->getSibling(this,'L')->keys.push_back(this->getSibling(this,'L')->maxKey());
                if(this->getParent()->children.size() > kLeafOrder+1){
                    //updateKeys
                    Key deleteKey = findSiblingKey(this,'L');
                    this->getParent()->keys.erase(remove(keys.begin(),keys.end(),deleteKey),keys.end());
                }
                else{
                    this->getSibling(this,'L')->updateParent(this->getParent()->getParent());
                }
                this->getParent()->children.erase(remove(this->getParent()->children.begin(),this->getParent()->children.end(),this)
                                                  ,this->getParent()->children.end());
            }
            else{
                this->getParent()->children.erase(remove(this->getParent()->children.begin(),this->getParent()->children.end(),this), this->getParent()->children.end());
                this->getParent()->keys.erase(remove(this->getParent()->keys.begin(),this->getParent()->keys.end(),findRightKey(this)),
                                              this->getParent()->keys.end());
            }
            
        }
    }
    else{
        auto i = std::find(this->children.begin(),this->children.end(),childToRemove);
        unsigned long distance = std::distance(this->children.begin(), i);
        this->children.erase(i);
        this->keys.erase(this->keys.begin() + (distance-1));
    }
}
