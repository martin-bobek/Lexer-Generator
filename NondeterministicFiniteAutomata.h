#ifndef NONDETERMINISTIC_FINITE_AUTOMATA_H__
#define NONDETERMINISTIC_FINITE_AUTOMATA_H__

#include <memory>
#include <vector>

using std::vector;

constexpr size_t EPSILON = 0;

class NFA
{
public:
    NFA() = default;
    NFA(char c);
    NFA(const NFA &) = delete;
    NFA(NFA &&mov) = default;
    NFA &operator=(const NFA &) = delete;
    NFA &operator=(NFA &&rhs) = default;
    vector<bool> &Closure(vector<bool> &subset) const;
    vector<bool> Move(const vector<bool> &subset, size_t cIndex) const;
    static NFA Complete(NFA &&arg, size_t acceptingType);
    static NFA Concatenate(NFA &&lhs, NFA &&rhs);
    static NFA Or(NFA &&lhs, NFA &&rhs);
    static NFA Star(NFA &&arg);
    static NFA Plus(NFA &&arg);
    static NFA Merge(vector<NFA> &&nfas);

    size_t Size() const;
    size_t Accepting(const vector<bool> &subset) const;

    static char Alphabet(size_t index) { return alphabet[index]; }
    static size_t AlphabetSize() { return alphabet.size(); }
private:
    class State;
    typedef std::unique_ptr<State> pState;
    void closureRecursion(size_t current, size_t checked, vector<bool> &subset) const;
    size_t exitCIndex;							// all references to exit_char need to be readjusted
    State *exitState;
    vector<pState> states;

    static size_t charIndex(char c);
    static vector<char> alphabet;
};

class NFA::State
{
public:
    State(size_t accepting = 0) : accepting(accepting) {}
    void attach(size_t cIndex, State *to);
    void assignNum(size_t num);
    vector<size_t> transList(size_t cIndex) const;
    size_t AcceptingType() const;
private:
    size_t accepting;
    size_t stateNum;
    struct Transition
    {
        const size_t cIndex;
        State *const state;
    };
    vector<Transition> transitions;
};

#endif
