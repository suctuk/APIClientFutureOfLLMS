# APIClientFutureOfLLMS

### API:
To call all endpoints, use the link https://apiclientfutureofllms.onrender.com

For example https://apiclientfutureofllms.onrender.com/test will return "hello world"


To compile:

g++ -o radio_client main.cpp Client.cpp -lcurl -I/opt/homebrew/include -L/opt/homebrew/lib -std=c++17 -pthread

./radio_client

python python_client.py