package main

import (
	"fmt"
	"log"
	"net/http"
	"net/http/httputil"
	"os"
)

var serverPort = 1235

func main() {
	// write a http client to send request to localhost:7000
	// and print the response body
	requestURL := fmt.Sprintf("http://localhost:%d", serverPort)
	res, err := http.Get(requestURL)
	if err != nil {
		fmt.Printf("error making http request: %s\n", err)
		os.Exit(1)
	}
	fmt.Printf("client: got response!\n")
	dump, err := httputil.DumpResponse(res, true)
	if err != nil {
		log.Fatal(err)
	}
	//fmt.Printf("%q", dump)
	fmt.Printf("%s", dump)// get the raw response
}
