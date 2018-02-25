version: '3'

services:

  server-demo:
    image: "kealan/server-demo:tag"
    container_name: "server-demo"
    hostname: "server-demo"
    ports:
      - "8001:8000"

  request:
    build: ./request
    container_name: "request"
    depends_on:
      - server-demo


