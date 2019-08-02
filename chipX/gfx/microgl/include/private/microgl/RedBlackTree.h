#pragma once

#include <stdexcept>

namespace ds {
    enum class node_color_t {
        BLACK, RED, NIL
    };

    template<typename Type>
    struct Node {
        Node* parent;
        Node* left;
        Node* right;
        enum node_color_t color;
        Type data;
    };

    template<typename Type, typename COMPARE>
    class RedBlackTree {

    public:
        COMPARE compare;

        typedef Node<Type> Node;

        Node * newNode(Type data) {
            Node * node = new Node();
            node->data = Type{data};

            return node;
        }

//        Node * newNode(Type data) {
//            Node * node = new Node();
//            node->data = data;
//        }

        void assert(bool ex) {
            if(!ex)
                throw std::invalid_argument( "received negative value" );
        }

        Node * getRoot() {
            return root;
        }

        bool isEmpty() {
            return getRoot()== nullptr;
        }

        bool contains(Type &data) {
            return search(data)!= nullptr;
        }

        Node* insertIfNotContains(Node* n) {
            if(contains(n->data))
                return nullptr;

            insert(n);
        }

        Node* insert(Node* n) {
            // Insert new Node into the current tree.
            insertRecurse(root, n);

            // Repair the tree in case any of the red-black properties have been violated.
            insertRepairTree(n);

            // Find the new root to return.
            root = n;
            while (getParent(root) != nullptr) {
                root = getParent(root);
            }
            return root;
        }

        Node * search(Type &data) {
            return search_iterative(root, data);
        }

        Node * search_iterative(Node * $root, Type &data) {
            Node *p = $root;

            while (p!= nullptr) {
                if(isEqual(data, p->data))
                    return p;
                else if(isPreceding(data, p->data))
                    p=p->left;
                else
                    p=p->right;
            }

            return nullptr;
        }

        Node * search_recurse(Node * $root, Type & data) {
            Node *p = $root;

            if(isEqual(p->data, data))
                return p;
            else if (isPreceding(data, p->data)) {
                Node *left = search_recurse(p->left, data);
                return left;
            } else {
                Node * right = search_recurse(p->right, data);
                return right;
            }

        }

        bool isLeaf(Node * node) {
            return node->left==nullptr && node->right==nullptr;
        }

        Node* insert(Type data) {
            return insert(newNode(data));
        }

//        Node* insert(Type data) {
//            return insert(newNode(data));
//        }

        void deleteChild(Node* n) {
            // Precondition: n has at most one non-leaf child.
            Node* child = (n->right == nullptr) ? n->left : n->right;
//            assert(child!= nullptr);

            replaceNode(n, child);
            if (n->color == node_color_t::BLACK) {
                if (child->color == node_color_t::RED) {
                    child->color = node_color_t::BLACK;
                } else {
                    deleteCase1(child);
                }
            }

            delete n;
//            free(n);
        }

        Type &removeMinElement() {
            Node * min = findMinElementInSubTree(getRoot());
            Type &data = min->data;
            deleteChild(min);

            return data;
        }

        Node * findMinElementInSubTree(Node * $root) {
            Node *t = $root;

            while (t->left != nullptr) {
                t = t->left;

            }

            return t;
        }

        Node * findMaxElementInSubTree(Node * $root) {
            Node *t = $root;

            while (t->right != nullptr) {
                t = t->right;
            }

            return t;
        }

        /*
def getNextNode (node):
    if node.right != NULL:
        node = node.right
        while node.left != NULL:
            node = node.left
        return node

    while node.parent != NULL:
        if node.parent.left == node:
            return node.parent
        node = node.parent

    return NULL
         */

        Node * findUpperBoundOf(Type & data) {

            Node * root = getRoot();

            while(root!= nullptr) {

                bool has_left = root->left!= nullptr;
                bool has_right = root->right!= nullptr;

                if(isEqual(root->data, data))
                    return root;
                    // we are bigger than current node, therefore slide into right subtree
                else if(isSucceeding(root->data, data)) {
                    if(has_right)
                        root = root->right;
                    else
                        return root;
                }
                    // we are less than current node, therefore slide into left subtree
                else {
                    if(has_left)
                        root = root->left;
                    else
                        return root;

                }

            }

        }

        Node * findLowerBoundOf(Type & data) {

            Node * root = getRoot();

            while(root!= nullptr) {

                bool has_left = root->left!= nullptr;
                bool has_right = root->right!= nullptr;

                if(isEqual(root->data, data))
                    return root;
                    // we are bigger than current node, therefore slide into right subtree
                else if(isPreceding(root->data, data)) {
                    if(has_left)
                        root = root->left;
                    else
                        return root;
                }
                    // we are less than current node, therefore slide into left subtree
                else {
                    if(has_right)
                        root = root->right;
                    else
                        return root;

                }

            }

        }

