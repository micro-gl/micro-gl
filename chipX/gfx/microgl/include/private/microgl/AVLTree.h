#pragma once

namespace ds {

    template<typename Key, typename Compare>
    class AVLTree {
    public:
        struct Node {
            Key key;
            unsigned char height=0;
            Node *left = nullptr;
            Node *right = nullptr;
            Node *parent = nullptr;

            Node(const Key &k, Node * $parent) {
                key = k;
                left = right = nullptr;
                parent = $parent;
            }
            Node() = default;
        };


        Node *getRoot() {
            return root;
        }

        bool isEmpty() {
            return getRoot()== nullptr;
        }

        Node* search(Key &k) {
            return search(getRoot(), k);
        }

        Node * search(Node * root, Key &k) {
            Node *t = root;

            while (t != nullptr) {
                if (isEqual(k, t->key)) {
                    return t;
                } else if (isPreceding(k, t->key)) {
                    t = t->left;
                } else {
                    t = t->right;
                }

            }

            return nullptr;
        }

        Node* searchExact(Key &k) {
            return searchExact(getRoot(), k);
        }

        Node * searchExact(Node * root, Key &k) {
            Node *t = root;

            while (t != nullptr) {
                if (isEqual(k, t->key)) {
                    return t;
                } else if (isPreceding(k, t->key)) {
                    t = t->left;
                } else {
                    t = t->right;
                }

            }

            return nullptr;
        }

        bool contains(Key &k) {
            return contains(getRoot(), k);
        }

        bool contains(Node * root, Key &k) {
            return search(root, k)!= nullptr;
        }

        Node * findLowerBoundOf(const Key & key) {

            Node * root = getRoot();

            Node * candidate = nullptr;

            while(root!= nullptr) {

                bool has_left = root->left!= nullptr;
                bool has_right = root->right!= nullptr;

                // if root key is before me, then he might be a candidate,
                // let's store it, and try to get better
                if(!isPreceding(key, root->key)) {
                    candidate = root;

                    // let's try to get a bigger lower bound
                    if(has_right)
                        root = root->right;
                    else break;
                }
                else {
                    if(has_left)
                        root = root->left;
                    else
                        break;

                }

            }

            return candidate;
        }

        Node * findUpperBoundOf(const Key & key) {

            Node * root = getRoot();

            Node * candidate = nullptr;

            while(root!= nullptr) {

                bool has_left = root->left!= nullptr;
                bool has_right = root->right!= nullptr;

                // if root key is before me, then he might be a candidate,
                // let's store it, and try to get better
                if(isPreceding(key, root->key)) {
                    candidate = root;

                    // let's try to get a bigger lower bound
                    if(has_left)
                        root = root->left;
                    else break;
                }
                else {
                    if(has_right)
                        root = root->right;
                    else
                        break;

                }

            }

            return candidate;
        }

        Node * successor(Node * node) {

            if(node== nullptr)
                return nullptr;

            // case 1, we have a right sub-tree
            Node * successor = findMin(node->right);

            // case 2
            if(successor == nullptr) {
                Node * p = node->parent;

                while (p!= nullptr && node==p->right) {
                    node = p;
                    p = p->parent;
                }

                successor = p;
            }

            return successor;
        }

        Node * predecessor(Node * node) {

            if(node== nullptr)
                return nullptr;

            Node * predecessor = findMax(node->left);

            if(predecessor == nullptr) {
                Node * p = node->parent;

                while (p!= nullptr && node==p->left) {
                    node = p;
                    p = p->parent;
                }

                predecessor = p;
            }

            return predecessor;
        }

        Node *insert(const Key &k) {
            return insert(getRoot(), k);
        }

        Node* insert(Node* p, const Key & k)
        {
            if(p== nullptr) {
                Node * node = new Node(k, nullptr);

                if(root==nullptr)
                    root=node;

                return node;
            }

            if(isPreceding(k, p->key)) {
                p->left = insert(p->left, k);
                p->left->parent = p;
            }
            else {
                p->right = insert(p->right, k);
                p->right->parent = p;
            }

            return balance(p);
        }

        Node* findMin() {
            return findMin(getRoot());
        }

        Node* findMin(Node* p) {
            Node * t = p;
            while (t && t->left!= nullptr) {
                t->left->parent = t;
                t = t->left;
            }

            return t;
        }

        Node* findMax() {
            return findMax(getRoot());
        }

        Node* findMax(Node* p) {
            Node * t = p;
            while (t && t->right!= nullptr)
                t = t->right;

            return t;
        }

        Key removeMinKey() {

            Node * min = findMin();
            Node min2;

            Key &key = min->key;

            remove(getRoot(), key, &min2);

            return min2.key;
        }

        Node* removeHeavy(const Key &k) {
            Node * n = findMax();

            if(isEqual(k, n->key)) {

            }
        }

        Node* remove(const Key &k) {
            return remove(getRoot(), k);
        }

