#ifndef PAIR_H
#define PAIR_H

template <typename A, typename B>
struct Pair {
    A first;
    B second;

    Pair() : first(A()), second(B()) {}

    Pair(const A& a, const B& b) : first(a), second(b) {}

    bool operator==(const Pair& otro) const {
        return first == otro.first && second == otro.second;
    }

    bool operator!=(const Pair& otro) const {
        return !(*this == otro);
    }
};

template <typename A, typename B>
Pair<A, B> makePair(const A& a, const B& b) {
    return Pair<A, B>(a, b);
}

#endif // PAIR_H