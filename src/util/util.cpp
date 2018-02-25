#include <iostream>

template <class T>
void dump_bytes(T const *t, std::ostream &out) {
    unsigned char const *p = reinterpret_cast<unsigned char const *>(t);
    for (size_t n = 0 ; n < sizeof(t) ; ++n)
        out << std::hex << (int) p[n] << " ";
    out << "\n";
}