        void remove(Node * p) {
            Node* q = p->left;
            Node* r = p->right;
            bool p_has_parent = p->parent;
            bool p_is_left_child = p_has_parent ? p->parent->left == p : false;
            bool p_is_right_child = p_has_parent ? p->parent->right == p : false;

            bool p_is_root = p==root;

            delete p;
//                free(p);

            if(p_is_root) {
                root= nullptr;
            }

            if( !r ) {
                if(p_is_root) {
                    root = q;
                    if(root!= nullptr)
                        root->parent = nullptr;
                }

                if(p_is_left_child)
                    p->parent->left = q;
                else if(p_is_right_child)
                    p->parent->right = q;

                if(p && p->parent) {

                    balance(p->parent);
                }

                return;
            }

            Node* min = findMin(r);

            min->right = removeMin_internal(r);
            min->left = q;

            if(min->left)
                min->left->parent = min;

            if(min->right)
                min->right->parent = min;

            if(p_is_root) {
                root = min;
                // todo:: this cleaned the bug
                root->parent = nullptr;
            }

            Node * balanced_root = balance(min);

            if(p_is_left_child)
                p->parent->left = balanced_root;
            else if(p_is_right_child)
                p->parent->right = balanced_root;

            if(p && p->parent) {
                balance(p->parent);

            }
        }

        // make this iterative with a stack
        Node* remove(Node* p, const Key &k, Node * removedNode= nullptr) // deleting k key from p tree
        {
            if( !p )
                return nullptr;

            else if(isPreceding(k, p->key)) {
                p->left = remove(p->left, k, removedNode);
                if(p->left)
                    p->left->parent = p;
            }
            else if(isPreceding(p->key, k)) {
                p->right = remove(p->right, k, removedNode);
                if(p->right)
                    p->right->parent = p;
            }
            else {
                Node* q = p->left;
                Node* r = p->right;

                if(removedNode)
                    *removedNode = *p;

                bool p_is_root = p==root;

                delete p;
//                free(p);

                if(p_is_root) {
                    root= nullptr;
                }

                if( !r ) {
                    if(p_is_root) {
                        root = q;
                        if(root!= nullptr)
                            root->parent = nullptr;
                    }

                    return q;
                }

                Node* min = findMin(r);

                min->right = removeMin_internal(r);
                min->left = q;

                if(min->left)
                    min->left->parent = min;

                if(min->right)
                    min->right->parent = min;

                if(p_is_root) {
                    root = min;
                    // todo:: this cleaned the bug
                    root->parent = nullptr;
                }

                return balance(min);
            }

            // this is when quitting the recursion
            return balance(p);
        }

        bool isPreceding(const Key &lhs, const Key &rhs) {
            return compare.isPreceding(lhs, rhs);
        }

        bool isPrecedingOrEqual(const Key &lhs, const Key &rhs) {
            return compare.isPreceding(lhs, rhs) || Equal(lhs, rhs);
        }

        bool isSucceeding(const Key &lhs, const Key &rhs) {
            return !compare.isPreceding(lhs, rhs) && !isEqual(lhs, rhs);
        }

        bool isEqual(const Key &lhs, const Key &rhs) {
            return !compare.isPreceding(lhs, rhs) && !compare.isPreceding(rhs, lhs);
        }

        bool isExact(const Key &lhs, const Key &rhs) {
            return compare.isEqual(lhs, rhs);
        }

        Compare & getComparator() {
            return compare;
        }

    private:
        Compare compare;
        Node * root = nullptr;

        Node * removeMin_internal() {
            return removeMin_internal(getRoot());
        }

        Node * removeMin_internal(Node* p) {
            if( p->left== nullptr )
                return p->right;
            p->left = removeMin_internal(p->left);
            return balance(p);
        }

        unsigned char height(Node* p)
        {
            return p ? p->height : 0;
        }

        int bFactor(Node* p)
        {
            return height(p->right)-height(p->left);
        }

        void fixHeight(Node* p)
        {
            unsigned char hl = height(p->left);
            unsigned char hr = height(p->right);
            p->height = (hl>hr?hl:hr)+1;
        }

        Node* rotateRight(Node* p)
        {
            Node * temp_parent = p->parent;
            Node* q = p->left;
            p->left = q->right;
            q->right = p;

            // fix parents
            q->parent = temp_parent;
            p->parent = q;

            if(p->left)
                p->left->parent = p;

            if(p==root)
                root=q;

            //

            fixHeight(p);
            fixHeight(q);
            return q;
        }

        Node* rotateLeft(Node* q)
        {
            Node * temp_parent = q->parent;
            Node* p = q->right;
            q->right = p->left;
            p->left = q;

            // fix parents
            p->parent = temp_parent;
            q->parent = p;
            if(q->right)
                q->right->parent = q;

            if(q==root)
                root=p;
            //

            fixHeight(q);
            fixHeight(p);
            return p;
        }

        Node* balance(Node* p) // balancing the p Node
        {
            fixHeight(p);
            if(bFactor(p) == 2 )
            {
                if(bFactor(p->right) < 0 ) {
                    p->right = rotateRight(p->right);
                    p->right->parent = p;
                }

                return rotateLeft(p);
            }
            if(bFactor(p) == -2 )
            {
                if(bFactor(p->left) > 0  ) {
                    p->left = rotateLeft(p->left);

                    p->left->parent = p;
                }
                return rotateRight(p);
            }
            return p; // balancing is not required
        }


    };

}