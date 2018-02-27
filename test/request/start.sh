#!/bin/sh
#
# start.sh
#
# Test the server
#
# @author Kealan McCusker <kealanmccusker@gmail.com>>
# -----------------------------------------------------------------------------

# NOTES:

echo "make client request"
sleep 300
/usr/bin/curl -i -H "Content-Type: application/json" -X POST http://localhost:8001/status 
/usr/bin/curl -i -H "Content-Type: application/json" -X POST http://localhost:8001/status > response.txt
more response.txt
grep "HTTP/1.1 200 OK" ./response.txt
if [ $? = 0 ]; then
    echo "TEST1 PASSED"
else 
    echo "TEST1 FAILED"
fi

/usr/bin/curl -i -H "Content-Type: application/json" -X POST -d '{"data": "testuser"}' http://localhost:8001/data > response.txt
grep "HTTP/1.1 200 OK" ./response.txt
if [ $? = 0 ]; then
    echo "TEST2 PASSED"
else 
    echo "TEST2 FAILED"
fi

/usr/bin/curl -i -H "Content-Type: application/json" -X POST -d '{"data": "testuser"}' http://localhost:8001/baddata  > response.txt
grep "HTTP/1.1 400 Bad Request" ./response.txt
if [ $? = 0 ]; then
    echo "TEST3 PASSED"
else 
    echo "TEST3 FAILED"
fi

echo "make client finished"
sleep 500

