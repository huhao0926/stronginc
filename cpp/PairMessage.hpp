#ifndef UTILS_PAIRMESSAGE_HPP_
#define UTILS_PAIRMESSAGE_HPP_

#include "cpp/global.h"
#include "cpp/Message.hpp"

template<class T>
class PairMessage: public Message {
	public:
	PairMessage() {
	}

	PairMessage(VertexID _u, T _val): u(_u), val(_val) {
	}

	VertexID getID() {
		return u;
	}

	T getValue() {
		return val;
	}

	public:
	VertexID u;
	T val;
};

#endif