        Node * successor(Node * node) {

            if(node== nullptr)
                return nullptr;

            Node * successor = findMinElementInSubTree(node->right);

            if(successor == nullptr) {
                while (node->parent!= nullptr) {
                    if(node->parent->left == node) {
                        successor = node->parent;
                        break;
                    }

                    node = node->parent;
                }

            }

            return successor;
        }

        Node * predecessor(Node * node) {

            if(node== nullptr)
                return nullptr;

            Node * predecessor = findMaxElementInSubTree(node->left);

            if(predecessor == nullptr) {
                while (node->parent!= nullptr) {
                    if(node->parent->right == node) {
                        predecessor = node->parent;
                        break;
                    }

                    node = node->parent;
                }

            }

            return predecessor;
        }

        void successorPredecessor(Node *$root, Type &data,
                                  Node *predecessor, Node *successor) {

            if ($root != nullptr) {
                if (isEqual($root->data, data)) {

                    predecessor = findMaxElementInSubTree($root);
                    successor = findMinElementInSubTree($root);

                } else if (isPreceding($root->data, data)) {
                    // we make the root as predecessor because we might have a
                    // situation when value matches with the root, it wont have
                    // right subtree to find the predecessor, in that case we need
                    // parent to be the predecessor.
                    predecessor = $root;
                    successorPredecessor($root->right, data, predecessor, successor);
                } else {
                    // we make the root as successor because we might have a
                    // situation when value matches with the root, it wont have
                    // right subtree to find the successor, in that case we need
                    // parent to be the successor
                    successor = $root;
                    successorPredecessor($root->left, data, predecessor, successor);
                }

            }

        }

        bool isPreceding(const Type &lhs, const Type &rhs) {
            return compare.isPreceding(lhs, rhs);
        }

        bool isSucceeding(const Type &lhs, const Type &rhs) {
            return !compare.isPreceding(lhs, rhs);
        }

        bool isEqual(const Type &lhs, const Type &rhs) {
            return compare.isEqual(lhs, rhs);
        }

    private:
        Node* root = nullptr;

        Node* getParent(Node* n) {
            // Note that parent is set to null for the root node.
            return n->parent;
        }

        Node* getGrandParent(Node* n) {
            Node* p = getParent(n);

            // No parent means no grandparent.
            if (p == nullptr) {
                return nullptr;
            }

            // Note: Will be nullptr if parent is root.
            return getParent(p);
        }

        Node* getSibling(Node* n) {
            Node* p = getParent(n);

            // No parent means no sibling.
            if (p == nullptr) {
                return nullptr;
            }

            if (n == p->left) {
                return p->right;
            } else {
                return p->left;
            }
        }

        Node* getUncle(Node* n) {
            Node* p = getParent(n);
            Node* g = getGrandParent(n);

            // No grandparent means no uncle
            if (g == nullptr) {
                return nullptr;
            }

            return getSibling(p);
        }

        // insertions support

        void replaceNode(Node* n, Node* child) {
            child->parent = n->parent;
            if (n == n->parent->left) {
                n->parent->left = child;
            } else {
                n->parent->right = child;
            }
        }

        void insertRecurse(Node * $root, Node* n) {
            // Recursively descend the tree until a leaf is found.
            if ($root != nullptr && isPreceding(n->data, $root->data)) {
                if ($root->left != nullptr) {
                    insertRecurse($root->left, n);
                    return;
                } else {
                    $root->left = n;
                }
            } else if ($root != nullptr) {
                if ($root->right != nullptr) {
                    insertRecurse($root->right, n);
                    return;
                } else {
                    $root->right = n;
                }
            }

            // Insert new Node n.
            n->parent = $root;
            n->left = nullptr;
            n->right = nullptr;
            n->color = node_color_t::RED;
        }

        void insertRepairTree(Node* n) {
            if (getParent(n) == nullptr) {
                insertCase1(n);
            } else if (getParent(n)->color == node_color_t::BLACK) {
                insertCase2(n);
            } else if (getUncle(n) != nullptr && getUncle(n)->color == node_color_t::RED) {
                insertCase3(n);
            } else {
                insertCase4(n);
            }
        }

        void insertCase1(Node* n) {
            if (getParent(n) == nullptr) {
                n->color = node_color_t::BLACK;
            }
        }

        void insertCase2(Node* n) {
            // Do nothing since tree is still valid.
            return;
        }
        void insertCase3(Node* n) {
            getParent(n)->color = node_color_t::BLACK;
            getUncle(n)->color = node_color_t::BLACK;
            getGrandParent(n)->color = node_color_t::RED;
            insertRepairTree(getGrandParent(n));
        }
        void insertCase4(Node* n) {
            Node* p = getParent(n);
            Node* g = getGrandParent(n);

            if (n == p->right && p == g->left) {
                rotateLeft(p);
                n = n->left;
            } else if (n == p->left && p == g->right) {
                rotateRight(p);
                n = n->right;
            }

            insertCase4Step2(n);
        }

