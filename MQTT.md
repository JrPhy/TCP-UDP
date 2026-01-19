相較於 HTTP，MQTT 是一種發布(publish)-訂閱(subscribe)的模式，並由一個 BROKER 來管理誰訂閱了什麼主題，把這些資料保存在本地。連線依然是用 TCP。在傳遞封包時的大小就會小非常多，約只有 HTTP 的 1/10 到 1/20，畢竟 MQTT 已經知道對方是誰了，所以在效能上會比 HTTP 還要好。實務上也不希望中間連線有中斷，所以會一直保持連線。

| 傳輸內容 | HTTP 封包大小 | MQTT 封包大小 |
|----------|---------------|---------------|
| 傳送「溫度：25°C」 | 約 300–500 bytes | 約 20–30 bytes |
| 傳送 JSON `{ "temp": 25 }` | 約 400–600 bytes | 約 30–40 bytes |
| 大量感測器數據 (1000 筆) | 幾百 KB | 幾十 KB |
| 效率 | 適合網頁、API，開銷大 | 適合 IoT，即時、低耗能 |

## 一、BROKER
在一些輕量級的應用，可以在程式內部使用 unordered_map，或是在外部建立一個資料來儲存訂閱者
```C++
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>

class Broker {
private:
    std::unordered_map<std::string, std::vector<std::string>> subscriptions;

public:
    void subscribe(const std::string& topic, const std::string& clientId) {
        subscriptions[topic].push_back(clientId);
        std::cout << clientId << " 訂閱了主題: " << topic << std::endl;
    }

    void publish(const std::string& topic, const std::string& message) {
        std::cout << "Broker 收到訊息: " << topic << " -> " << message << std::endl;
        if (subscriptions.find(topic) != subscriptions.end()) {
            for (const auto& clientId : subscriptions[topic]) {
                std::cout << "送給 " << clientId << ": " << message << std::endl;
            }
        } else {
            std::cout << "目前沒有訂閱者。" << std::endl;
        }
    }
};
```
若使用 FLASK 框架則簡潔許多
```PYTHON
from flask import Flask, request
from flask_socketio import SocketIO, emit, join_room

app = Flask(__name__)
socketio = SocketIO(app)

# 訂閱者加入某個主題 (用房間 room 模擬)
@socketio.on('subscribe')
def handle_subscribe(data):
    topic = data['topic']
    join_room(topic)
    emit('message', f'已訂閱主題: {topic}')

# 發佈者發佈訊息到某個主題
@socketio.on('publish')
def handle_publish(data):
    topic = data['topic']
    message = data['message']
    emit('message', f'{topic} -> {message}', room=topic)

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000)
```
發布後就可以在接收端去根據訊息動作，例如調整空調溫度等。

## 二、IOT 架構
現在網路方便，除了一些資安的議題外通常都是使用網路連接設備與 BROKER，會架設本地或是雲端的，然後再利用外部設備如手機或電腦連進 BROKER 再發訊息給 IOT 裝置。
