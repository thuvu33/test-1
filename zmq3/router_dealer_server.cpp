// spojenie typu ROUTER <-> DEALER, serverovska cast implementovana v zmq
#include <string>
#include <iostream>
#include <cassert>
#include <zmq.h>

using std::string;
using std::cout;


int main(int argc, char * argv[])
{
	void * ctx = zmq_ctx_new();
	void * socket = zmq_socket(ctx, ZMQ_ROUTER);
	int rc = zmq_bind(socket, "tcp://*:5557");
	assert(rc == 0 && "unsuccessful binding");

	while (true)
	{
		// odpoved pride ako (identity, message)
		char identity[1024];
		int identity_size = zmq_recv(socket, identity, 1024, 0);
		assert(rc != -1);
		string s(identity, identity_size);
		cout << "identity: " << s << std::endl;

		// zisti, ci je sprav viac
		int more;
		size_t option_len = sizeof(more);
		int rc = zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &option_len);
		assert(rc == 0);
		assert(more == 1 && "este cakam spravu");

		if (more)
		{
			char d[1024];
			rc = zmq_recv(socket, d, 1024, 0);
			assert(rc != -1);
			string s(d, rc);
			cout << "message: " << s << std::endl;
		}

		// answer
		zmq_send(socket, identity, identity_size, ZMQ_SNDMORE);
		zmq_send(socket, "world", 5, 0);
	}

	return 0;
}