        void insertCase4Step2(Node* n) {
            Node* p = getParent(n);
            Node* g = getGrandParent(n);

            if (n == p->left) {
                rotateRight(g);
            } else {
                rotateLeft(g);
            }
            p->color = node_color_t::BLACK;
            g->color = node_color_t::RED;
        }


        // deletions support

        void deleteCase1(Node* n) {
            if (n->parent != nullptr) {
                deleteCase2(n);
            }
        }

        void deleteCase2(Node* n) {
            Node* s = getSibling(n);

            if (s->color == node_color_t::RED) {
                n->parent->color = node_color_t::RED;
                s->color = node_color_t::BLACK;
                if (n == n->parent->left) {
                    rotateLeft(n->parent);
                } else {
                    rotateRight(n->parent);
                }
            }
            deleteCase3(n);
        }

        void deleteCase3(Node* n) {
            Node* s = getSibling(n);

            if ((n->parent->color == node_color_t::BLACK) && (s->color == node_color_t::BLACK) &&
                (s->left->color == node_color_t::BLACK) && (s->right->color == node_color_t::BLACK)) {
                s->color = node_color_t::RED;
                deleteCase1(n->parent);
            } else {
                deleteCase4(n);
            }
        }

        void deleteCase4(Node* n) {
            Node* s = getSibling(n);

            if ((n->parent->color == node_color_t::RED) && (s->color == node_color_t::BLACK) &&
                (s->left->color == node_color_t::BLACK) && (s->right->color == node_color_t::BLACK)) {
                s->color = node_color_t::RED;
                n->parent->color = node_color_t::BLACK;
            } else {
                deleteCase5(n);
            }
        }

        void deleteCase5(Node* n) {
            Node* s = getSibling(n);

            // This if statement is trivial, due to case 2 (even though case 2 changed
            // the sibling to a sibling's child, the sibling's child can't be red, since
            // no red parent can have a red child).
            if (s->color == node_color_t::BLACK) {
                // The following statements just force the red to be on the left of the
                // left of the parent, or right of the right, so case six will rotate
                // correctly.
                if ((n == n->parent->left) && (s->right->color ==node_color_t:: BLACK) &&
                    (s->left->color == node_color_t::RED)) {
                    // This last test is trivial too due to cases 2-4.
                    s->color = node_color_t::RED;
                    s->left->color = node_color_t::BLACK;
                    rotateRight(s);
                } else if ((n == n->parent->right) && (s->left->color == node_color_t::BLACK) &&
                           (s->right->color == node_color_t::RED)) {
                    // This last test is trivial too due to cases 2-4.
                    s->color = node_color_t::RED;
                    s->right->color = node_color_t::BLACK;
                    rotateLeft(s);
                }
            }
            deleteCase6(n);
        }

        void deleteCase6(Node* n) {
            Node* s = getSibling(n);

            s->color = n->parent->color;
            n->parent->color = node_color_t::BLACK;

            if (n == n->parent->left) {
                s->right->color = node_color_t::BLACK;
                rotateLeft(n->parent);
            } else {
                s->left->color = node_color_t::BLACK;
                rotateRight(n->parent);
            }
        }


        // rotations

        void rotateLeft(Node* n) {
            Node* nnew = n->right;
            Node* p = getParent(n);
            assert(nnew != nullptr);  // Since the leaves of a red-black tree are empty,
            // they cannot become internal nodes.
            n->right = nnew->left;
            nnew->left = n;
            n->parent = nnew;
            // Handle other child/parent pointers.
            if (n->right != nullptr) {
                n->right->parent = n;
            }

            // Initially n could be the root.
            if (p != nullptr) {
                if (n == p->left) {
                    p->left = nnew;
                } else if (n == p->right) {
                    p->right = nnew;
                }
            }
            nnew->parent = p;
        }

        void rotateRight(Node* n) {
            Node* nnew = n->left;
            Node* p = getParent(n);
            assert(nnew != nullptr);  // Since the leaves of a red-black tree are empty,
            // they cannot become internal nodes.

            n->left = nnew->right;
            nnew->right = n;
            n->parent = nnew;

            // Handle other child/parent pointers.
            if (n->left != nullptr) {
                n->left->parent = n;
            }

            // Initially n could be the root.
            if (p != nullptr) {
                if (n == p->left) {
                    p->left = nnew;
                } else if (n == p->right) {
                    p->right = nnew;
                }
            }
            nnew->parent = p;
        }

    };

}