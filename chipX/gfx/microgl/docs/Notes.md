#### some notes

- avoid branching in loops, it kills performance
- avoid non-inlined function calls inside loops, they kill performance
- virtual methods cannot be inlined - https://www.geeksforgeeks.org/inline-virtual-function/
- https://stackoverflow.com/questions/26115269/polymorphic-behaviour-without-virtual

- https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
- http://www.flipcode.com/archives/Faking_Polymorphism_with_C_Templates.shtml
- https://www.fluentcpp.com/2017/05/16/what-the-crtp-brings-to-code/
- https://eli.thegreenplace.net/2013/12/05/the-cost-of-dynamic-virtual-calls-vs-static-crtp-dispatch-in-c
- https://stackoverflow.com/questions/43569868/crtp-pattern-yet-store-inhomogenous-types-in-a-datastructure
- https://katyscode.wordpress.com/2013/08/22/c-polymorphic-cloning-and-the-crtp-curiously-recurring-template-pattern/
- https://eli.thegreenplace.net/2011/05/17/the-curiously-recurring-template-pattern-in-c


template<typename Derived>
class Base {

    static inline void hello() {
        Derived::hello();
    }
}

class ExampleDerived : Base<ExampleDerived> {

    static inline void hello() {
        cout << "hello";
    }
}

https://www.desmos.com/calculator/afoxmvnpk0

#### structure alignment
- every type's relative memory address should be a multiple of the number of it's bytes, that's what alignment means !!!
- https://software.intel.com/en-us/articles/coding-for-performance-data-alignment-and-structures


#### performance
- storing and accessing 32bit is faster than 8 bit values