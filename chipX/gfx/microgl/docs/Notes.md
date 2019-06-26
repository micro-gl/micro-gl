#### some notes

- avoid branching in loops, it kills performance
- avoid non-inlined function calls inside loops, they kill performance
- virtual methods cannot be inlined - https://www.geeksforgeeks.org/inline-virtual-function/

template<typename Derived>
class Base<Derived> {

    static inline void hello() {
        Derived::hello();
    }
}

class ExampleDerived : Base<ExampleDerived> {

    static inline void hello() {
        cout << "hello";
    }
}

