#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"
#include <iostream>
#include <chrono>
#include <microgl/tesselation/experiments/bentley_ottmann/AVLTree.h>

using namespace ds;

void avl() {
    struct Key {
        int a;
    };

    struct order {
        bool isPreceding(const Key *k1, const Key *k2) {
            return k1->a < k2->a;
        }
    };

    AVLTree<Key *, order> tree;

    Key a{1}, b{2}, c{3}, d{4};

    tree.insert(&a);
    tree.insert(&b);
    tree.insert(&c);
    tree.insert(&d);

    auto *min = tree.removeMinKey();
    min = tree.removeMinKey();
    min = tree.removeMinKey();
    min = tree.removeMinKey();

    int v=0;
}

int main() {
    avl();
}

#pragma clang diagnostic pop