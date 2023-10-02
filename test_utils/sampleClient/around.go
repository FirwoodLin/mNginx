package main

import (
	"bufio"
	"fmt"
	"net"
	"os"
	"strings"
)

var (
	addr = "127.0.0.1:1235"
)

func main() {
	Fixed()
}

// Fixed 向mNginx发送固定的消息,并回显接收到的
func Fixed() {
	// use tcp to send data to port 7000, print what we get back
	conn, err := net.Dial("tcp", addr)
	if err != nil {
		fmt.Println("err : ", err)
		return
	}
	fmt.Println("client succeed in conn")
	defer conn.Close()                   // 关闭TCP连接
	_, err = conn.Write([]byte("12345")) // 发送数据
	if err != nil {
		return
	}
	buf := [512]byte{} // 接收数据
	n, err := conn.Read(buf[:])
	if err != nil {
		fmt.Println("recv failed, err:", err)
		return
	}
	fmt.Println(string(buf[:n]))
}

// FromStdIn 向mNginx发送 stdin 中的数据,并回显接收到的
func FromStdIn() {
	// use tcp to send data to port 7000, print what we get back
	conn, err := net.Dial("tcp", addr)
	if err != nil {
		fmt.Println("err : ", err)
		return
	}
	fmt.Println("client succeed in conn")
	defer conn.Close() // 关闭TCP连接
	inputReader := bufio.NewReader(os.Stdin)
	for {
		input, _ := inputReader.ReadString('\n') // 读取用户输入
		inputInfo := strings.Trim(input, "\r\n")
		if strings.ToUpper(inputInfo) == "Q" { // 如果输入q就退出
			return
		}
		_, err := conn.Write([]byte(inputInfo)) // 发送数据
		if err != nil {
			return
		}
		buf := [512]byte{}
		n, err := conn.Read(buf[:])
		if err != nil {
			fmt.Println("recv failed, err:", err)
			return
		}
		fmt.Println(string(buf[:n]))
	}
}
