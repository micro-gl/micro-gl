#pragma once

namespace ds {
    enum class node_color_t {
        BLACK, RED, NIL
    };

    template<typename Type>
    class RedBlackTree {

    public:
        struct Node {
            Node* parent;
            Node* left;
            Node* right;
            enum node_color_t color;
            int key;
            Type data;
        };

        Node * newNode(int key, Type &data) {
            Node * node = new Node();

            node->key = key;
            node->data = data;
        }

        Node * getRoot() {
            return root;
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

        Node * search(int key) {
            return search_iterative(root, key);
        }

        Node * search_iterative(Node * $root, int key) {
            Node *p = $root;

            while (p!= nullptr) {
                if(key==p->key)
                    return p;
                else if(key<p->key)
                    p=p->left;
                else
                    p=p->right;
            }

            return nullptr;
        }

        Node * search_recurse(Node * $root, int key) {
            Node *p = $root;

            if(p->key==key)
                return p;
            else if (key < p->key) {
                Node *left = search_recurse(p->left, key);
                return left;
            } else {
                Node * right = search_recurse(p->right, key);
                return right;
            }

        }

        Node* insert(int key, Type & data) {
            return insert(newNode(key, data));
        }

        void deleteChild(Node* n) {
            // Precondition: n has at most one non-leaf child.
            Node* child = (n->right == nullptr) ? n->left : n->right;
            assert(child);

            replaceNode(n, child);
            if (n->color == node_color_t::BLACK) {
                if (child->color == node_color_t::RED) {
                    child->color = node_color_t::BLACK;
                } else {
                    deleteCase1(child);
                }
            }
            free(n);
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

        void insertRecurse(Node* n) {
            // Recursively descend the tree until a leaf is found.
            if (root != nullptr && n->key < root->key) {
                if (root->left != nullptr) {
                    insertRecurse(root->left, n);
                    return;
                } else {
                    root->left = n;
                }
            } else if (root != nullptr) {
                if (root->right != nullptr) {
                    insertRecurse(root->right, n);
                    return;
                } else {
                    root->right = n;
                }
            }

            // Insert new Node n.
            n->parent = root;
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