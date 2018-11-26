

namespace Math {
    // ------------------------------------------------------------------------
    template <class T>
    T clamp(T a, T b, T t)
    {
        if (t < a) return a;
        if (t > b) return b;
        return t;
    }

} // namespace Math