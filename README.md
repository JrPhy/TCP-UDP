網路在現今的世界已經是不可或缺的應用了，要讓使用者連接上網發訊息要靠硬體跟軟體的合作。在此基礎上建立了 OSI 七層的模型，從底層的硬體到上層的軟體，各層名稱與用途如下圖所示
```
+----------------------+-----------------------------+
|   應用層 (Application) | HTTP, FTP, SMTP, DNS      |
+----------------------+-----------------------------+
|   表達層 (Presentation)| SSL/TLS, JPEG, MPEG       |
+----------------------+-----------------------------+
|   會議層 (Session)     | RPC, NetBIOS              |
+----------------------+-----------------------------+
|   傳輸層 (Transport)   | TCP, UDP                  |
+----------------------+-----------------------------+
|   網路層 (Network)     | IP, ICMP, 路由器 (Router)  |
+----------------------+-----------------------------+
|   資料鏈結層 (Data Link)| Ethernet, PPP, Switch, MAC|
+----------------------+-----------------------------+
|   實體層 (Physical)    | 電纜, 光纖, Hub, Wi-Fi 信號 |
+----------------------+-----------------------------+
```
軟體的部分就是在傳輸層以上的 4~7 層，主要有 TCP 與 UDP 兩種連接方式。兩者差異在於是否握手，前者會等待對方回覆確保連接，常用於檔案傳輸或是登入等需要**可靠性**時使用，後者則是只將資料送出不論對方是否收到，常用於影音串流等需要**即時性**時使用。

在寫程式時會有一個是 server 另一個是 client，server 會綁定地址等 client 送資料過來，下方程式碼會比較 TCP 與 UDP 的差異。
SERVER
```C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int server_fd, client_fd;  // UDP 不需要 client_fd，但保留原本
    struct sockaddr_in server_addr, client_addr;
    char buffer[1024] = {0};

    // 原本 TCP：server_fd = socket(AF_INET, SOCK_STREAM, 0);
    // 改成 UDP：
    server_fd = socket(AF_INET, SOCK_DGRAM, 0); // 建立 UDP socket

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // TCP 專用：listen() + accept()
    // listen(server_fd, 3);
    // socklen_t addrlen = sizeof(client_addr);
    // client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);

    socklen_t addrlen = sizeof(client_addr);

    // TCP 用 read()，UDP 改用 recvfrom()
    // read(client_fd, buffer, sizeof(buffer));
    int n = recvfrom(server_fd, buffer, sizeof(buffer), 0,
                     (struct sockaddr*)&client_addr, &addrlen);
    buffer[n] = '\0';
    printf("收到: %s\n", buffer);

    // TCP 用 send()，UDP 用 sendto()
    // char *reply = "Hello from TCP server";
    char *reply = "Hello from UDP server";
    // send(client_fd, reply, strlen(reply), 0);
    sendto(server_fd, reply, strlen(reply), 0,
           (struct sockaddr*)&client_addr, addrlen);

    // TCP 關閉 client_fd
    // close(client_fd);
    close(server_fd);
    return 0;
}

```
CLIENT
```C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[1024] = {0};

    // 原本 TCP：sock = socket(AF_INET, SOCK_STREAM, 0);
    // 改成 UDP：
    sock = socket(AF_INET, SOCK_DGRAM, 0); // 建立 UDP socket

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // TCP 用 connect()
    // connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // 原本 TCP 用 send()，UDP 用 sendto()
    // char *msg = "Hello TCP server";
    char *msg = "Hello UDP server";
    // send(sock, msg, strlen(msg), 0);
    sendto(sock, msg, strlen(msg), 0,
           (struct sockaddr*)&server_addr, sizeof(server_addr));

    // 原本 TCP 用 read()，UDP 改用 recvfrom()
    // read(sock, buffer, sizeof(buffer));
    socklen_t addrlen = sizeof(server_addr);
    int n = recvfrom(sock, buffer, sizeof(buffer), 0,
                     (struct sockaddr*)&server_addr, &addrlen);
    buffer[n] = '\0';
    printf("伺服器回覆: %s\n", buffer);

    close(sock);
    return 0;
}
