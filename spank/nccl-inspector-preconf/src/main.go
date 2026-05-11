package main

/*
#cgo CFLAGS: -Wall -Wextra
*/
import "C"

var _ C.int

// main is required for the Go main package but is unused in the built plugin.
func main() {}
