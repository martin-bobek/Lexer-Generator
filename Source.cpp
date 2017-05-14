#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>

using std::move;

#define EPSILON		0

class Node;
typedef std::unique_ptr<Node> pNode;
// produces ugly code for input (cba)*(a|b)

// char index 0 is reserved for epsilon transition

class NFA
{
public:
	NFA() : exitState(nullptr) {};
	NFA(char c);
	NFA(const NFA &) = delete;
	NFA(NFA &&mov) = default;
	NFA &operator=(const NFA &) = delete;
	NFA &operator=(NFA &&rhs) = default;
	std::vector<bool> &Closure(std::vector<bool> &subset) const;
	std::vector<bool> Move(const std::vector<bool> &subset, size_t cIndex) const;
	static NFA Complete(NFA &&arg);
	static NFA Concatenate(NFA &&lhs, NFA &&rhs);
	static NFA Or(NFA &&lhs, NFA &&rhs);
	static NFA Star(NFA &&arg);
	static NFA Plus(NFA &&arg);
	size_t Size() const;
	size_t Accepting() const;

	static char Alphabet(size_t index) { return alphabet[index]; }
	static size_t AlphabetSize() { return alphabet.size(); }
private:
	class State;
	typedef std::unique_ptr<State> pState;
	void closureRecursion(size_t current, size_t checked, std::vector<bool> &subset) const;
	size_t exitCIndex;							// all references to exit_char need to be readjusted
	State *exitState;
	std::vector<pState> states;

	static size_t charIndex(char c);
	static std::vector<char> alphabet;
};
std::vector<char> NFA::alphabet(1, '\0');
class NFA::State
{
public:
	State(bool accepting = false) : accepting(accepting) {}
	void attach(size_t cIndex, State *to);
	void assignNum(size_t num);
	std::vector<size_t> transList(size_t cIndex) const;
	bool isAccepting() const;
private:
	bool accepting;
	size_t stateNum;
	struct Transition
	{
		const size_t cIndex;
		State *const state;
	};
	std::vector<Transition> transitions;
};

class DFA
{
public:
	DFA(const NFA &nfa);
	DFA(const DFA &dfa);

	void PrintStates() const;
	void PrintHeaders(std::ostream &out) const;
	void PrintDefinitions(std::ostream &out) const;
private:
	static bool isNonempty(const std::vector<bool> &subset);

	struct StateInfo
	{
		bool accepting = false;
		std::vector<size_t> transitions = std::vector<size_t>(NFA::AlphabetSize(), EPSILON);
	};
	std::vector<StateInfo> stateInfo;
};

class Node
{
public:
	virtual NFA GenNfa(NFA &&nfa = NFA()) const = 0;
};
class Tree
{
public:
	Tree(const std::string &input);
	NFA GenNfa() const { return NFA::Complete(node->GenNfa()); }
private:
	pNode node;
};
class Terminal : public Node
{
public:
	Terminal(char symbol) : symbol(symbol) {}
	NFA GenNfa(NFA &&nfa = NFA()) const { return NFA(symbol); }
private:
	const char symbol;
};
class NonTerminal : public Node
{
protected:
	std::vector<pNode> nodes;
};
class Q : public NonTerminal
{
public:
	Q(std::string::const_iterator &it, std::string::const_iterator end);
	NFA GenNfa(NFA &&nfa = NFA()) const;
};
class R : public NonTerminal
{
public:
	R(std::string::const_iterator &it, std::string::const_iterator end);
	NFA GenNfa(NFA &&nfa = NFA()) const;
};
class S : public NonTerminal
{
public:
	S(std::string::const_iterator &it, std::string::const_iterator end);
	NFA GenNfa(NFA &&nfa = NFA()) const;
};
class T : public NonTerminal
{
public:
	T(std::string::const_iterator &it, std::string::const_iterator end);
	NFA GenNfa(NFA &&nfa = NFA()) const;
};
class U : public NonTerminal
{
public:
	U(std::string::const_iterator &it, std::string::const_iterator end);
	NFA GenNfa(NFA &&nfa = NFA()) const;
};
class V : public NonTerminal
{
public:
	V(std::string::const_iterator &it, std::string::const_iterator end);
	NFA GenNfa(NFA &&nfa = NFA()) const;
};
class W : public NonTerminal
{
public:
	W(std::string::const_iterator &it, std::string::const_iterator end);
	NFA GenNfa(NFA &&nfa = NFA()) const;
};

