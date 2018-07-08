#ifndef CORE_MESSAGEBUFFER_H_
#define CORE_MESSAGEBUFFER_H_

#include <stddef.h>
#include <vector>

#include "cpp/core/global.h"
#include "communication.h"
extern unsigned cnt;

template<class MessageT>
class MessageBuffer {
public:
	std::vector<std::vector<MessageT>> out_messages;
	std::vector<MessageT> in_messages;
	unsigned tag=0;

	void init(int num_peers = get_num_workers()) {
		//tag = cnt;
		tag = 0;
		for (int i = 0; i < num_peers; i++) {
			std::vector<MessageT> msgBuf;
			out_messages.push_back(msgBuf);
		}

	}

	void add_message(const int dest, const MessageT& msg) {
		out_messages[dest].push_back(msg);
	}

	std::vector<MessageT>& get_messages() {
		return in_messages;
	}

	void reset_in_messages(){
		in_messages.clear();
	}

	void reset_out_messages(){
//	    out_messages.clear();
	}

	void sync_messages() {

		int num_worker = get_num_workers();
		int me = get_worker_id();

		all_to_all(out_messages, tag);

		// gather all messages
		for (int i = 0; i < num_worker; i++) {
			std::vector<MessageT>& msgBuf = out_messages[i];
			in_messages.insert(in_messages.end(), msgBuf.begin(), msgBuf.end());
		}

		for (int i = 0; i < num_worker; ++i) {
			out_messages[i].clear();
		}
		return;
	}

	int exchange_message_size(){
	    if (in_messages.size()!=0){
	        return 1;
	    }
//	    for(int i=0 ;i <in_messages.size(); ++i){
//	        if(in_messages[i].size() !=0){
//	            num_sum+=1;
//	        }
//	    }
	    return 0;
	}

};
#endif /* CORE_MESSAGEBUFFER_H_ */
