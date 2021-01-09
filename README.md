## Multiplayer TicTacToe game
Multiplayer TicTacToe game is based on client-server model using socket programming in C. Server communicates with client to see if they are ready to join. The players in LAN can connect to the server using IPv4 address of server provided. Each connection is opened in a new socket, so it supports any number of simultaneous games. Server is implemented with management of logins.

## Usage
Compiling
```
make all
```
First run the server
```
./server
```

Run clients with server IP

```
./client IPv4
```
## Screenshots
<img src="images/server.png" />
<img src="images/welcome.png" />
<img src="images/playing.png" />
<img src="images/score.png" />

## Authors

* **Jan Kuliga**: [kuliga](https://github.com/kuliga)
* **Konrad Sikora**: [konrad516](https://github.com/konrad516)
* **Krzysztof Bera**: [krzysiubera](https://github.com/krzysiubera)