int main(int argc, char *argv[])
{
	std::chrono::time_point<std::chrono::high_resolution_clock> t0;
	try 
	{
		std::string expression;
		std::cout << "Regular Expression: ";
		std::cin >> expression;
		t0 = std::chrono::high_resolution_clock::now();
		Tree syntaxTree(expression);
		NFA nfa = syntaxTree.GenNfa();
		DFA dfa(nfa);
		DFA optimal(dfa);
		dfa.PrintStates();
		std::cout << std::endl;
		optimal.PrintStates();

		std::ofstream out(argc < 2 ? "out.cpp" : argv[1]);

		optimal.PrintHeaders(out);
		out << std::endl;
		optimal.PrintDefinitions(out);
		out.close();
	}
	catch (char *msg)
	{
		std::cout << msg << std::endl;
	}
	std::chrono::time_point<std::chrono::high_resolution_clock> t = std::chrono::high_resolution_clock::now();
	std::cout << "\nExecution time: " << std::chrono::duration_cast<std::chrono::microseconds>(t - t0).count() << std::endl;
	system("pause");
}

// code to expand alphabet should be added here
NFA::NFA(char c) : exitCIndex(charIndex(c))
{
	states.emplace_back(new State);
	exitState = states.back().get();
}
NFA NFA::Complete(NFA &&arg)
{
	arg.states.emplace_back(new State(true));
	arg.exitState->attach(arg.exitCIndex, arg.states.back().get());
	for (size_t i = 0; i < arg.states.size(); i++)
		arg.states[i]->assignNum(i);
	return move(arg);
}
std::vector<bool> &NFA::Closure(std::vector<bool> &subset) const
{
	for (size_t i = 0; i < subset.size(); i++)
		if (subset[i])
			closureRecursion(i, i, subset);
	return subset;
}
std::vector<bool> NFA::Move(const std::vector<bool> &subset, size_t cIndex) const
{
	std::vector<bool> result(states.size(), false);
	for (size_t i = 0; i < subset.size(); i++)
	{
		if (subset[i])
		{
			for (auto tran : states[i]->transList(cIndex))
				result[tran] = true;
		}
	}
	return Closure(result);
}
void NFA::closureRecursion(size_t current, size_t checked, std::vector<bool> &subset) const
{
	if (current > checked)
		subset[current] = true;
	else if (!subset[current] || checked == current) // note that this fails if there is an epsilon loop
	{
		subset[current] = true;
		for (auto tran : states[current]->transList(EPSILON))
			closureRecursion(tran, checked, subset);
	}
}
NFA NFA::Concatenate(NFA &&lhs, NFA &&rhs)
{
	if (!rhs.exitState)
		return move(lhs);
	if (!lhs.exitState)
		return move(rhs);
	NFA result;
	result.states.reserve(lhs.states.size() + rhs.states.size() + 1);
	lhs.exitState->attach(lhs.exitCIndex, rhs.states[0].get());
	for (auto &state : lhs.states)
		result.states.push_back(move(state));
	for (auto &state : rhs.states)
		result.states.push_back(move(state));
	result.exitCIndex = rhs.exitCIndex;
	result.exitState = rhs.exitState;
	return result;
}
NFA NFA::Or(NFA &&lhs, NFA &&rhs)
{
	if (!rhs.exitState)
		return move(lhs);
	if (!lhs.exitState)
		return move(rhs);
	NFA result;
	result.states.reserve(lhs.states.size() + rhs.states.size() + 3);
	pState in(new State), out(new State);
	in->attach(EPSILON, lhs.states[0].get());
	in->attach(EPSILON, rhs.states[0].get());
	lhs.exitState->attach(lhs.exitCIndex, out.get());
	rhs.exitState->attach(rhs.exitCIndex, out.get());
	result.states.push_back(move(in));
	for (auto &state : lhs.states)
		result.states.push_back(move(state));
	for (auto &state : rhs.states)
		result.states.push_back(move(state));
	result.exitCIndex = EPSILON;
	result.exitState = out.get();
	result.states.push_back(move(out));
	return result;
}
NFA NFA::Star(NFA &&arg)
{
	if (!arg.exitState)
		return NFA();
	NFA result;
	result.states.reserve(arg.states.size() + 2);
	pState hub(new State);
	hub->attach(EPSILON, arg.states[0].get());
	arg.exitState->attach(arg.exitCIndex, hub.get());
	result.exitCIndex = EPSILON;
	result.exitState = hub.get();
	result.states.push_back(move(hub));
	for (auto &state : arg.states)
		result.states.push_back(move(state));
	return result;
}
NFA NFA::Plus(NFA &&arg)
{
	if (!arg.exitState)
		return NFA();
	NFA result;
	result.states.reserve(arg.states.size() + 3);
	pState in(new State), out(new State);
	in->attach(EPSILON, arg.states[0].get());
	arg.exitState->attach(arg.exitCIndex, out.get());
	out->attach(EPSILON, in.get());
	result.states.push_back(move(in));
	for (auto &state : arg.states)
		result.states.push_back(move(state));
	result.exitCIndex = EPSILON;
	result.exitState = out.get();
	result.states.push_back(move(out));
	return result;
}
size_t NFA::Size() const
{
	return states.size();
}
size_t NFA::Accepting() const
{
	for (size_t i = 0; i < states.size(); i++)
		if (states[i]->isAccepting())
			return i;
	throw "NFA::Accepting: No accepting state found!";
}
void NFA::State::attach(size_t cIndex, NFA::State *to)				// all references need to be adjusted to give cIndex instead of c
{
	transitions.push_back({ cIndex, to });
}
void NFA::State::assignNum(size_t num)
{
	stateNum = num;
}
std::vector<size_t> NFA::State::transList(size_t cIndex) const		// all references need to be adjusted to give cIndex instead of c
{
	std::vector<size_t> result;
	result.reserve(transitions.size());
	for (auto trans : transitions)
		if (trans.cIndex == cIndex)
			result.push_back(trans.state->stateNum);
	return result;
}
bool NFA::State::isAccepting() const
{
	return accepting;
}
size_t NFA::charIndex(char c)
{
	std::vector<char>::const_iterator it = std::find(alphabet.begin(), alphabet.end(), c);
	size_t index = it - alphabet.begin();
	if (it == alphabet.end())
		alphabet.push_back(c);
	return index;
}

