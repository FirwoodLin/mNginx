# 请求示例

client -> mNginx

```text
Request URL: http://localhost/api/v1/healthy
Request Method: GET
\r\n
Request Header:
Host: localhost
User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/117.0.0.0 Safari/537.36 Edg/117.0.2045.40
Accept: */*
\r\n
\r\n
```

mNginx -> real server

```text
Request URL: http://127.0.0.1:7000/api/v1/healthy
Request Method: GET
\r\n
Request Header:
Host: test.com
User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/117.0.0.0 Safari/537.36 Edg/117.0.2045.40
Accept: */*
\r\n
\r\n
```

mNginx -> real server -- 注释版

```text
Request URL: http://127.0.0.1:7000/api/v1/healthy ## 从 proxy_pass 获取
Request Method: GET
\r\n
Request Header:
Host: test.com  ## 从 proxy_set_header 获取
User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/117.0.0.0 Safari/537.36 Edg/117.0.2045.40
Accept: */*
\r\n
\r\n
```

# 配置信息

mNginx 配置

```text
location /api/v1 {
    proxy_set_header Host $test.com;
    // 
    proxy_pass http: 127.0.0.1:7000;
    // 反向代理的目标服务器
}
```

# 响应示例

html 文件

- 文件长度
- 当前时间
- 修改时间

```text
HTTP/1.1 200 OK
Content-Length: 143
Accept-Ranges: bytes
Content-Type: text/html; charset=utf-8
Date: Tue, 03 Oct 2023 07:05:39 GMT
Last-Modified: Tue, 03 Oct 2023 03:58:46 GMT
```

txt

- 文件长度
- 当前时间
- 修改时间

```text
HTTP/1.1 200 OK
Content-Length: 33
Accept-Ranges: bytes
Content-Type: text/plain; charset=utf-8
Date: Tue, 03 Oct 2023 05:12:58 GMT
Last-Modified: Tue, 03 Oct 2023 05:10:25 GMT
```

png 文件

- 文件长度
- 当前时间
- 修改时间

```text
HTTP/1.1 200 OK
Content-Length: 142544
Accept-Ranges: bytes
Content-Type: image/png
Date: Tue, 03 Oct 2023 05:19:42 GMT
Last-Modified: Tue, 03 Oct 2023 03:48:47 GMT
```
