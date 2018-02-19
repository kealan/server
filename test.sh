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
sleep 200
grep "HTTP/1.1 200 OK" ./request/test.txt
if [[ "$?" = 0 ]]; then
    echo "TEST PASSED"
    docker-compose down
    exit 0
fi