DFA::DFA(const NFA &nfa)
{
	std::vector<std::vector<bool>> states;
	std::vector<bool> stateSet(nfa.Size(), false);
	stateSet[0] = true;
	states.push_back(nfa.Closure(stateSet));
	stateInfo.push_back(StateInfo());
	if (stateSet[nfa.Accepting()])
		stateInfo[0].accepting = true;
	for (size_t stateIndex = 0; stateIndex < states.size(); stateIndex++)
	{
		for (size_t charIndex = 1; charIndex < NFA::AlphabetSize(); charIndex++)
		{
			stateSet = nfa.Move(states[stateIndex], charIndex);
			if (isNonempty(stateSet))
			{
				for (size_t prevStateIndex = 0;; prevStateIndex++)
				{
					if (prevStateIndex == states.size())
					{
						states.push_back(stateSet);
						stateInfo.push_back(StateInfo());
						if (stateSet[nfa.Accepting()])
							stateInfo[prevStateIndex].accepting = true;
						stateInfo[stateIndex].transitions[charIndex] = prevStateIndex + 1;
						break;
					}
					if (stateSet == states[prevStateIndex])
					{
						stateInfo[stateIndex].transitions[charIndex] = prevStateIndex + 1;
						break;
					}
				}
			}
			else
				stateInfo[stateIndex].transitions[charIndex] = 0;
		}
	}
}
DFA::DFA(const DFA &dfa)
{
	struct transition
	{
		size_t fromOld, fromNew, to;
		bool marked;
	};
	std::vector<size_t> states;
	states.reserve(dfa.stateInfo.size());
	size_t numStates = 1;
	if (dfa.stateInfo[0].accepting)
		for (auto info : dfa.stateInfo)
			states.push_back(info.accepting ? 1 : (numStates = 2));
	else
		for (auto info : dfa.stateInfo)
			states.push_back(info.accepting ? (numStates = 2) : 1);
	bool consistent = false;
	while (!consistent)
	{
		consistent = true;
		for (size_t charIndex = 1; charIndex < NFA::AlphabetSize(); charIndex++)
		{
			std::vector<transition> transitions;
			transitions.reserve(states.size());
			std::vector<size_t> currentTransVals(numStates);
			for (size_t dfaState = states.size(); dfaState-- > 0;)
			{
				size_t trans = (dfa.stateInfo[dfaState].transitions[charIndex] == 0) ?
					0 : states[dfa.stateInfo[dfaState].transitions[charIndex] - 1];
				currentTransVals[states[dfaState] - 1] = trans;
				transitions.push_back({ dfaState, states[dfaState], trans });
			}
			for (size_t i = transitions.size(); i-- > 0;)
			{
				if (transitions[i].marked == false)
				{
					if (transitions[i].to != currentTransVals[transitions[i].fromNew - 1])
					{
						consistent = false;
						states[transitions[i].fromOld] = ++numStates;
						currentTransVals[transitions[i].fromNew - 1] = transitions[i].to; //experimental: num_states
					}
					for (size_t j = i; j-- > 0;)
					{
						if ((transitions[j].marked == false) && (transitions[j].fromNew == transitions[i].fromNew) &&
							(transitions[j].to == currentTransVals[transitions[j].fromNew - 1]))
						{
							transitions[j].marked = true;
							states[transitions[j].fromOld] = states[transitions[i].fromOld];
						}
					}
				}
			}
		}
	}
	stateInfo = std::vector<StateInfo>(numStates);
	for (size_t i = 0; i < states.size(); i++)
	{
		stateInfo[states[i] - 1].accepting = dfa.stateInfo[i].accepting;
		for (size_t j = 1; j < NFA::AlphabetSize(); j++)
			stateInfo[states[i] - 1].transitions[j] = (dfa.stateInfo[i].transitions[j] == 0) ? 0 : states[dfa.stateInfo[i].transitions[j] - 1];
	}
}
bool DFA::isNonempty(const std::vector<bool> &subset)
{
	for (auto element : subset)
		if (element == true)
			return true;
	return false;
}

