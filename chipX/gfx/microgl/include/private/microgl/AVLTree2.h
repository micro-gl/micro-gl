#pragma once

namespace ds {

    template<typename Key, typename Compare>
    class AVLTree2 {
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
//                if($parent!= nullptr)
//                    height = 1;

                parent = $parent;
            }
            Node() {
            }
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

        bool contains(Key &k) {
            return contains(getRoot(), k);
        }

        bool contains(Node * root, Key &k) {
            return search(root, k)!= nullptr;
        }

        Node * findUpperBoundOf(const Key & key) {

            Node * root = getRoot();

            Node * leastUpperBound = root;

            while(root!= nullptr) {

                bool has_left = root->left!= nullptr;
                bool has_right = root->right!= nullptr;

                if(isEqual(root->key, key)) {
                    leastUpperBound = root;

                    // try to get the right most one in case there are more duplicates
                    if(has_right)
                        root = root->right;
                    else
                        break;
                }
                    // we are smaller than current node, therefore slide into right subtree
                else if(isPreceding(key, root->key)) {
                    if(isPreceding(root->key, leastUpperBound->key)) {
                        leastUpperBound = root;
                    }

                    if(has_left)
                        root = root->left;
                    else
                        break;
                }
                    // we are less than current node, therefore slide into left subtree
                else {
                    if(has_right)
                        root = root->right;
                    else
                        break;
                }

            }

            // final verification
            if(leastUpperBound &&
                (isPreceding(key, leastUpperBound->key) ||
                isEqual(key, leastUpperBound->key)))
                return leastUpperBound;

            return nullptr;
        }

        Node * findLowerBoundOf(const Key & key) {

            Node * root = getRoot();

            Node * maximalLowerBound = root;

            while(root!= nullptr) {

                bool has_left = root->left!= nullptr;
                bool has_right = root->right!= nullptr;

                if(isEqual(root->data, key)) {

                    maximalLowerBound = root;

                    // try to get the left most one in case there are more duplicates
                    if(has_left)
                        root = root->left;
                    else
                        break;
                }
                    // we are bigger than current node, therefore slide into right subtree
                else if(isPreceding(root->data, key)) {
                    maximalLowerBound = root;

                    if(has_right)
                        root = root->right;
                }
                    // we are less than current node, therefore slide into left subtree
                else {
                    if(has_left)
                        root = root->left;
                }

            }

            // final verification
            if(maximalLowerBound &&
                (isPreceding(maximalLowerBound->key, key) ||
                    isEqual(maximalLowerBound->key, key)))
                return maximalLowerBound;

            return nullptr;
        }

        Node * successor(Node * node) {

            if(node== nullptr)
                return nullptr;

            Node * successor = findMin(node->right);

            if(successor == nullptr) {
//                while (node->parent!= nullptr) {
                    if(node->parent && node->parent->left == node) {
                        successor = node->parent;
//                        break;
                    }

//                    node = node->parent;
//                }

            }

            return successor;
        }

        Node * predecessor(Node * node) {

            if(node== nullptr)
                return nullptr;

            Node * predecessor = findMax(node->left);

            if(predecessor == nullptr) {
//                while (node->parent!= nullptr) {
                    if(node->parent && node->parent->right == node) {
                        predecessor = node->parent;
//                        break;
//                    }

//                    node = node->parent;
                }

            }

            return predecessor;
        }

        Node *insert(const Key &k) { // insert k key in a tree with p root
//            if(root==nullptr)
//                root = new Node();

            return insert(getRoot(), k);
        }

        Node* insert(Node* p, const Key & k) // insert k key in a tree with p root
        {
            if(p== nullptr) {
//                return new Node(k, p);
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

        Key & removeMinKey() {
            Node * min = findMin();

            Key &key = min->key;

            remove(getRoot(), key);

            return key;
        }

        Node* remove(const Key &k) {
            return remove(getRoot(), k);
        }

        Node* remove(Node * node) {
            if( !node )
                return nullptr;

            Node* q = node->left;
            Node* r = node->right;

            delete node;

            if( !r )
                return q;

            Node* min = findMin(r);

            min->right = removeMin_internal(r);
            min->left = q;

            if(min->left)
                min->left->parent = min;
            if(min->right)
                min->right->parent = min;

            return balance(min);
        }

        // make this iterative with a stack
        Node* remove(Node* p, const Key &k) // deleting k key from p tree
        {
            if( !p )
                return nullptr;

            if (isEqual(k, p->key)) {
                Node* q = p->left;
                Node* r = p->right;

                bool p_is_root = p==root;

                delete p;
//                free(p);

                if(p_is_root) {
                    p=root= nullptr;
                }

                if( !r )
                    return q;

                Node* min = findMin(r);

                min->right = removeMin_internal(r);
                min->left = q;

                if(min->left)
                    min->left->parent = min;

                if(min->right)
                    min->right->parent = min;

                if(p_is_root)
                    root = min;

                return balance(min);
            }
            else if(isPreceding(k, p->key)) {
                p->left = remove(p->left, k);
                if(p->left)
                    p->left->parent = p;
            }
            else {
                p->right = remove(p->right, k);
                if(p->right)
                    p->right->parent = p;
            }



            // this is when quitting the recursion
            return balance(p);
        }

        bool isPreceding(const Key &lhs, const Key &rhs) {
            return compare.isPreceding(lhs, rhs);
        }

        bool isSucceeding(const Key &lhs, const Key &rhs) {
            return !compare.isPreceding(lhs, rhs) && !isEqual(lhs, rhs);
        }

        bool isEqual(const Key &lhs, const Key &rhs) {
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