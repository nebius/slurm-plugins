package main

import (
	"log"
	"os"

	"github.com/nebius/nccl-inspector-preconf/internal/utils/optionrender"
)

// main renders the generated SPANK option source to stdout.
func main() {
	if err := optionrender.Render(os.Stdout); err != nil {
		log.Fatal(err)
	}
}
