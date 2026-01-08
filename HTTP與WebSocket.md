此兩種協議皆為應用層，HTTP 建立在 TCP 之上，WebSocket 建立於 HTTP 之上。

## 一、HTTP
是一種「請求-響應」的過程，例如在瀏覽網頁時，使用者發出了請求(網址)，另一端就回傳網頁內容與狀態 (HTML+CSS+JS, status)，例如常看到的 404 就是其中一種狀態。以下為其中一種請求例子
```
GET /index.html HTTP/1.1
Host: www.example.com
User-Agent: Mozilla/5.0
Accept: text/html
```
響應
```
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 1234

<html>...</html>
```
```
Client ---------------------> Server
        (Request)
Client <--------------------- Server
        (Response)
每次都要重新建立連線，回應後斷開
```
狀態有以下幾種
```
1xx → 訊息——請求已被伺服器接收，繼續處理
2xx → 成功——請求已成功被伺服器接收、理解、並接受
3xx → 重新導向——需要後續操作才能完成這一請求
4xx → 請求錯誤——請求含有詞法錯誤或者無法被執行
5xx → 伺服器錯誤——伺服器在處理某個正確請求時發生錯誤
```
HTTP 使用方法除了 GET 外還有以下幾種，RESTful API 就是基於此方法再去撰寫，前後端交換資料很常用到 GET/POST
```
GET => 讀取（R）
POST => 新增（C）
PUT/PATCH => 更新（U）
DELETE => 刪除（D）
```
特點就是響應完就斷線，當然也可以用 keep-alive 的方式保持連線，但是會一直需要傳遞標頭所以其實會需要一直握手，若要雙向溝通則需要用到 WebSocket。

## 二、WebSocket
與 HTTP 的差異在於 WebSocket 可以雙向溝通，且建立連線後就保持連線，不像 HTTP keep-alive 一樣需要一直握手傳標頭，適合用於即時通訊或是 IOT 裝置。現在建立 WebSocket 也有許多套件可以用，在此就以 FLASK 作後端範例，前端就是 js。在傳遞訊息時本身是未加密的，通常希望只有接收跟發送方知道，中間攔截者就算攔截到也無法知道內容，所以在 server 端就需要有公鑰跟私鑰，在前端就可以使用 wss 來做加密通訊，所以現在都會強調網址開頭是 ```https```，後面的 s 就是 security 的意思。不過加密後通常密文檔案會比較大。
```後端
#pip install flask flask-socketio 需要安裝

from flask import Flask, render_template
from flask_socketio import SocketIO, emit

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app)

@app.route('/')
def index():
    return render_template('chat.html')

@socketio.on('message')
def handle_message(msg):
    print("收到訊息:", msg)
    emit('response', f"伺服器回覆: {msg}", broadcast=True)

if __name__ == '__main__':
    # 啟動時指定憑證與金鑰
    socketio.run(app,
                 host='0.0.0.0',
                 port=5000,
                 ssl_context=('cert.pem', 'key.pem'))
```
```前端
<!DOCTYPE html>
<html>
<head>
  <title>安全 WebSocket 測試</title>
  <script src="https://cdn.socket.io/4.0.0/socket.io.min.js"></script>
</head>
<body>
  <h1>安全 WebSocket 測試</h1>
  <input id="msg" placeholder="輸入訊息">
  <button onclick="sendMessage()">送出</button>
  <ul id="messages"></ul>

  <script>
    // 使用 wss:// 連線 (加密)
    const socket = io("wss://localhost:5000", {
      transports: ["websocket"]
    });

    socket.on("response", function(data) {
      const li = document.createElement("li");
      li.textContent = data;
      document.getElementById("messages").appendChild(li);
    });

    function sendMessage() {
      const msg = document.getElementById("msg").value;
      socket.send(msg);
    }
  </script>
</body>
</html>
```
