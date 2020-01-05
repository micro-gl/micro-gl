namespace microgl {
    namespace traits {
        template<class T1, class T2>
        struct is_same {
        public:
            const static bool value = false;
        };

        template<class T>
        struct is_same<T, T> {
        public:
            const static bool value = true;
        };

    }

}