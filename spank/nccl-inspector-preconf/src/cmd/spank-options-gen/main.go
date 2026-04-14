package main

import (
	"log"
	"os"

	"github.com/nebius/nccl-inspector-preconf/internal/utils/optionrender"
)

func main() {
	if err := optionrender.Render(os.Stdout); err != nil {
		log.Fatal(err)
	}
}
