package main

import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"os"
)

type Image struct {
	ImageURL string `json:"image_url"`
}

func main() {
	token := os.Getenv("HACKATTIC_TOKEN")
	reqURL := fmt.Sprintf("https://hackattic.com/challenges/basic_face_detection/problem?access_token=%s", token)

	// send the http request
	resp, _ := http.Get(reqURL)
	defer resp.Body.Close()

	body, _ := io.ReadAll(resp.Body)

	// get image url from json
	var image Image

	if err := json.Unmarshal(body, &image); err != nil {
		fmt.Printf("JSON decoding failed: %s\n", err)
	}

	// download image
	respImg, _ := http.Get(image.ImageURL)
	defer resp.Body.Close()

	file, _ := os.Create("image.jpg")
	defer file.Close()

	if _, err := io.Copy(file, respImg.Body); err != nil {
		fmt.Printf("Copying image byte failed: %s\n", err)
	}

}
