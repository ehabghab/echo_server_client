# This is an echo server



Building server & client 
```
make
```

Running server & client 
```
shell_1 >> ./server
shell_2 >> ./client
```

Client output
```
[16:57:10 Client_.cpp:61] Please enter your msg: Hello!
[16:57:10 Client_.cpp:74] Hello! --> server @127.0.0.1:7777
[16:57:10 Client_.cpp:82] Hello! <-- server @127.0.0.1:7777
[16:57:10 Client_.cpp:61] Please enter your msg: <waiting for input>
```

Server output
```
[16:57:11 Server_.cpp:58] Listening @ PORT:7777
[16:57:11 Server_.cpp:76] Connected!
[16:57:11 Server_.cpp:96] Receving Hello! <-- client
[16:57:11 Server_.cpp:100] Echoing Hello! --> client
```