void DFA::PrintStates() const
{
	for (size_t i = 0; i < stateInfo.size(); i++)
	{
		std::cout << "State " << i + 1 << ": " << (stateInfo[i].accepting ? "Accepting\n" : "\n");
		for (size_t j = 1; j < NFA::AlphabetSize(); j++)
			if (stateInfo[i].transitions[j] != 0)
				std::cout << "\tMove(" << i + 1 << ", " << NFA::Alphabet(j) << ") = " << stateInfo[i].transitions[j] << std::endl;
	}
}
void DFA::PrintHeaders(std::ostream &out) const
{
	for (size_t i = 0; i < stateInfo.size(); )
		out << "bool State_" << ++i << "(std::string::const_iterator it, std::string::const_iterator end);" << std::endl;
}
void DFA::PrintDefinitions(std::ostream &out) const
{
	for (size_t i = 0; i < stateInfo.size(); i++)
	{
		out << "bool State_" << i + 1 << "(std::string::const_iterator it, std::string::const_iterator end)\n";
		out << "{\n";
		out << "\tif (it != end)\n";
		out << "\t{\n";
		out << "\t\tswitch (*it++)\n";
		out << "\t\t{\n";
		for (size_t j = 1; j < NFA::AlphabetSize(); j++)
		{
			if (stateInfo[i].transitions[j] != 0)
			{
				out << "\t\tcase '" << NFA::Alphabet(j) << "':\n";
				out << "\t\t\treturn State_" << stateInfo[i].transitions[j] << "(it, end);\n";
			}
		}
		out << "\t\tdefault :\n";
		out << "\t\t\treturn false;\n";
		out << "\t\t}\n";
		out << "\t}\n";
		out << "\telse\n";
		out << "\t\treturn " << (stateInfo[i].accepting ? "true;\n" : "false;\n");
		out << "}" << std::endl;
	}
}

