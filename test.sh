#!/bin/sh
#
# test.sh
#
# Test the server
#
# @author Kealan McCusker <kealanmccusker@gmail.com>>
# -----------------------------------------------------------------------------

# NOTES:

#echo "make client request"
# curl -H "Content-Type: application/json" http://localhost:8001/status  --trace-ascii /dev/stdout
cd ./test
docker-compose up --build &
sleep 60
docker-compose  logs request > response.txt
more response.txt
docker-compose down
grep "HTTP/1.1 200 OK" ./response.txt
if [ $? = 0 ]; then
    echo "TEST PASSED"
    exit 0
fi

