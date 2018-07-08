#ifndef UTILS_SERIALIZATION_H_
#define UTILS_SERIALIZATION_H_

#include <vector>
#include <set>
#include <string>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include "cpp/core/config.h"
#include "cpp/core/global.h"
#include "PairMessage.hpp"
#include "cpp/core/vertex.h"
#include "cpp/core/edge.h"
#include "cpp/core/strongr.h"
class ibinstream {
private:
	std::vector<char> buf;

public:
	char* get_buf() {
		return &buf[0];
	}

	size_t size() {
		return buf.size();
	}

	void raw_byte(char c) {
		buf.push_back(c);
	}

	void raw_bytes(const void* ptr, int size) {
		buf.insert(buf.end(), (const char*) ptr, (const char*) ptr + size);
	}
};

ibinstream& operator<<(ibinstream& m, size_t i);

ibinstream& operator<<(ibinstream& m, bool i);

ibinstream& operator<<(ibinstream& m, int i) ;

ibinstream& operator<<(ibinstream& m, long i) ;


ibinstream& operator<<(ibinstream& m, double i);

ibinstream& operator<<(ibinstream& m, char c);

template<class T1, class T2>
ibinstream& operator<<(ibinstream &m, const std::pair<T1, T2> &p) {
	m << p.first;
	m << p.second;
	return m;
}

template<class T>
ibinstream& operator<<(ibinstream& m, const T* p) {
	return m << *p;
}

/*
template<class T>
ibinstream& operator<<(ibinstream& m, const T p) {
	m.raw_bytes(&p, sizeof(T));
	return m;
}
*/


template<class T>
ibinstream& operator<<(ibinstream& m, const std::vector<T>& v) {
	m << v.size();//这个也是重载输出流运算符，要调用ibinstream& operator<<(ibinstream& m, int i)函数
	for (const auto& it : v) {
		m << it;
	}
	return m;
}

template<>
ibinstream& operator<<(ibinstream& m, const std::vector<int>& v) ;

template<>
ibinstream& operator<<(ibinstream& m, const std::vector<double>& v) ;

template<>
ibinstream& operator<<(ibinstream& m, const std::vector<long>& v);

template<class T>
ibinstream& operator<<(ibinstream& m, const std::set<T>& v) {
	m << v.size();
	for (const auto& it : v) {
		m << it;
	}
	return m;
}

ibinstream& operator<<(ibinstream& m, const std::string& str) ;

template<class KeyT, class ValT>
ibinstream& operator<<(ibinstream& m, const std::map<KeyT, ValT>& v) {
	m << v.size();
	for (const auto& it : v) {
		m << it.first;
		m << it.second;
	}
	return m;
}
template<class KeyT, class ValT>
ibinstream& operator<<(ibinstream& m, const std::unordered_map<KeyT, ValT>& v)
{
	m << v.size();
	for (const auto& it : v) {
		m << it.first;
		m << it.second;
	}
	return m;
}

template<class T>
ibinstream& operator<<(ibinstream& m, const std::unordered_set<T>& v)
{
	m << v.size();
	for (const auto& it : v) {
		m << it;
	}
	return m;
}

class obinstream {
private:
	char* buf; //responsible for deleting the buffer, do not delete outside
	size_t size;
	size_t index;

public:
	obinstream(char* b, size_t s) :
		buf(b), size(s), index(0) {
	}
	;
	obinstream(char* b, size_t s, size_t idx) :
		buf(b), size(s), index(idx) {
	}
	;
	~obinstream() {
		delete[] buf;
	}

	char raw_byte() {
		return buf[index++];
	}

	void* raw_bytes(unsigned int n_bytes) {
		char* ret = buf + index;
		index += n_bytes;
		return ret;
	}

};

obinstream& operator>>(obinstream& m, size_t& i);

obinstream& operator>>(obinstream& m, bool& i) ;

obinstream& operator>>(obinstream& m, int& i) ;

obinstream& operator>>(obinstream& m, long& i) ;

obinstream& operator>>(obinstream& m, double& i);

obinstream& operator>>(obinstream& m, char& c);

template<class T1, class T2>
obinstream& operator>>(obinstream &m, std::pair<T1, T2> &p) {
	m >> p.first;
	m >> p.second;
	return m;
}

template<class T>
obinstream& operator>>(obinstream& m, T*& p) {
	p = new T;
	return m >> (*p);
}


/*
template<class T>
obinstream& operator>>(obinstream& m, T& p) {
	p=*(T*) m.raw_bytes(sizeof(T));
	return m;
}
*/



template<class T>
obinstream& operator>>(obinstream& m, std::vector<T>& v) {
	size_t size;
	m >> size;
	v.resize(size);
	for (auto& it : v) {
		m >> it;
	}
	return m;
}

template<>
obinstream& operator>>(obinstream& m, std::vector<int>& v) ;

template<>
obinstream& operator>>(obinstream& m, std::vector<double>& v) ;

template<>
obinstream& operator>>(obinstream& m, std::vector<long>& v);

template<class T>
obinstream& operator>>(obinstream& m, std::set<T>& v) {
	size_t size;
	m >> size;
	for (size_t i = 0; i < size; i++) {
		T tmp;
		m >> tmp;
		v.insert(v.end(), tmp);
	}
	return m;
}

obinstream& operator>>(obinstream& m, std::string& str);
template<class KeyT, class ValT>
obinstream& operator>>(obinstream& m, std::map<KeyT, ValT>& v) {
	size_t size;
	m >> size;
	for (int i = 0; i < size; i++) {
		KeyT key;
		m >> key;
		m >> v[key];
	}
	return m;
}
template<class KeyT, class ValT>
obinstream& operator>>(obinstream& m, std::unordered_map<KeyT, ValT>& v)
{
	size_t size;
	m >> size;
	for (int i = 0; i < size; i++) {
		KeyT key;
		m >> key;
		m >> v[key];
	}
	return m;
}

template<class T>
obinstream& operator>>(obinstream& m, std::unordered_set<T>& v)
{
	size_t size;
	m >> size;
	for (int i = 0; i < size; i++) {
		T key;
		m >> key;
		v.insert(key);
	}
	return m;
}

//FIXME: all the codes below may be bugy

// serialization of Vertex
//template<class VD>
ibinstream &operator <<(ibinstream &m, const Vertex &v);

//template<class VD>
obinstream &operator >>(obinstream &m, Vertex &v);

// serialization of Edge
//template<class ED>
ibinstream &operator <<(ibinstream &m, const Edge &e);

//template<class ED>
obinstream &operator >>(obinstream &m, Edge &e);

//serialization of strongresult
ibinstream &operator <<(ibinstream &m, const StrongR &strongr);

obinstream &operator >>(obinstream &m,  StrongR &strongr);
//serialization of PairMessage
template<class T>
ibinstream &operator <<(ibinstream &m, const PairMessage<T> &message);

template<class T>
obinstream &operator >>(obinstream &m, PairMessage<T> &message);
/*
template<class VD, class ED>
obinstream &operator >>(obinstream &m, Query<VD, ED> &q) {

	m >> q.terminate;
	m >> q.bound;
	m >> q.root;

	m >> q.keys;

	m >> q.attrs;
	m >> q.edges;

	m >> q.numVertices;
	m >> q.numEdges;

	m >> q.last >> q.head;
	m >> q.pre >> q.to >> q.next >> q.from;

	m >> q.sigIncoming1hop >> q.sigOutgoing1hop >> q.sigIncoming2hop >> q.sigOutgoing2hop;

	return m;
}
*/
#endif /* UTILS_SERIALIZATION_H_ */