Tree::Tree(const std::string &input)
{
	std::string::const_iterator it = input.begin(), end = input.end();
	if (it == end)
		throw "Tree::Tree 1: Syntax Error!";
	else if (*it == '(' || (*it >= 'a' && *it <= 'z') || (*it >= 'A' && *it <= 'Z'))
	{
		node = pNode(new Q(it, end));
		if (it != end)
			throw "Tree::Tree 2: Syntax Error!";
	}
	else
		throw "Tree::Tree 3: Syntax Error!";
}
Q::Q(std::string::const_iterator &it, std::string::const_iterator end)
{
	if (it == end)
		throw "Q::Q 1: Syntax Error!";
	else if (*it == '(' || (*it >= 'a' && *it <= 'z') || (*it >= 'A' && *it <= 'Z'))
	{
		nodes.emplace_back(new S(it, end));
		nodes.emplace_back(new R(it, end));
	}
	else
		throw "Q::Q 2: Syntax Error!";
}
NFA Q::GenNfa(NFA &&nfa) const
{
	return nodes[1]->GenNfa(nodes[0]->GenNfa());
}
R::R(std::string::const_iterator &it, std::string::const_iterator end)
{
	if (it == end || *it == ')');
	else if (*it == '|')
	{
		nodes.emplace_back(new Terminal('|'));
		it++;
		nodes.emplace_back(new S(it, end));
		nodes.emplace_back(new R(it, end));
	}
	else
		throw "R::R 1: Syntax Error!";
}
NFA R::GenNfa(NFA &&nfa) const
{
	if (nodes.empty())
		return std::move(nfa);
	return NFA::Or(std::move(nfa), nodes[2]->GenNfa(nodes[1]->GenNfa()));
}
S::S(std::string::const_iterator &it, std::string::const_iterator end)
{
	if (it == end)
		throw "S::S 1: Syntax Error!";
	else if (*it == '(' || (*it >= 'a' && *it <= 'z') || (*it >= 'A' && *it <= 'Z'))
	{
		nodes.emplace_back(new U(it, end));
		nodes.emplace_back(new T(it, end));
	}
	else
		throw "S::S 2: Syntax Error!";
}
NFA S::GenNfa(NFA &&nfa) const
{
	return nodes[1]->GenNfa(nodes[0]->GenNfa());
}
T::T(std::string::const_iterator &it, std::string::const_iterator end)
{
	if (it == end || *it == '|' || *it == ')');
	else if (*it == '(' || (*it >= 'a' && *it <= 'z') || (*it >= 'A' && *it <= 'Z'))
	{
		nodes.emplace_back(new U(it, end));
		nodes.emplace_back(new T(it, end));
	}
	else
		throw "T::T 1: Syntax Error!";
}
NFA T::GenNfa(NFA &&nfa) const
{
	if (nodes.empty())
		return std::move(nfa);
	return nodes[1]->GenNfa(NFA::Concatenate(std::move(nfa), nodes[0]->GenNfa()));
}
U::U(std::string::const_iterator &it, std::string::const_iterator end)
{
	if (it == end)
		throw "U::U 1: Syntax Error!";
	else if (*it == '(' || (*it >= 'a' && *it <= 'z') || (*it >= 'A' && *it <= 'Z'))
	{
		nodes.emplace_back(new W(it, end));
		nodes.emplace_back(new V(it, end));
	}
	else
		throw "U::U 2: Syntax Error!";
}
NFA U::GenNfa(NFA &&nfa) const
{
	return nodes[1]->GenNfa(nodes[0]->GenNfa());
}
V::V(std::string::const_iterator &it, std::string::const_iterator end)
{
	if (it == end || *it == '|' || *it == '(' || *it == ')' || (*it >= 'a' && *it <= 'z') || (*it >= 'A' && *it <= 'Z'));
	else if (*it == '*')
	{
		nodes.emplace_back(new Terminal('*'));
		it++;
		nodes.emplace_back(new V(it, end));
	}
	else
		throw "V::V 1: Syntax Error!";
}
NFA V::GenNfa(NFA &&nfa) const
{
	if (nodes.empty())
		return std::move(nfa);
	return NFA::Star(nodes[1]->GenNfa(std::move(nfa)));
}
W::W(std::string::const_iterator &it, std::string::const_iterator end)
{
	if (it == end)
		throw "W::W 1: Syntax Error!";
	else if ((*it >= 'a' && *it <= 'z') || (*it >= 'A' && *it <= 'Z'))
	{
		nodes.emplace_back(new Terminal(*it));
		it++;
	}
	else if (*it == '(')
	{
		nodes.emplace_back(new Terminal('('));
		it++;
		nodes.emplace_back(new Q(it, end));
		if (it != end && *it == ')')
			nodes.emplace_back(new Terminal(')'));
		else
			throw "W::W 2: Syntax Error!";
		it++;
	}
	else
		throw "W::W 3: Syntax Error!";
}
NFA W::GenNfa(NFA &&nfa) const
{
	if (nodes.size() == 1)
		return nodes[0]->GenNfa();
	return nodes[1]->GenNfa();
}