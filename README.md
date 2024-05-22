# ws_server
This is a web socket server implemented with C.</br>
You can quickly start the server by specifying IP, Port, and message processing functions.</br>
The project also has built-in lightweight logger.

# example
```c
#include <unistd.h>
#include <pthread.h>
#include "./src/wbsocket_server.h"

long msg_switch(unsigned char *req, long req_len, unsigned char *res, long res_len)
{
	bzero(res, res_len);
	long len = sprintf((char *)res, "Recv %s Successfully", req);
	return len;
}

void test_ws_server()
{
	logger_init("./log");

	start_server("172.17.83.59", "52323", msg_switch);

	logger_destroy();
}

int main(void)
{
	test_ws_server();
	return 0;
}
```
