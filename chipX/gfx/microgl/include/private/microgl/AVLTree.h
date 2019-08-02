#pragma once

#include <iostream>

namespace ds {

    template<typename Key, typename Compare>
    class AVLTree {
    public:
        struct Node {
            Key key;
            unsigned char height;
            Node *l = nullptr;
            Node *r = nullptr;
            Node *p = nullptr;

            Node(const Key &k, Node * $parent) {
                key = k;
                l = r = nullptr;
                height = 1;
                p = $parent;
            }
            Node() {
                height = 0;
            }
        };

        AVLTree() {
            root = nullptr;
        }

        Node *getRoot() {
            return root;
        }

        bool isEmpty() {
            return getRoot()== nullptr;
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

    private:
        Compare compare;
        Node * root = nullptr;

#define max(a,b) (a)>(b) ? (a) : (b)


        int getHeight(Node * n) {
            if(!n)
                return 0;

            return(1 + max(getHeight(n->l), getHeight(n->r)));
        }

        int setBalance(Node * n) {
            if(!n)
                return 0;

            int lH = getHeight(n->l);
            int rH = getHeight(n->r);

            n->b = lH - rH;
            return(n->b);
        }

        void insertNode(const Key & key) {
            if(!root) {
                printf("Adding root %d\n", key);
                root = new Node();
                root->key = key;
                root->p = nullptr;
                root->b = 0;
            } else {
                Node * t = root;
                while(t) {
                    if(isEqual(key, t->key)) {
                        printf("Node exists %d\n", key);
                    }
                    if(isPreceding(key, t->key)) {
                        if(!t->l) {
                            printf("Adding on left %d\n", key);
                            t->l = new Node();
                            t->l->key = key;
                            t->l->p = t;
                            t->l->height = 0;
                            t = t->l;
                            break;
                        } else
                            t = t->l;
                    } else {
                        if(!t->r) {
                            printf("Adding %d on right of %p, %3d\n", key, t, t->key);
                            t->r = new Node();
                            t->r->key = key;
                            t->r->p = t;
                            t->r->b = 0;
                            t = t->r;
                            break;
                        } else
                            t = t->r;
                    }

                }

                updateTreeBal(root);

                Node * p = t->p;
                while(p) {
                    int b = setBalance(p);
                    if(b >= 2 || b <= -2)  {
                        printf("Disbalanced by %d @ %p k:%3d after adding %p k:%3d\n", b, p, p->key, t, t->key);
                        rotate(p, t);
                    }
                    p = p->p;
                }
                updateTreeBal(root);
            }
        }

/**
 *
 * case 1: L->L
 * case 2: L->R
 * case 3: R->R
 * case 4: R->L
 *
 **/

        void rotate(Node * p, Node * n) {
            debug("pk:%d nk:%d\n", p->key, n->key);
            if(isPreceding(p->key, n->key)) {
                Node * c = p->r;
                if(isPreceding(c->key, n->key)) {
                    caseRR(p, n);
                } else if(!isEqual(c->key, n->key)) {
                    caseRL(p, n);
                }
            } else if (isPreceding(n->key, p->key)) {
                Node * c = p->l;
                if(isPreceding(c->key, n->key)) {
                    caseLR(p, n);
                } else if(!isEqual(c->key, n->key)) {
                    caseLL(p, n);
                }

            }

        }

        void caseLL(Node * p, Node * n) {
            std::cout<<("Case: L L\n");
            Node * pp = p->p;
            Node * c = p->l;
            Node * cr = c->r;

            if(!pp) {
                root = c;
                c->p = NULL;
            } else {
                c->p = pp;
                if(isPreceding(c->key, pp->key))
                    pp->l = c;
                else
                    pp->r = c;
            }

            c->r = p;
            p->p = c;
            p->l = cr;
        }

        void caseLR(Node * p, Node * n) {
            std::cout << ("Case: L R\n");
            Node * pp = p->p;
            Node * c = p->l;
            Node * nl = n->l;
            Node * nr = n->r;

            if(!pp) {
                root = n;
                n->p = NULL;
            } else {
                n->p = pp;
                if(isPreceding(n->key, pp->key))
                    pp->l = n;
                else
                    pp->r = n;
            }

            n->r = p;
            p->p = n;
            n->l = c;
            c->p = n;
            p->l = nl;
            c->r = nr;
        }

        void caseRL(Node * p, Node * n) {
            std::cout << ("Case: R L\n");
            Node * pp = p->p;
            Node * c = p->r;
            Node * cr = c->r;

            if(!pp) {
                root = n;
                n->p = NULL;
            } else {
                n->p = pp;
                if(isPreceding(n->key, pp->key))
                    pp->l = n;
                else
                    pp->r = n;
            }

            n->l = p;
            p->p = n;
            n->r = c;
            c->p = n;
            p->r = NULL;
            c->l = NULL;
        }

        void caseRR(Node * p, Node * n) {
            std::cout <<"Case: R R\n";
            Node * pp = p->p;        // parent of node with disbalance
            Node * c = p->r;     // child of disbalanced node
            Node * cl = c->l;        // left child of child of disbalanced node

            if(!pp) {               // p is root
                root = c;
                c->p = NULL;
            } else {
                c->p = pp;
                if(isPreceding(c->key, pp->key))
                    pp->l = c;
                else
                    pp->r = c;
            }

            c->l = p;
            p->p = c;
            p->r = cl;
        }

        void updateTreeBal(Node * n) {
            if(!n)
                return;

            updateTreeBal(n->l);
            setBalance(n);
            updateTreeBal(n->r);
        }

        void debugTree(Node * n) {
            if(!n)
                return;

            printf("[0x%08X] L:0x%08X R:0x%08X P:0x%08X K:%3d B:%2d\n", n, n->l, n->r, n->p, n->key, n->b);
            debugTree(n->l);
            debugTree(n->r);
        }
    };

}