package main

import (
	"log"
	"net"
)

func main() {
	// use tcp to listen on port 7000, print what request we get
	server, err := net.Listen("tcp", "127.0.0.1:7000")
	if err != nil {
		log.Printf("%v", err)
		return
	}
	defer server.Close()
	log.Printf("Server started on %s", server.Addr().String())
	for {
		conn, err := server.Accept()
		defer conn.Close()
		if err != nil {
			log.Printf("%v", err)
			continue
		}
		go handle(conn)
	}
}
func handle(conn net.Conn) {
	for {
		buf := make([]byte, 1024)
		_, err := conn.Read(buf)
		if err != nil {
			log.Printf("%v", err)
			return
		}
		log.Printf("Received: %s", buf)
		_, err = conn.Write(buf)
		if err != nil {
			log.Printf("%v", err)
			return
		}
		log.Printf("Sent")
	}
}
